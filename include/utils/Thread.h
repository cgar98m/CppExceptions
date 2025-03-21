#pragma once

#include <windows.h>
#include <atomic>
#include <mutex>
#include "utils/ExitCode.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"

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

            virtual Utils::ExitCode workerWrapper();
            
        protected:
            virtual Utils::ExitCode worker();
    };

    // Gestor de un thread
    class ThreadHolder: public Utils::ILoggerHolder
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
            ThreadHolder() = delete;
            explicit ThreadHolder(Thread &thread, const Params &params, const Utils::Logger& logger = Utils::BasicLogger::getInstance());
            ThreadHolder& operator=(const ThreadHolder&) = delete;
            virtual ~ThreadHolder();

            virtual bool run();

            bool requestStop();
            Utils::ExitCode waitStop();
            Utils::ExitCode stop();

            Utils::ExitCode threadLoop();

            static DWORD WINAPI threadWrapper(LPVOID param);
    };
};
