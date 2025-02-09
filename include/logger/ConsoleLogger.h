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
        public:
            static std::shared_ptr<ConsoleLogger> getInstance();
            
            ~ConsoleLogger() = default;
            
            void print(const std::string &message) override;
        
        private:
            ConsoleLogger() = default;
            ConsoleLogger& operator=(const ConsoleLogger&) = delete;

            // Variables
            static std::shared_ptr<ConsoleLogger> consoleLogger;
            static std::mutex muxInstance;

    };
};
