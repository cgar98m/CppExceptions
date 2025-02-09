#include "logger/ConsoleLogger.h"

#include <iostream>

namespace Logger
{
    ////////////////////
    // ConsoleLogger  //
    ////////////////////

    std::shared_ptr<ConsoleLogger> ConsoleLogger::getInstance()
    {
        std::lock_guard<std::mutex> l_oLock(muxInstance);
        if (!consoleLogger) consoleLogger.reset(new ConsoleLogger());
        return consoleLogger;
    }

    void ConsoleLogger::print(const std::string &message)
    {
        std::cout << message << std::endl;
    }

    std::shared_ptr<ConsoleLogger> ConsoleLogger::consoleLogger;
    std::mutex ConsoleLogger::muxInstance;
};
