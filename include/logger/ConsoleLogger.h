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
            static std::shared_ptr<ConsoleLogger> consoleLogger;
            static std::mutex                     muxInstance;
            
            std::mutex printMutex;

        public:
            static std::shared_ptr<ConsoleLogger> getInstance();
            
            ConsoleLogger& operator=(const ConsoleLogger&) = delete;
            ~ConsoleLogger() override = default;
            
            void print(const std::string &message) final;
        
        private:
            ConsoleLogger() = default;
    };
};
