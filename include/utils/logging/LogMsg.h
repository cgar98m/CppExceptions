#pragma once

#include <windows.h>
#include <string>
#include "utils/logging/LogLevel.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////
        // Datos necesarios de una traza  //
        ////////////////////////////////////
        
        struct LogMsg
        {
            // Variables miembro
            public:
                std::string     text;
                SYSTEMTIME      date      = {};
                DWORD           processId = GetCurrentProcessId();
                DWORD           threadId  = GetCurrentThreadId();
                LogLevel::Level logLevel  = LogLevel::Level::LEVEL_INFO;
        };
    };
};
