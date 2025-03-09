#include "logger/ConsoleLogger.h"

#include <iostream>

namespace Logger
{
    ////////////////////
    // ConsoleLogger  //
    ////////////////////
    
    Logger     ConsoleLogger::consoleLogger;
    std::mutex ConsoleLogger::muxInstance;

    Logger ConsoleLogger::getInstance()
    {
        std::lock_guard<std::mutex> lock(muxInstance);
        if (!consoleLogger) consoleLogger.reset(new ConsoleLogger());
        return consoleLogger;
    }

    void ConsoleLogger::print(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(printMutex);
        std::cout << message << std::endl;
    }

};
