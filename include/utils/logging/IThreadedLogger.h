#pragma once

#include <windows.h>
#include <mutex>
#include <string>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/thread/IQueueThread.hpp"
#include "utils/thread/ThreadHolder.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////
        // Hilo con gestion de logs por cola  //
        ////////////////////////////////////////
        
        class IThreadedLogger: public Thread::IQueueThread<LogMsg>, public ILogger, public LoggerHolder
        {
            // Constantes
            public:
                static const DWORD TIMEOUT_MS_LOOP_WAIT;
                static const DWORD TIMEOUT_MS_STOP_WAIT;
            
            private:
                static const DWORD TIMEOUT_MS_MUX_WAIT;
                static const char  *LOGGER_NAME;
            
            // Tipos, estructuras y enums
            public:
                struct Params
                {
                    int         threadPriority = THREAD_PRIORITY_NORMAL;
                    DWORD       loopWait       = TIMEOUT_MS_LOOP_WAIT;
                    DWORD       stopWait       = TIMEOUT_MS_STOP_WAIT;
                    DWORD       muxWait        = TIMEOUT_MS_MUX_WAIT;
                    std::string threadName     = LOGGER_NAME;
                };
                
            // Constructor/Destructor
            public:
                explicit IThreadedLogger(const Params &params = Params(), const SharedLogger &logger = BASIC_LOGGER());
                virtual ~IThreadedLogger();
            
            // Deleted
            public:
                IThreadedLogger()                                  = delete;
                IThreadedLogger(const IThreadedLogger&)            = delete;
                IThreadedLogger &operator=(const IThreadedLogger&) = delete;
    
            // Pure virtual
            private:
                virtual bool validateStream(const LogMsg &message)            = 0;
                virtual bool processPrintToStream(const std::string& message) = 0;

            // Final virtual
            public:
                virtual bool printRequest(const LogMsg &message) final;

            private:
                virtual bool processData(const LogMsg &data)     final;
                virtual bool processPrint(const LogMsg &message) final;
            
            // Variables miembro
            protected:
                std::mutex internalMux;
                HANDLE     ostreamMux = nullptr;

            private:
                DWORD                muxWait = TIMEOUT_MS_MUX_WAIT;
                Thread::ThreadHolder printThread;
        };
    };
};
