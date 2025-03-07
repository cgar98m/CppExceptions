#pragma once

#include <windows.h>
#include <atomic>
#include <mutex>
#include "error/Types.h"

namespace Utils
{
    // Thread
    class Thread
    {
        private:
            HANDLE            threadHandle = nullptr;
            DWORD             threadId     = 0;
            std::atomic<bool> running      = false;

        public:
            Thread() = default;
            Thread(const Thread&) = delete;
            Thread& operator=(const Thread&) = delete;
            virtual ~Thread() = default;

            HANDLE getHandle() const;
            void setHandle(HANDLE newThreadHandle = nullptr);

            DWORD getId() const;
            void setId(DWORD newThreadId = 0);

            bool isRunning() const;
            void setRunning(bool newRunning = false);

            virtual Error::ExitCode workerWrapper();
            
        protected:
            virtual Error::ExitCode worker();
    };

    // Gestor de un thread
    class ThreadHolder
    {
        public:
            static const DWORD TIMEOUT_MS_STOP_WAIT;
            static const DWORD TIMEOUT_MS_LOOP_WAIT;
        
            struct Params
            {
                int   threadPriority = THREAD_PRIORITY_NORMAL;
                DWORD loopWait       = TIMEOUT_MS_LOOP_WAIT;
                DWORD stopWait       = TIMEOUT_MS_STOP_WAIT;
            };

        private:
            Params params;

            std::recursive_mutex threadMux;
            Thread               &thread;

        public:
            ThreadHolder(Thread &thread, const Params &params);
            virtual ~ThreadHolder();

            virtual bool run();

            bool requestStop();
            Error::ExitCode waitStop();
            Error::ExitCode stop();

            Error::ExitCode threadLoop();

            static DWORD WINAPI threadWrapper(LPVOID param);
    };
};
