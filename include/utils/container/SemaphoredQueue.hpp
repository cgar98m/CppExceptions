#pragma once

#include <windows.h>
#include <deque>
#include <mutex>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"

namespace Utils
{
    // Cola que hace uso de senales
    template <typename T>
    class SemaphoredQueue: public ILoggerHolder
    {
        private:
            static const LONG  QUEUE_MAX_SIZE;
            static const DWORD QUEUE_POP_TIMEOUT;

            std::deque<T> queueList;
            std::mutex    queueMutex;

            HANDLE        queueSemaphore;
            std::mutex    semaphoreMutex;

        public:
            SemaphoredQueue(const Logger &errorLogger = BasicLogger::getInstance())
                : ILoggerHolder(errorLogger)
                , queueSemaphore(CreateSemaphore(nullptr, 0, QUEUE_MAX_SIZE, nullptr))
            {
            }

            virtual ~SemaphoredQueue()
            {
                std::lock_guard<std::mutex> lock(semaphoreMutex);
                if (queueSemaphore)
                {
                    ReleaseSemaphore(queueSemaphore, 1, nullptr);
                    CloseHandle(queueSemaphore);
                    queueSemaphore = nullptr;
                }
            }

            bool push(const T &data)
            {
                HANDLE semaphore = nullptr;
                {
                    std::lock_guard<std::mutex> lock(semaphoreMutex);
                    if (!queueSemaphore)
                    {
                        LOGGER_THIS_LOG_INFO() << "Semaforo NO inicializado";
                        return false;
                    }

                    HANDLE processHandle = GetCurrentProcess();
                    if (!DuplicateHandle(processHandle
                                       , queueSemaphore
                                       , processHandle
                                       , &semaphore
                                       , 0
                                       , FALSE
                                       , DUPLICATE_SAME_ACCESS))
                    {
                        LOGGER_THIS_LOG_INFO() << "Semaforo NO duplicado: " << GetLastError();
                        return false;
                    }
                }
                if (!semaphore)
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO valido";
                    return false;
                }

                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    queueList.push_back(data);
                }

                bool result = true;
                if (!ReleaseSemaphore(semaphore, 1, nullptr))
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO senalizado: " << GetLastError();
                    return false;
                }
                CloseHandle(semaphore);

                return result;
            }

            DWORD top(T &data, DWORD timeout = QUEUE_POP_TIMEOUT)
            {
                HANDLE semaphore = nullptr;
                {
                    std::lock_guard<std::mutex> lock(semaphoreMutex);
                    if (!queueSemaphore)
                    {
                        LOGGER_THIS_LOG_INFO() << "Semaforo NO inicializado";
                        return WAIT_ABANDONED;
                    }

                    HANDLE processHandle = GetCurrentProcess();
                    if (!DuplicateHandle(processHandle
                                       , queueSemaphore
                                       , processHandle
                                       , &semaphore
                                       , 0
                                       , FALSE
                                       , DUPLICATE_SAME_ACCESS))
                    {
                        LOGGER_THIS_LOG_INFO() << "Semaforo NO duplicado: " << GetLastError();
                        return WAIT_ABANDONED;
                    }
                }
                if (!semaphore)
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO valido";
                    return WAIT_ABANDONED;
                }

                DWORD resultado = WaitForSingleObject(semaphore, timeout);
                switch (resultado)
                {
                    case WAIT_OBJECT_0:
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        if (queueList.empty())
                        {
                            LOGGER_THIS_LOG_INFO() << "Cola vacia";
                            resultado = WAIT_FAILED;
                            break;
                        }
                        
                        data = queueList.front();
                        if (!ReleaseSemaphore(semaphore, 1, nullptr))
                        {
                            LOGGER_THIS_LOG_INFO() << "Semaforo NO senalizado: " << GetLastError();
                            resultado = WAIT_FAILED;
                        }
                        break;
                    }
        
                    case WAIT_TIMEOUT:
                        break;
                    
                    case WAIT_ABANDONED:
                    case WAIT_FAILED:
                    default:
                        LOGGER_THIS_LOG_INFO() << "ERROR esperando mensaje: " << GetLastError();
                        break;
                }
                CloseHandle(semaphore);

                return resultado;
            }

            DWORD pop(T &data, DWORD timeout = QUEUE_POP_TIMEOUT)
            {
                HANDLE semaphore = nullptr;
                {
                    std::lock_guard<std::mutex> lock(semaphoreMutex);
                    if (!queueSemaphore)
                    {
                        LOGGER_THIS_LOG_INFO() << "Semaforo NO inicializado";
                        return WAIT_ABANDONED;
                    }

                    HANDLE processHandle = GetCurrentProcess();
                    if (!DuplicateHandle(processHandle
                                       , queueSemaphore
                                       , processHandle
                                       , &semaphore
                                       , 0
                                       , FALSE
                                       , DUPLICATE_SAME_ACCESS))
                    {
                        LOGGER_THIS_LOG_INFO() << "Semaforo NO duplicado: " << GetLastError();
                        return WAIT_ABANDONED;
                    }
                }
                if (!semaphore)
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO valido";
                    return WAIT_ABANDONED;
                }

                DWORD resultado = WaitForSingleObject(semaphore, timeout);
                switch (resultado)
                {
                    case WAIT_OBJECT_0:
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        if (queueList.empty())
                        {
                            LOGGER_THIS_LOG_INFO() << "Cola vacia";
                            resultado = WAIT_FAILED;
                            break;
                        }
                        
                        data = queueList.front();
                        queueList.pop_front();
                        break;
                    }
        
                    case WAIT_TIMEOUT:
                        break;
                    
                    case WAIT_ABANDONED:
                    case WAIT_FAILED:
                    default:
                        LOGGER_THIS_LOG_INFO() << "ERROR esperando mensaje: " << GetLastError();
                        break;
                }
                CloseHandle(semaphore);

                return resultado;
            }

            bool clear()
            {
                std::lock_guard<std::mutex> lockSemaphore(semaphoreMutex);
                if (!queueSemaphore)
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO inicializado";
                    return false;
                }

                HANDLE semaphore = CreateSemaphore(nullptr, 0, QUEUE_MAX_SIZE, nullptr);
                if (!semaphore)
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO creado";
                    return false;
                }

                if (!ReleaseSemaphore(queueSemaphore, 1, nullptr))
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO senalizado: " << GetLastError();
                    CloseHandle(semaphore);
                    return false;
                }
                if (!CloseHandle(queueSemaphore))
                {
                    LOGGER_THIS_LOG_INFO() << "Semaforo NO cerrado: " << GetLastError();
                    CloseHandle(semaphore);
                    return false;
                }
                queueSemaphore = semaphore;

                std::lock_guard<std::mutex> lockQueue(queueMutex);
                queueList.clear();
                return true;
            }
    };

    template <typename T> const LONG  SemaphoredQueue<T>::QUEUE_MAX_SIZE    = 128;
    template <typename T> const DWORD SemaphoredQueue<T>::QUEUE_POP_TIMEOUT = 5000;
};
