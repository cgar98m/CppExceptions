#include "utils/logging/ConsoleLogger.h"

#include <iostream>
#include "utils/logging/BasicLogger.h"

namespace Utils
{
    //////////////////////////
    // Logger para consola  //
    //////////////////////////

    const char  *ConsoleLogger::MUX_NAME   = BasicLogger::MUX_NAME;
    const DWORD ConsoleLogger::MUX_TIMEOUT = BasicLogger::MUX_TIMEOUT;
    
    Logger     ConsoleLogger::consoleLogger;
    std::mutex ConsoleLogger::instanceMux;

    Logger ConsoleLogger::getInstance(const Logger& errorLogger)
    {
        std::lock_guard<std::mutex> lock(instanceMux);
        if (!consoleLogger) consoleLogger.reset(new ConsoleLogger(errorLogger));
        return consoleLogger;
    }

    ConsoleLogger::~ConsoleLogger()
    {
        std::lock_guard<std::mutex> lockPrint(internalMux);
        if (ostreamMux)
        {
            CloseHandle(ostreamMux);
            ostreamMux = nullptr;
        }

        std::lock_guard<std::mutex> lockLogger(instanceMux);
        if (consoleLogger) consoleLogger.reset();
    }

    ConsoleLogger::ConsoleLogger(const Logger& errorLogger)
        : IThreadedLogger(errorLogger)
        , ILoggerHolder(errorLogger)
        , ostreamMux(CreateMutex(nullptr, FALSE, MUX_NAME))
    {
    }

    bool ConsoleLogger::printEnqueued(const LogMsg &message)
    {
        if (message.text.empty()) return true;

        HANDLE localPrintMutex = nullptr;
        {
            std::lock_guard<std::mutex> lock(internalMux);
            if (!ostreamMux)
            {
                LOGGER_THIS_LOG_INFO() << "Mutex NO inicializado";
                return false;
            }

            HANDLE processHandle = GetCurrentProcess();
            if (!DuplicateHandle(processHandle
                , ostreamMux
                , processHandle
                , &localPrintMutex
                , 0
                , FALSE
                , DUPLICATE_SAME_ACCESS))
            {
                LOGGER_THIS_LOG_INFO() << "Mutex NO duplicado: " << GetLastError();
                return false;
            }
        }
        if (!localPrintMutex)
        {
            LOGGER_THIS_LOG_INFO() << "Mutex NO valido";
            return false;
        }

        DWORD waitResult = WaitForSingleObject(localPrintMutex, MUX_TIMEOUT);
        if (waitResult != WAIT_OBJECT_0)
        {
            if (waitResult == WAIT_TIMEOUT) LOGGER_THIS_LOG_INFO() << "TIMEOUT esperando mutex";
            else                            LOGGER_THIS_LOG_INFO() << "ERROR esperando mutex: " << GetLastError();
            return false;
        }

        // Limitamos el tamano del texto
        std::string shortenedText(message.text, 0, LOGGER_PRINT_LIMIT_SIZE);

        // Printamos el mensaje
        for (size_t idx = 0; idx < shortenedText.size(); idx += LOGGER_PRINT_BUFFER_SIZE)
        {
            std::cout << std::string(shortenedText, idx, LOGGER_PRINT_BUFFER_SIZE);
        }
        std::cout << std::endl;
        
        ReleaseMutex(localPrintMutex);
        CloseHandle(localPrintMutex);
        return true;
    }
};
