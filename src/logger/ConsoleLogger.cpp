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

    ConsoleLogger::~ConsoleLogger()
    {
        std::lock_guard<std::mutex> lock(printMutexMux);
        if (printMutex) CloseHandle(printMutex);
    }

    ConsoleLogger::ConsoleLogger()
        : IThreadedLogger()
        , printMutex(CreateMutex(nullptr, FALSE, LOGGER_STANDARD_OUTPUT_MUX_NAME.c_str()))
    {
    }

    bool ConsoleLogger::printEnqueued(const std::string &message)
    {
        HANDLE localPrintMutex = nullptr;
        {
            std::lock_guard<std::mutex> lock(printMutexMux);
            if (!printMutex) return false;

            HANDLE processHandle = GetCurrentProcess();
            if (!DuplicateHandle(processHandle
                , printMutex
                , processHandle
                , &localPrintMutex
                , 0
                , FALSE
                , DUPLICATE_SAME_ACCESS))
            return false;
        }
        if (!localPrintMutex) return false;

        if (WaitForSingleObject(localPrintMutex, LOGGER_STANDARD_OUTPUT_MUX_TIMEOUT) != WAIT_OBJECT_0) return false;

        for (size_t idx = 0; idx < message.size(); idx += LOGGER_BUFFER_SIZE)
        {
            std::cout << std::string(message, idx, LOGGER_BUFFER_SIZE) << std::flush;
        }
        std::cout << std::endl;
        
        ReleaseMutex(localPrintMutex);
        CloseHandle(localPrintMutex);
        return true;
    }
};
