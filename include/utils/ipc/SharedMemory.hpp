#pragma once

#include <windows.h>
#include <mutex>
#include <string>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LogEntry.h"

namespace Utils
{
    namespace Ipc
    {
        ////////////////////////
        // Memoria compartida //
        ////////////////////////

        template <typename T>
        class SharedMemory: public Logging::LoggerHolder
        {
            //------------//
            // Constantes //
            //------------//

            public:
                static const DWORD TIMEOUT_READ;
                static const DWORD TIMEOUT_WRITE;
            
            private:
                static const char *SHARED_MEMORY_NAME;
                static const char *SHARED_MEMORY_MAPPED_FILE_NAME;
                static const char *SHARED_MEMORY_MUTEX_NAME;
    
            //------------------------//
            // Constructor/Destructor //
            //------------------------//

            public:
                SharedMemory(const std::string &name, bool isOwner, const SharedLogger &logger = BASIC_LOGGER())
                    : Logging::LoggerHolder(logger)
                    , name(name)
                {
                    bool result = false;
                    
                    // Obtenemos el mutex global
                    if (!this->createGlobalMutex(isOwner)) return;
    
                    // Obtenemos la zona de memoria compartida
                    if (isOwner) result = this->createFileMapping();
                    else         result = this->openFileMapping();
    
                    if (!result)
                    {
                        this->closeFileMap();
                        return;
                    }
    
                    // Obtenemos el map view
                    if (!this->getMapView()) this->closeFileMap();
    
                    // Liberamos el mutex global (si somos el propietario)
                    if (isOwner && !this->releaseOwnership()) this->closeFileMap();
                }
    
                ~SharedMemory()
                {
                    this->closeFileMap();
                }

            //----------//
            // Deleted  //
            //----------//

            public:
                explicit SharedMemory(const SharedMemory&)   = delete;
                SharedMemory &operator=(const SharedMemory&) = delete;
    
            //--------------------//
            // Funciones miembro  //
            //--------------------//

            public:
                bool isValid()
                {
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    return this->sharedMemmoryGlobalMutex && this->fileMapping && this->mapView;
                }
    
                bool readData(T &data, DWORD timeout = TIMEOUT_READ)
                {
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (!this->sharedMemmoryGlobalMutex || !this->fileMapping || !this->mapView) return false;
                    
                    if (!this->getOwnership(timeout)) return false;
    
                    data = *this->mapView;
    
                    return this->releaseOwnership();
                }
    
                bool writeData(const T &data, bool flush = true, DWORD timeout = TIMEOUT_WRITE)
                {
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (!this->sharedMemmoryGlobalMutex || !this->fileMapping || !this->mapView) return false;
                    
                    if (!this->getOwnership(timeout)) return false;
    
                    *this->mapView = data;
    
                    bool result = true;
                    if (flush)
                    {
                        if (!FlushViewOfFile(this->mapView, 0))
                        {
                            LOGGER_THIS_LOG_ERROR() << "ERROR refrescando datos de " << this->name << ": " << GetLastError();
                            result = false;
                        }
                    }
    
                    if (!this->releaseOwnership()) return false;
    
                    return result;
                }
    
            private:
                std::string getFileMapName() const
                {
                    return std::string(SHARED_MEMORY_NAME) + std::string("/") +
                           std::string(SHARED_MEMORY_MAPPED_FILE_NAME) + std::string("/") +
                           this->name;
                }
    
                std::string getGlobalMutexName() const
                {
                    return std::string(SHARED_MEMORY_NAME) + std::string("/") +
                           std::string(SHARED_MEMORY_MUTEX_NAME) + std::string("/") +
                           this->name;
                }
    
                bool createGlobalMutex(bool isOwner)
                {
                    if (this->name.empty())
                    {
                        LOGGER_THIS_LOG_ERROR() << "Nombre de memoria compartida NO valido";
                        return false;
                    }
                    
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (this->sharedMemmoryGlobalMutex) return true;
    
                    std::string mutexName = this->getGlobalMutexName();
                    this->sharedMemmoryGlobalMutex = CreateMutex(nullptr
                                                               , isOwner
                                                               , mutexName.c_str());
                    if (!this->sharedMemmoryGlobalMutex)
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR creando mutex global " << this->name << ": " << GetLastError();
                        return false;
                    }
    
                    return true;
                }
    
                bool createFileMapping()
                {
                    if (this->name.empty())
                    {
                        LOGGER_THIS_LOG_ERROR() << "Nombre de memoria compartida NO valido";
                        return false;
                    }
                    
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (!this->sharedMemmoryGlobalMutex)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Mutex NO valido";
                        return false;
                    }
                    if (this->fileMapping) return true;
    
