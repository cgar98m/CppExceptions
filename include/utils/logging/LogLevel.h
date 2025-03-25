#pragma once

#include <windows.h>
#include <string>

namespace Utils
{
    namespace Logging
    {
        //////////////////////////////////
        // Informacion del nivel de log //
        //////////////////////////////////
        
        class LogLevel
        {
            // Constantes
            private:
                static const char *LOG_LEVEL_ERROR_DESCRIPTION;
                static const char *LOG_LEVEL_WARNING_DESCRIPTION;
                static const char *LOG_LEVEL_INFO_DESCRIPTION;
                static const char *LOG_LEVEL_DEBUG_DESCRIPTION;
                static const char *LOG_LEVEL_VERBOSE_DESCRIPTION;
                static const char *LOG_LEVEL_UNKNOWN_DESCRIPTION;

            // Tipos, estructuras y enums
            public:
                enum class Level: BYTE
                {
                    LEVEL_ERROR = 0,
                    LEVEL_WARNING,
                    LEVEL_INFO,
                    LEVEL_DEBUG,
                    LEVEL_VERBOSE
                };
            
            // Funciones de clase
            public:
                static std::string getLogLevelDescription(Level logLevel);
        };
    };
};
