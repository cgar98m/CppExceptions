#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////////////
        // Interfaz de una clase que tiene un logger  //
        ////////////////////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        LoggerHolder::LoggerHolder(const SharedLogger &logger)
            : logger(logger)
        {
        }

        //--------------------//
        // Variables miembro  //
        //--------------------//
        
        const SharedLogger &LoggerHolder::getLogger() const
        {
            return this->logger;
        }
    };
};
