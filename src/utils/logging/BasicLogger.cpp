#include "utils/logging/BasicLogger.h"

#include <iostream>

namespace Utils
{
    ////////////////////////////////////////////////
    // Interfaz de un logger por salida estandar  //
    ////////////////////////////////////////////////

    const char  *BasicLogger::MUX_NAME   = "Utils/Logging/StdCoutMutex";
    const DWORD BasicLogger::MUX_TIMEOUT = 1000;

    Logger     BasicLogger::basicLogger;
    std::mutex BasicLogger::instanceMux;

    Logger BasicLogger::getInstance()
    {
        std::lock_guard<std::mutex> lock(instanceMux);
        if (!basicLogger) basicLogger.reset(new BasicLogger());
        return basicLogger;
    }

    BasicLogger::~BasicLogger()
    {
        std::lock_guard<std::mutex> lock(internalMux);
        if (ostreamMux)
        {
            CloseHandle(ostreamMux);
            ostreamMux = nullptr;
        }
    }

    bool BasicLogger::print(const LogMsg &message)
    {
        return printEnqueued(message);
    }

    BasicLogger::BasicLogger()
        : ILogger()
        , ostreamMux(CreateMutex(nullptr, FALSE, MUX_NAME))
    {
    }

    bool BasicLogger::printEnqueued(const LogMsg &message)
    {
        HANDLE localPrintMutex = nullptr;
        {
            std::lock_guard<std::mutex> lock(internalMux);
            if (!ostreamMux) return false;

            HANDLE processHandle = GetCurrentProcess();
            if (!DuplicateHandle(processHandle
                , ostreamMux
                , processHandle
                , &localPrintMutex
                , 0
                , FALSE
                , DUPLICATE_SAME_ACCESS))
            {
                return false;
            }
        }
        if (!localPrintMutex) return false;

        if (WaitForSingleObject(localPrintMutex, MUX_TIMEOUT) != WAIT_OBJECT_0) return false;

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
