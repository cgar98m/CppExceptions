#pragma once

#include <windows.h>
#include <mutex>
#include <string>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"

namespace Utils
{
    // Memoria compartida
    template <typename T>
    class SharedMemory: public Utils::ILoggerHolder
    {
        public:
            static const DWORD TIMEOUT_READ;
            static const DWORD TIMEOUT_WRITE;

        private:
            const std::string name;

            HANDLE fileMapping              = nullptr;
            T*     mapView                  = nullptr;
            HANDLE sharedMemmoryGlobalMutex = nullptr;
            std::recursive_mutex sharedMemmoryMutex;

        public:
            SharedMemory(const std::string &name, bool isOwner, const Utils::Logger &logger = Utils::BasicLogger::getInstance())
                : ILoggerHolder(logger)
                , name(name)
            {
                bool result = false;
                
                // Obtenemos el mutex global
                if (!createGlobalMutex(isOwner)) return;

                // Obtenemos la zona de memoria compartida
                if (isOwner) result = createFileMapping();
                else         result = openFileMapping();

                if (!result)
                {
                    closeFileMap();
                    return;
                }

                // Obtenemos el map view
                if (!getMapView()) closeFileMap();

                // Liberamos el mutex global (si somos el propietario)
                if (isOwner && !releaseOwnership()) closeFileMap();
            }

            ~SharedMemory()
            {
                closeFileMap();
            }

            bool isValid()
            {
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                return sharedMemmoryGlobalMutex && fileMapping && mapView;
            }

            bool readData(T &data, DWORD timeout = TIMEOUT_READ)
            {
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (!sharedMemmoryGlobalMutex || !fileMapping || !mapView) return false;
                
                if (!getOwnership(timeout)) return false;

                data = *mapView;

                return releaseOwnership();
            }

            bool writeData(const T &data, bool flush = true, DWORD timeout = TIMEOUT_WRITE)
            {
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (!sharedMemmoryGlobalMutex || !fileMapping || !mapView) return false;
                
                if (!getOwnership(timeout)) return false;

                *mapView = data;

                bool result = true;
                if (flush)
                {
                    if (!FlushViewOfFile(mapView, 0))
                    {
                        LOGGER_THIS_LOG_INFO() << "ERROR refrescando datos de " << name << ": " << GetLastError();
                        result = false;
                    }
                }

                if (!releaseOwnership()) return false;

                return result;
            }

        private:
            std::string getFileMapName() const
            {
                return std::string("MappedFile_") + name;
            }

            std::string getGlobalMutexName() const
            {
                return std::string("GlobalMutex_") + name;
            }

            bool createGlobalMutex(bool isOwner)
            {
                if (name.empty())
                {
                    LOGGER_THIS_LOG_INFO() << "Nombre de memoria compartida NO valido";
                    return false;
                }
                
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (sharedMemmoryGlobalMutex) return true;

                std::string mutexName = getGlobalMutexName();
                sharedMemmoryGlobalMutex = CreateMutex(nullptr
                                                     , isOwner
                                                     , mutexName.c_str());
                if (!sharedMemmoryGlobalMutex)
                {
                    LOGGER_THIS_LOG_INFO() << "ERROR creando mutex global " << name << ": " << GetLastError();
                    return false;
                }

                return true;
            }

            bool createFileMapping()
            {
                if (name.empty())
                {
                    LOGGER_THIS_LOG_INFO() << "Nombre de memoria compartida NO valido";
                    return false;
                }
                
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (!sharedMemmoryGlobalMutex)
                {
                    LOGGER_THIS_LOG_INFO() << "Mutex NO valido";
                    return false;
                }
                if (fileMapping) return true;

                // Creamos la memoria compartida
                uint64_t structSize     = sizeof(T);
                std::string fileMapName = getFileMapName();
                fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE
                                              , nullptr
                                              , PAGE_READWRITE
                                              , (structSize >> 32) & 0xFFFFFFFF
                                              , structSize & 0xFFFFFFFF
                                              , fileMapName.c_str());
                if (!fileMapping)
                {
                    LOGGER_THIS_LOG_INFO() << "ERROR creando espacio de memoria compartido " << name << ": " << GetLastError();
                    return false;
                }

                return true;
            }

