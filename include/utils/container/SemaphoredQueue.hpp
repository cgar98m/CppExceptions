#pragma once

#include <windows.h>
#include <deque>
#include <mutex>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LogEntry.h"
#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Container
    {
        //////////////////////////////////
        // Cola que hace uso de senales //
        //////////////////////////////////

        template <typename T>
        class SemaphoredQueue: public Logging::LoggerHolder
        {
            //------------//
            // Constantes //
            //------------//
            
            private:
                static const LONG  QUEUE_MAX_SIZE;
                static const DWORD QUEUE_POP_TIMEOUT;
    
            //------------------------//
            // Constructor/Destructor //
            //------------------------//

            public:
                SemaphoredQueue(const SharedLogger &errorLogger = BASIC_LOGGER())
                    : Logging::LoggerHolder(errorLogger)
                    , queueSemaphore(CreateSemaphore(nullptr, 0, QUEUE_MAX_SIZE, nullptr))
                {
                }
    
                virtual ~SemaphoredQueue()
                {
                    std::lock_guard<std::mutex> lockSemaphore(this->semaphoreMutex);
                    std::lock_guard<std::mutex> lockQueue(this->queueMutex);

                    if (this->queueSemaphore)
                    {
                        // Notificamos un item para que se desbloquee la espera del semaforo
                        ReleaseSemaphore(this->queueSemaphore, 1, nullptr);
                        CloseHandle(this->queueSemaphore);
                        this->queueSemaphore = nullptr;
                    }
                 
                    this->queueList.clear();
                }

            //----------//
            // Deleted  //
            //----------//

            public:
                explicit SemaphoredQueue(const SemaphoredQueue&)   = delete;
                SemaphoredQueue &operator=(const SemaphoredQueue&) = delete;

            //--------------------//
            // Funciones miembro  //
            //--------------------//

            public:
                bool push(const T &data)
                {
                    // Duplicamos el semaforo
                    HANDLE semaphore = this->getSemaphoreDuplicate();
                    if (!semaphore)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO valido";
                        return false;
                    }
    
                    // Anadimos el item a la cola
                    std::lock_guard<std::mutex> lockQueue(this->queueMutex);
                    this->queueList.push_back(data);
    
                    // Notificamos el nuevo item
                    bool result = true;
                    if (!ReleaseSemaphore(semaphore, 1, nullptr))
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO senalizado: " << GetLastError();
                        result = false;
                    }

                    // Cerramos el semaforo
                    CloseHandle(semaphore);
                    return result;
                }
    
                DWORD top(T &data, DWORD timeout = QUEUE_POP_TIMEOUT)
                {
                    // Duplicamos el semaforo
                    HANDLE semaphore = this->getSemaphoreDuplicate();
                    if (!semaphore)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO valido";
                        return WAIT_ABANDONED;
                    }
    
                    // Esperamos a la notificacion de un nuevo item
                    DWORD resultado = WaitForSingleObject(semaphore, timeout);
                    switch (resultado)
                    {
                        case WAIT_OBJECT_0:
                        {
                            // Consultamos el item
                            std::lock_guard<std::mutex> lockQueue(this->queueMutex);
                            if (this->queueList.empty())
                            {
                                LOGGER_THIS_LOG_WARNING() << "Cola vacia";
                                resultado = WAIT_FAILED;
                                break;
                            }
                            data = this->queueList.front();
                            
                            // Notificamos el item para poder releer el item
                            if (!ReleaseSemaphore(semaphore, 1, nullptr))
                            {
                                LOGGER_THIS_LOG_ERROR() << "Semaforo NO senalizado: " << GetLastError();
                                resultado = WAIT_FAILED;
                            }
                            break;
                        }
            
                        case WAIT_TIMEOUT:
                            break;
                        
                        case WAIT_ABANDONED:
                        case WAIT_FAILED:
                        default:
                            LOGGER_THIS_LOG_ERROR() << "ERROR esperando mensaje: " << GetLastError();
                            break;
                    }

                    // Cerramos el semaforo
                    CloseHandle(semaphore);
                    return resultado;
                }
    
                DWORD pop(T &data, DWORD timeout = QUEUE_POP_TIMEOUT)
                {
                    // Duplicamos el semaforo
                    HANDLE semaphore = this->getSemaphoreDuplicate();
                    if (!semaphore)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO valido";
                        return WAIT_ABANDONED;
                    }
    
                    // Esperamos a la notificacion de un nuevo item
                    DWORD resultado = WaitForSingleObject(semaphore, timeout);
                    switch (resultado)
                    {
                        case WAIT_OBJECT_0:
                        {
                            // Consultamos el item
                            std::lock_guard<std::mutex> lockQueue(this->queueMutex);
                            if (this->queueList.empty())
                            {
                                LOGGER_THIS_LOG_WARNING() << "Cola vacia";
                                resultado = WAIT_FAILED;
                                break;
                            }
                            
                            // Quitamos el item de la cola
                            data = this->queueList.front();
                            this->queueList.pop_front();
                            break;
                        }
            
                        case WAIT_TIMEOUT:
                            LOGGER_THIS_LOG_WARNING() << "TIMEOUT esperando mensaje";
                            resultado = WAIT_OBJECT_0;
                            break;
                        
                        case WAIT_ABANDONED:
                        case WAIT_FAILED:
                        default:
                            LOGGER_THIS_LOG_ERROR() << "ERROR esperando mensaje: " << GetLastError();
                            break;
                    }
                    
                    // Cerramos el semaforo
                    CloseHandle(semaphore);
                    return resultado;
                }
    
                bool clear()
                {
                    std::lock_guard<std::mutex> lockSemaphore(this->semaphoreMutex);
                    std::lock_guard<std::mutex> lockQueue(this->queueMutex);

                    // Verificamos si hay semaforo
                    if (!this->queueSemaphore)
                    {
                        LOGGER_THIS_LOG_WARNING() << "Semaforo NO inicializado";
                        return false;
                    }
    
                    // Creamos un semaforo nuevo para reiniciar el recuento de items
                    HANDLE semaphore = CreateSemaphore(nullptr, 0, QUEUE_MAX_SIZE, nullptr);
                    if (!semaphore)
                    {
                        LOGGER_THIS_LOG_INFO() << "Semaforo NO creado";
                        return false;
                    }
    
                    // Notificamos un item para que se desbloquee la espera del semaforo
                    if (!ReleaseSemaphore(this->queueSemaphore, 1, nullptr))
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO senalizado: " << GetLastError();
                        CloseHandle(semaphore);
                        return false;
                    }

                    // Tratamos de cerrar el semaforo en uso
                    if (!CloseHandle(this->queueSemaphore))
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO cerrado: " << GetLastError();
                        CloseHandle(semaphore);
                        return false;
                    }
                    this->queueSemaphore = semaphore;
    
                    // Limpiamos la cola
                    this->queueList.clear();
                    return true;
                }
            
            private:
                HANDLE getSemaphoreDuplicate()
                {
                    HANDLE semaphore = nullptr;

                    // Verificamos si hay semaforo
                    std::lock_guard<std::mutex> lockSemaphore(this->semaphoreMutex);
                    if (!this->queueSemaphore)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO inicializado";
                        return semaphore;
                    }

                    // Duplicamos el handle
                    HANDLE processHandle = GetCurrentProcess();
                    if (!DuplicateHandle(processHandle
                                        , this->queueSemaphore
                                        , processHandle
                                        , &semaphore
                                        , 0
                                        , FALSE
                                        , DUPLICATE_SAME_ACCESS))
                    {
                        LOGGER_THIS_LOG_ERROR() << "Semaforo NO duplicado: " << GetLastError();
                        semaphore = nullptr;
                    }

                    return semaphore;
                }

            //--------------------//
            // Variables miembro  //
            //--------------------//
            
            private:
                std::deque<T> queueList;
                std::mutex    queueMutex;
    
                HANDLE        queueSemaphore;
                std::mutex    semaphoreMutex;
        };

        //------------------------------//
        // Instanciacion de constantes  //
        //------------------------------//
    
        template <typename T> const LONG  SemaphoredQueue<T>::QUEUE_MAX_SIZE    = 128;
        template <typename T> const DWORD SemaphoredQueue<T>::QUEUE_POP_TIMEOUT = 5000;
    };
};
