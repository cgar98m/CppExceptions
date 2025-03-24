#include "utils/logging/LogLevel.h"

namespace Utils
{
    namespace Logging
    {
        //////////////////////////////////
        // Informacion del nivel de log //
        //////////////////////////////////

        //------------//
        // Constantes //
        //------------//
        
        const char *LogLevel::LOG_LEVEL_ERROR_DESCRIPTION   = "ERROR";
        const char *LogLevel::LOG_LEVEL_WARNING_DESCRIPTION = "WARN ";
        const char *LogLevel::LOG_LEVEL_INFO_DESCRIPTION    = "INFO ";
        const char *LogLevel::LOG_LEVEL_DEBUG_DESCRIPTION   = "DEBUG";
        const char *LogLevel::LOG_LEVEL_VERBOSE_DESCRIPTION = "VERB ";
        const char *LogLevel::LOG_LEVEL_UNKNOWN_DESCRIPTION = "?????";
        
        //--------------------//
        // Funciones de clase //
        //--------------------//

        std::string LogLevel::getLogLevelDescription(Level logLevel)
        {
            switch (logLevel)
            {
                case Level::LEVEL_ERROR:   return LOG_LEVEL_ERROR_DESCRIPTION;
                case Level::LEVEL_WARNING: return LOG_LEVEL_WARNING_DESCRIPTION;
                case Level::LEVEL_INFO:    return LOG_LEVEL_INFO_DESCRIPTION;
                case Level::LEVEL_DEBUG:   return LOG_LEVEL_DEBUG_DESCRIPTION;
                case Level::LEVEL_VERBOSE: return LOG_LEVEL_VERBOSE_DESCRIPTION;
                default:                   break;
            }
    
            return LOG_LEVEL_UNKNOWN_DESCRIPTION;
        }
    };
};
