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

    bool ConsoleLogger::printEnqueued(const std::string &message)
    {
        std::cout << message << std::endl;
        return true;
    }

};
