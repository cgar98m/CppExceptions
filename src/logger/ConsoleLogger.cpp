#include "logger/ConsoleLogger.h"

#include <iostream>

namespace Logger
{
    //////////////////////////
    // Logger para consola  //
    //////////////////////////

    const std::string ConsoleLogger::MUX_PREFIX  = STDCOUT_MUX_PREFIX;
    const DWORD       ConsoleLogger::MUX_TIMEOUT = BasicLogger::MUX_TIMEOUT;
    
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
        std::lock_guard<std::mutex> lockPrint(printMutexMux);
        if (printMutex) CloseHandle(printMutex);

        std::lock_guard<std::mutex> lockLogger(muxInstance);
        if (consoleLogger) consoleLogger.reset();
    }

    ConsoleLogger::ConsoleLogger()
        : IThreadedLogger()
        , printMutex(CreateMutex(nullptr, FALSE, MUX_PREFIX.c_str()))
    {
    }

    bool ConsoleLogger::printEnqueued(const LogMsg &message)
    {
        if (message.text.empty()) return true;

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

        if (WaitForSingleObject(localPrintMutex, MUX_TIMEOUT) != WAIT_OBJECT_0) return false;

        std::cout << "[" << message.processId << "]: ";
        for (size_t idx = 0; idx < message.text.size(); idx += LOGGER_BUFFER_SIZE)
        {
            std::cout << std::string(message.text, idx, LOGGER_BUFFER_SIZE);
        }
        std::cout << std::endl;
        
        ReleaseMutex(localPrintMutex);
        CloseHandle(localPrintMutex);
        return true;
    }
};
