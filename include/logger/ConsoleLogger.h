#pragma once

#include <memory>
#include <mutex>
#include <string>
#include "logger/ILogger.h"

namespace Logger
{
    // Logger para consola
    class ConsoleLogger: public ILogger
    {
        private:
            static Logger     consoleLogger;
            static std::mutex muxInstance;
            
            std::mutex printMutex;

        public:
            static Logger getInstance();
            
            ConsoleLogger& operator=(const ConsoleLogger&) = delete;
            ~ConsoleLogger() override = default;
            
            void print(const std::string &message) final;
        
        private:
            ConsoleLogger() = default;
    };
};
