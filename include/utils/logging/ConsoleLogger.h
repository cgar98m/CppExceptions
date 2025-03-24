#pragma once

#include <windows.h>
#include <mutex>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/IThreadedLogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/logging/LogMsg.h"

//////////////
// Defines  //
//////////////

#define CONSOLE_LOGGER() Utils::Logging::ConsoleLogger::getInstance()

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////////////////////
        // Logger para salida estandar con impresion por hilo //
        ////////////////////////////////////////////////////////
        
        class ConsoleLogger: public IThreadedLogger
        {
            // Constantes
            public:
                static const char  *MUX_NAME;
                static const DWORD TIMEOUT_MS_MUX_WAIT;
            
            private:
                static const char *LOGGER_NAME;
    
            // Constructor/Destructor
            public:
                virtual ~ConsoleLogger()
                {
                }
                
            private:
                explicit ConsoleLogger(const SharedLogger &logger = BASIC_LOGGER());

            // Deleted
            public:
                ConsoleLogger()                                = delete;
                explicit ConsoleLogger(const ConsoleLogger&)   = delete;
                ConsoleLogger &operator=(const ConsoleLogger&) = delete;

            // Final virtual
            private:
                virtual bool validateStream(const LogMsg &message)            final;
                virtual bool processPrintToStream(const std::string &message) final;

            // Funciones de clase
            public:
                static SharedLogger getInstance(const SharedLogger &logger = BASIC_LOGGER());

            // Variables de clase
            private:
                static SharedLogger consoleLogger;
                static std::mutex   instanceMux;
        };
    };
};