                    // Creamos la memoria compartida
                    uint64_t    structSize  = sizeof(T);
                    std::string fileMapName = this->getFileMapName();
                    this->fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE
                                                        , nullptr
                                                        , PAGE_READWRITE
                                                        , (structSize >> 32) & 0xFFFFFFFF
                                                        , structSize & 0xFFFFFFFF
                                                        , fileMapName.c_str());
                    if (!this->fileMapping)
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR creando espacio de memoria compartido " << this->name << ": " << GetLastError();
                        return false;
                    }
    
                    return true;
                }
    
                bool openFileMapping()
                {
                    if (this->name.empty())
                    {
                        LOGGER_THIS_LOG_ERROR() << "Nombre de memoria compartida NO valido";
                        return false;
                    }
    
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (!this->sharedMemmoryGlobalMutex)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Mutex NO valido";
                        return false;
                    }
                    if (this->fileMapping) return true;
    
                    // Obtenemos la memoria compartida
                    std::string fileMapName = this->getFileMapName();
                    this->fileMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE
                                                      , FALSE
                                                      , fileMapName.c_str());
                    if (!this->fileMapping)
                    {
                        LOGGER_THIS_LOG_INFO() << "ERROR abriendo espacio de memoria compartido " << this->name << ": " << GetLastError();
                        return false;
                    }
    
                    return true;
                }
    
                bool getMapView()
                {
                    if (this->name.empty())
                    {
                        LOGGER_THIS_LOG_ERROR() << "Nombre de memoria compartida NO valido";
                        return false;
                    }
    
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (!this->sharedMemmoryGlobalMutex)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Mutex NO valido";
                        return false;
                    }
                    if (!this->fileMapping)
                    {
                        LOGGER_THIS_LOG_ERROR() << "FileMap NO valido";
                        return false;
                    }
                    if (this->mapView) return true;
    
                    // Obtenemos el map view
                    uint64_t structSize   = sizeof(T);
                    LPVOID mapViewAddress = MapViewOfFile(this->fileMapping
                                                        , FILE_MAP_ALL_ACCESS
                                                        , 0
                                                        , 0
                                                        , 0);
                    if (!mapViewAddress)
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo map view de " << this->name << ": " << GetLastError();
                        return false;
                    }
    
                    this->mapView = static_cast<T*>(mapViewAddress);
                    if (!this->mapView)
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR traduciendo map view de " << this->name << ": " << GetLastError();
                        if (!UnmapViewOfFile(mapViewAddress)) LOGGER_THIS_LOG_WARNING() << "ERROR cerrando map view de " << this->name << ": " << GetLastError();
                        return false;
                    }
    
                    return true;
                }
    
                bool getOwnership(DWORD timeout)
                {
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (!this->sharedMemmoryGlobalMutex)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Mutex NO valido";
                        return false;
                    }
                    
                    switch (WaitForSingleObject(this->sharedMemmoryGlobalMutex, timeout))
                    {
                        case WAIT_ABANDONED:
                            LOGGER_THIS_LOG_ERROR() << "Posible error en la integridad de los datos detectado en memoria compartida " << this->name;
                        case WAIT_OBJECT_0:
                            break;
                        
                        case WAIT_TIMEOUT:
                            return false;
    
                        default:
                            LOGGER_THIS_LOG_WARNING() << "ERROR obteniendo propiedad de memoria compartida " << this->name << ": " << GetLastError();
                            return false;
                    }
    
                    return true;
                }
    
                bool releaseOwnership()
                {
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
                    if (!this->sharedMemmoryGlobalMutex)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Mutex NO valido";
                        return false;
                    }
    
                    if (!ReleaseMutex(this->sharedMemmoryGlobalMutex))
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR liberando propiedad de mutex global en memoria compartida " << this->name;
                        return false;
                    }
    
                    return true;
                }
    
                void closeFileMap()
                {
                    std::lock_guard<std::recursive_mutex> lock(this->sharedMemmoryMutex);
    
                    if (this->mapView)
                    {
                        if (!UnmapViewOfFile(this->mapView)) LOGGER_THIS_LOG_ERROR() << "ERROR cerrando map view de " << this->name << ": " << GetLastError();
                        this->mapView = nullptr;
                    }
    
                    if (this->fileMapping)
                    {
                        if (!CloseHandle(this->fileMapping)) LOGGER_THIS_LOG_ERROR() << "ERROR cerrando espacio de memoria compartido " << this->name << ": " << GetLastError();
                        this->fileMapping = nullptr;
                    }
    
                    if (this->sharedMemmoryGlobalMutex)
                    {
                        if (!CloseHandle(this->sharedMemmoryGlobalMutex)) LOGGER_THIS_LOG_ERROR() << "ERROR cerrando mutex global " << this->name << ": " << GetLastError();
                        this->sharedMemmoryGlobalMutex = nullptr;
                    }
                }

            //--------------------//
            // Variables miembro  //
            //--------------------//
            
            private:
                const std::string name;
    
                HANDLE fileMapping              = nullptr;
                T*     mapView                  = nullptr;
                HANDLE sharedMemmoryGlobalMutex = nullptr;
                std::recursive_mutex sharedMemmoryMutex;
        };

        //////////////////////////////////
        // Instanciacion de constantes  //
        //////////////////////////////////
    
        template <typename T> const DWORD SharedMemory<T>::TIMEOUT_READ  = 1000;
        template <typename T> const DWORD SharedMemory<T>::TIMEOUT_WRITE = 1000;

        template <typename T> const char *SharedMemory<T>::SHARED_MEMORY_NAME             = "SharedMemory";
        template <typename T> const char *SharedMemory<T>::SHARED_MEMORY_MAPPED_FILE_NAME = "MappedFile";
        template <typename T> const char *SharedMemory<T>::SHARED_MEMORY_MUTEX_NAME       = "GlobalMutex";
    };
};
