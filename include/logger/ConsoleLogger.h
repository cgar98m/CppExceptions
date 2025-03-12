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
        private:
            static Logger     consoleLogger;
            static std::mutex muxInstance;

        public:
            static Logger getInstance();
            
            ConsoleLogger(const ConsoleLogger&) = delete;
            ConsoleLogger& operator=(const ConsoleLogger&) = delete;
            ~ConsoleLogger() = default;
            
        private:
            ConsoleLogger() = default;
            
            bool printEnqueued(const std::string &message) final;
    };
};
