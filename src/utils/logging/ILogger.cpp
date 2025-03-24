#include "utils/logging/ILogger.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////
        // Interfaz de un logger  //
        ////////////////////////////

        //----------//
        // Virtual  //
        //----------//

        bool ILogger::printRequest(const LogMsg &message)
        {
            return this->processPrint(message);
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        LogLevel::Level ILogger::getLogLevel() const
        {
            return this->logLevel;
        }
        
        void ILogger::setLogLevel(LogLevel::Level logLevel)
        {
            this->logLevel = logLevel;
        }
    };
};
