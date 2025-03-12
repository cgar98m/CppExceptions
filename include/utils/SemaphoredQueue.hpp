#pragma once

#include <windows.h>
#include <deque>
#include <mutex>

namespace Utils
{
    // Cola que hace uso de senales
    template <typename T>
    class SemaphoredQueue
    {
        private:
            static const LONG  QUEUE_MAX_SIZE;
            static const DWORD QUEUE_POP_TIMEOUT;

            std::deque<T> queueList;
            std::mutex    queueMutex;

            HANDLE        queueSemaphore;
            std::mutex    semaphoreMutex;

        public:
            SemaphoredQueue()
                : queueSemaphore(CreateSemaphore(nullptr, 0, QUEUE_MAX_SIZE, nullptr))
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

            bool push(const T& data)
            {
                HANDLE semaphore = nullptr;
                {
                    std::lock_guard<std::mutex> lock(semaphoreMutex);
                    if (!queueSemaphore) return false;

                    HANDLE processHandle = GetCurrentProcess();
                    if (!DuplicateHandle(processHandle
                                       , queueSemaphore
                                       , processHandle
                                       , &semaphore
                                       , 0
                                       , FALSE
                                       , DUPLICATE_SAME_ACCESS))
                        return false;
                }
                if (!semaphore) return false;

                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    queueList.push_back(data);
                }

                bool result = true;
                if (!ReleaseSemaphore(semaphore, 1, nullptr)) result = false;
                CloseHandle(semaphore);

                return result;
            }

            DWORD pop(T& data, DWORD timeout = QUEUE_POP_TIMEOUT)
            {
                HANDLE semaphore = nullptr;
                {
                    std::lock_guard<std::mutex> lock(semaphoreMutex);
                    if (!queueSemaphore) return WAIT_ABANDONED;

                    HANDLE processHandle = GetCurrentProcess();
                    if (!DuplicateHandle(processHandle
                                       , queueSemaphore
                                       , processHandle
                                       , &semaphore
                                       , 0
                                       , FALSE
                                       , DUPLICATE_SAME_ACCESS))
                        return WAIT_ABANDONED;
                }
                if (!semaphore) return WAIT_ABANDONED;

                DWORD resultado = WaitForSingleObject(semaphore, timeout);
                switch (resultado)
                {
                    case WAIT_OBJECT_0:
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        if (queueList.empty())
                        {
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
                        break;
                }
                CloseHandle(semaphore);

                return resultado;
            }

            bool clear()
            {
                std::lock_guard<std::mutex> lockSemaphore(semaphoreMutex);
                if (!queueSemaphore) return false;

                HANDLE semaphore = CreateSemaphore(nullptr, 0, QUEUE_MAX_SIZE, nullptr);
                if (!semaphore) return false;

                if (!ReleaseSemaphore(queueSemaphore, 1, nullptr) || !CloseHandle(queueSemaphore))
                {
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
