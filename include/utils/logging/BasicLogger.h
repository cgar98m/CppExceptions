#pragma once

#include <windows.h>
#include <mutex>
#include "utils/logging/ILogger.h"
#include "utils/logging/LogMsg.h"

//////////////
// Defines  //
//////////////

#define BASIC_LOGGER() Utils::Logging::BasicLogger::getInstance()

namespace Utils
{
    namespace Logging
    {
        //////////////////////////////////
        // Logger para salida estandar  //
        //////////////////////////////////

        class BasicLogger: public ILogger
        {
            // Constantes
            public:
                static const char  *MUX_NAME;
                static const DWORD TIMEOUT_MS_MUX_WAIT;
    
            // Constructor/Destructor
            public:
                virtual ~BasicLogger();
            
            private:
                BasicLogger();
            
            // Deleted
            public:
                explicit BasicLogger(const BasicLogger&)   = delete;
                BasicLogger &operator=(const BasicLogger&) = delete;
            
            // Final virtual
            public:
                virtual bool printRequest(const LogMsg &message) final;
            
            private:
                virtual bool processPrint(const LogMsg &message) final;

            // Funciones de clase
            public:
                static SharedLogger getInstance();
                
            // Variables de clase
            private:
                static SharedLogger basicLogger;
                static std::mutex   instanceMux;
                
            // Variables miembro
            private:
                HANDLE     ostreamMux = nullptr;
                std::mutex internalMux;
        };
    };
};
