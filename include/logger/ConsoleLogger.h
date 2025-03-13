#pragma once

#include <memory>
#include <mutex>
#include <string>
#include "logger/IThreadedLogger.h"

namespace Logger
{
    // Logger para consola
    class ConsoleLogger: public IThreadedLogger
    {
        public:
            static const std::string MUX_PREFIX;
            static const DWORD       MUX_TIMEOUT;

        private:
            static Logger     consoleLogger;
            static std::mutex muxInstance;

            HANDLE     printMutex;
            std::mutex printMutexMux;

        public:
            static Logger getInstance();
            
            ConsoleLogger(const ConsoleLogger&) = delete;
            ConsoleLogger& operator=(const ConsoleLogger&) = delete;
            virtual ~ConsoleLogger();
            
        private:
            ConsoleLogger();
            
            bool printEnqueued(const LogMsg &message) final;
    };
};
