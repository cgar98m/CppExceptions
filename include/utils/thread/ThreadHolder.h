#pragma once

#include <windows.h>
#include <mutex>
#include "utils/ExitCode.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/thread/IThread.h"

namespace Utils
{
    namespace Thread
    {
        ////////////////////////
        // Gestor de un hilo  //
        ////////////////////////
        
        class ThreadHolder: public Logging::LoggerHolder
        {
            // Constantes
            public:
                static const DWORD TIMEOUT_MS_LOOP_WAIT;
                static const DWORD TIMEOUT_MS_STOP_WAIT;

            // Tipos, estructuras y enums
            public:
                struct Params
                {
                    int   threadPriority = THREAD_PRIORITY_NORMAL;
                    DWORD loopWait       = TIMEOUT_MS_LOOP_WAIT;
                    DWORD stopWait       = TIMEOUT_MS_STOP_WAIT;
                };

            // Constructor/Destructor
            public:
                explicit ThreadHolder(IThread &thread, const Params &params = Params(), const SharedLogger &logger = BASIC_LOGGER());
                virtual ~ThreadHolder();
            
            // Deleted
            public:
                ThreadHolder()                               = delete;
                ThreadHolder &operator=(const ThreadHolder&) = delete;

            // Funciones de clase
            private:
                static DWORD WINAPI threadWrapper(LPVOID param);

            // Funciones miembro
            public:
                bool run();

                void requestStop();
                ExitCode waitStop(DWORD timeout, bool closeThread = false);
                ExitCode stop();

                DWORD getStopTimeout();

            private:
                ExitCode threadLoop();

                void cleanThreadData();

            // Variables miembro
            private:
                IThread &thread;
                Params  params;

                std::recursive_mutex threadMux;
        };
    };
};