            bool openFileMapping()
            {
                if (name.empty()) return false;

                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (!sharedMemmoryGlobalMutex)
                {
                    LOGGER_THIS_LOG_INFO() << "Mutex NO valido";
                    return false;
                }
                if (fileMapping) return true;

                // Obtenemos la memoria compartida
                std::string fileMapName = getFileMapName();
                fileMapping = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE
                                            , FALSE
                                            , fileMapName.c_str());
                if (!fileMapping)
                {
                    LOGGER_THIS_LOG_INFO() << "ERROR abriendo espacio de memoria compartido " << name << ": " << GetLastError();
                    return false;
                }

                return true;
            }

            bool getMapView()
            {
                if (name.empty()) return false;

                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (!sharedMemmoryGlobalMutex)
                {
                    LOGGER_THIS_LOG_INFO() << "Mutex NO valido";
                    return false;
                }
                if (!fileMapping)
                {
                    LOGGER_THIS_LOG_INFO() << "FileMap NO valido";
                    return false;
                }
                if (mapView) return true;

                // Obtenemos el map view
                uint64_t structSize = sizeof(T);
                LPVOID mapViewAddress = MapViewOfFile(fileMapping
                                                    , FILE_MAP_ALL_ACCESS
                                                    , 0
                                                    , 0
                                                    , 0);
                if (!mapViewAddress)
                {
                    LOGGER_THIS_LOG_INFO() << "ERROR obteniendo map view de " << name << ": " << GetLastError();
                    return false;
                }

                mapView = static_cast<T*>(mapViewAddress);
                if (!mapView)
                {
                    LOGGER_THIS_LOG_INFO() << "ERROR traduciendo map view de " << name << ": " << GetLastError();
                    if (!UnmapViewOfFile(mapViewAddress)) LOGGER_THIS_LOG_INFO() << "ERROR cerrando map view de " << name << ": " << GetLastError();
                    return false;
                }

                return true;
            }

            bool getOwnership(DWORD timeout)
            {
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (!sharedMemmoryGlobalMutex)
                {
                    LOGGER_THIS_LOG_INFO() << "Mutex NO valido";
                    return false;
                }
                
                switch (WaitForSingleObject(sharedMemmoryGlobalMutex, timeout))
                {
                    case WAIT_ABANDONED:
                        LOGGER_THIS_LOG_INFO() << "Posible error en la integridad de los datos detectado en memoria compartida " << name;
                    case WAIT_OBJECT_0:
                        break;
                    
                    case WAIT_TIMEOUT:
                        return false;

                    default:
                        LOGGER_THIS_LOG_INFO() << "ERROR obteniendo propiedad de memoria compartida " << name << ": " << GetLastError();
                        return false;
                }

                return true;
            }

            bool releaseOwnership()
            {
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);
                if (!sharedMemmoryGlobalMutex)
                {
                    LOGGER_THIS_LOG_INFO() << "Mutex NO valido";
                    return false;
                }

                if (!ReleaseMutex(sharedMemmoryGlobalMutex))
                {
                    LOGGER_THIS_LOG_INFO() << "ERROR liberando propiedad de mutex global en memoria compartida " << name;
                    return false;
                }

                return true;
            }

            void closeFileMap()
            {
                std::lock_guard<std::recursive_mutex> lock(sharedMemmoryMutex);

                if (mapView)
                {
                    if (!UnmapViewOfFile(mapView)) LOGGER_THIS_LOG_INFO() << "ERROR cerrando map view de " << name << ": " << GetLastError();
                    mapView = nullptr;
                }

                if (fileMapping)
                {
                    if (!CloseHandle(fileMapping)) LOGGER_THIS_LOG_INFO() << "ERROR cerrando espacio de memoria compartido " << name << ": " << GetLastError();
                    fileMapping = nullptr;
                }

                if (sharedMemmoryGlobalMutex)
                {
                    if (!CloseHandle(sharedMemmoryGlobalMutex)) LOGGER_THIS_LOG_INFO() << "ERROR cerrando mutex global " << name << ": " << GetLastError();
                    sharedMemmoryGlobalMutex = nullptr;
                }
            }
    };

    template <typename T> const DWORD SharedMemory<T>::TIMEOUT_READ  = 1000;
    template <typename T> const DWORD SharedMemory<T>::TIMEOUT_WRITE = 1000;
};
