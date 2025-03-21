#pragma once

#include <windows.h>
#include <string>

namespace Utils
{
    // Constantes globales
    const size_t LOGGER_PRINT_BUFFER_SIZE = 1024;       // 1KB
    const size_t LOGGER_PRINT_LIMIT_SIZE  = 1024 * 16;  // 16KB
    const DWORD  LOGGER_FLUSH_TIMEOUT     = 2000;       // 2s

    // Informacion del nivel de log
    class LogLevel
    {
        private:
            static const char *LOG_LEVEL_ERROR_DESCRIPTION;
            static const char *LOG_LEVEL_WARNING_DESCRIPTION;
            static const char *LOG_LEVEL_INFO_DESCRIPTION;
            static const char *LOG_LEVEL_DEBUG_DESCRIPTION;
            static const char *LOG_LEVEL_VERBOSE_DESCRIPTION;
            static const char *LOG_LEVEL_UNKNOWN_DESCRIPTION;

        public:
            enum class Level : BYTE
            {
                LEVEL_ERROR = 0,
                LEVEL_WARNING,
                LEVEL_INFO,
                LEVEL_DEBUG,
                LEVEL_VERBOSE
            };

        public:
            static std::string getLogLevelDescription(LogLevel::Level logLevel);
    };

    // Datos necesarios de una traza
    struct LogMsg
    {
        std::string     text;
        SYSTEMTIME      date      = {};
        DWORD           processId = GetCurrentProcessId();
        DWORD           threadId  = GetCurrentThreadId();
        LogLevel::Level logLevel  = LogLevel::Level::LEVEL_INFO;
    };
};
