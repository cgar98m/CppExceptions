#pragma once

#include <memory>
#include "utils/logging/LogLevel.h"
#include "utils/logging/LogMsg.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////
        // Interfaz de un logger  //
        ////////////////////////////
        
        class ILogger
        {
            // Constructor/Destructor
            public:
                ILogger()          = default;
                virtual ~ILogger() = default;
            
            // Deleted
            public:
                ILogger(const ILogger&)            = delete;
                ILogger &operator=(const ILogger&) = delete;
            
            // Pure virtual
            private:
                virtual bool processPrint(const LogMsg &message) = 0;

            // Virtual
            public:
                virtual bool printRequest(const LogMsg &message);

            // Funciones miembro
            public:
                LogLevel::Level getLogLevel() const;
                void setLogLevel(LogLevel::Level logLevel);
    
            // Variables miembro
            private:
                LogLevel::Level logLevel = LogLevel::Level::LEVEL_INFO;
        };

    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using SharedLogger = std::shared_ptr<Utils::Logging::ILogger>;
