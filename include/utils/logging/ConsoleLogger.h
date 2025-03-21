#pragma once

#include <windows.h>
#include <mutex>
#include <string>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/IThreadedLogger.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    // Logger para consola
    class ConsoleLogger: public IThreadedLogger, public ILoggerHolder
    {
        public:
            static const char  *MUX_NAME;
            static const DWORD MUX_TIMEOUT;

        private:
            static Logger     consoleLogger;
            static std::mutex instanceMux;

            HANDLE     ostreamMux = nullptr;
            std::mutex internalMux;

        public:
            static Logger getInstance(const Logger& errorLogger = BasicLogger::getInstance());
            
            ConsoleLogger(const ConsoleLogger&) = delete;
            ConsoleLogger& operator=(const ConsoleLogger&) = delete;
            virtual ~ConsoleLogger();
            
        private:
            ConsoleLogger(const Logger& errorLogger = BasicLogger::getInstance());
            
            bool printEnqueued(const LogMsg &message) final;
    };
};
