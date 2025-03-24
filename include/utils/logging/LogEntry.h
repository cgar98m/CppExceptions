#pragma once

#include <ostream>
#include <sstream>
#include <string>
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/logging/LogLevel.h"
#include "utils/logging/LogMsg.h"

//////////////
// Defines  //
//////////////

#define LOGGER_LOG_X(logger, logLevel) Utils::Logging::LogEntry((logger), (logLevel), __FUNCTION__, __FILE__, __LINE__)()
#define LOGGER_LOG_ERROR(logger)   LOGGER_LOG_X((logger), Utils::Logging::LogLevel::Level::LEVEL_ERROR)
#define LOGGER_LOG_WARNING(logger) LOGGER_LOG_X((logger), Utils::Logging::LogLevel::Level::LEVEL_WARNING)
#define LOGGER_LOG_INFO(logger)    LOGGER_LOG_X((logger), Utils::Logging::LogLevel::Level::LEVEL_INFO)
#define LOGGER_LOG_DEBUG(logger)   LOGGER_LOG_X((logger), Utils::Logging::LogLevel::Level::LEVEL_DEBUG)
#define LOGGER_LOG_VERBOSE(logger) LOGGER_LOG_X((logger), Utils::Logging::LogLevel::Level::LEVEL_VERBOSE)

#define LOGGER_THIS_LOG_X(logLevel) LOGGER_LOG_X(THIS_LOGGER(), (logLevel))
#define LOGGER_THIS_LOG_ERROR()   LOGGER_THIS_LOG_X(Utils::Logging::LogLevel::Level::LEVEL_ERROR)
#define LOGGER_THIS_LOG_WARNING() LOGGER_THIS_LOG_X(Utils::Logging::LogLevel::Level::LEVEL_WARNING)
#define LOGGER_THIS_LOG_INFO()    LOGGER_THIS_LOG_X(Utils::Logging::LogLevel::Level::LEVEL_INFO)
#define LOGGER_THIS_LOG_DEBUG()   LOGGER_THIS_LOG_X(Utils::Logging::LogLevel::Level::LEVEL_DEBUG)
#define LOGGER_THIS_LOG_VERBOSE() LOGGER_THIS_LOG_X(Utils::Logging::LogLevel::Level::LEVEL_VERBOSE)

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////////
        // Entrada de un log (NO es thread safe)  //
        ////////////////////////////////////////////

        class LogEntry
        {
            // Constructor/Destructor
            public:
                explicit LogEntry(const SharedLogger &logger,
                                  LogLevel::Level    logLevel = LogLevel::Level::LEVEL_INFO,
                                  const std::string  &func    = std::string(),
                                  const std::string  &file    = __FILE__,
                                  size_t             line     = __LINE__);
                virtual ~LogEntry();
            
            // Deleted
            public:
                LogEntry()                           = delete;
                LogEntry &operator=(const LogEntry&) = delete;

            // Funciones miembro
            public:
                std::ostream &operator()();

            private:
                void flush();

            // Variables miembro
            private:
                SharedLogger logger;

                LogMsg      message;
                std::string func;
                std::string file;
                size_t      line = 0;

                std::stringstream sstream;
        };
    };
};
