#pragma once

#include "utils/logging/ILogger.h"

//////////////
// Defines  //
//////////////

#define THIS_LOGGER() this->getLogger()

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////////////
        // Interfaz de una clase que tiene un logger  //
        ////////////////////////////////////////////////

        class LoggerHolder
        {
            // Constructor/Destructor
            public:
                explicit LoggerHolder(const SharedLogger &logger);
                virtual ~LoggerHolder() = default;
            
            // Deleted
            public:
                LoggerHolder()                               = delete;
                LoggerHolder &operator=(const SharedLogger&) = delete;

            // Funciones miembro
            protected:
                const SharedLogger &getLogger() const;

            // Variables miembro
            private:
                SharedLogger logger;
        };
    };
};
