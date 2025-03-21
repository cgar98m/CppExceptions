#pragma once

#include <windows.h>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include "utils/logging/LogTypes.h"

// Defines globales
#define LOGGER_LOG_X(logger, logLevel) Utils::LogEntry((logger), (logLevel), __func__, __FILE__, __LINE__)()
#define LOGGER_LOG_ERROR(logger)   LOGGER_LOG_X((logger), Utils::LogLevel::Level::LEVEL_ERROR)
#define LOGGER_LOG_WARNING(logger) LOGGER_LOG_X((logger), Utils::LogLevel::Level::LEVEL_WARNING)
#define LOGGER_LOG_INFO(logger)    LOGGER_LOG_X((logger), Utils::LogLevel::Level::LEVEL_INFO)
#define LOGGER_LOG_DEBUG(logger)   LOGGER_LOG_X((logger), Utils::LogLevel::Level::LEVEL_DEBUG)
#define LOGGER_LOG_VERBOSE(logger) LOGGER_LOG_X((logger), Utils::LogLevel::Level::LEVEL_VERBOSE)

#define LOGGER_THIS_LOG_X(logLevel) LOGGER_LOG_X(getLogger(), (logLevel))
#define LOGGER_THIS_LOG_ERROR()   LOGGER_THIS_LOG_X(Utils::LogLevel::Level::LEVEL_ERROR)
#define LOGGER_THIS_LOG_WARNING() LOGGER_THIS_LOG_X(Utils::LogLevel::Level::LEVEL_WARNING)
#define LOGGER_THIS_LOG_INFO()    LOGGER_THIS_LOG_X(Utils::LogLevel::Level::LEVEL_INFO)
#define LOGGER_THIS_LOG_DEBUG()   LOGGER_THIS_LOG_X(Utils::LogLevel::Level::LEVEL_DEBUG)
#define LOGGER_THIS_LOG_VERBOSE() LOGGER_THIS_LOG_X(Utils::LogLevel::Level::LEVEL_VERBOSE)

namespace Utils
{
    // Interfaz de un logger
    class ILogger
    {
        private:
            LogLevel::Level logLevel = LogLevel::Level::LEVEL_INFO;

        public:
            ILogger() = default;
            ILogger(const ILogger&) = delete;
            ILogger& operator=(const ILogger&) = delete;
            virtual ~ILogger() = default;

            LogLevel::Level getLogLevel() const;

            virtual bool print(const LogMsg &message);

        private:
            virtual bool printEnqueued(const LogMsg &message) = 0;
    };
    using Logger = std::shared_ptr<ILogger>;

    // Entrada de un log (NO es thread safe)
    class LogEntry
    {
        private:
            Logger logger;

            LogMsg      message;
            std::string func;
            std::string file;
            size_t      line = 0;

            std::stringstream sstream;

        public:
            LogEntry() = delete;
            explicit LogEntry(const Logger      &logger,
                              LogLevel::Level   logLevel = LogLevel::Level::LEVEL_INFO,
                              const std::string &func    = std::string(),
                              const std::string &file    = __FILE__,
                              size_t            line     = __LINE__);
            LogEntry& operator=(const LogEntry&) = delete;
            ~LogEntry();

            std::ostream &operator()();

        private:
            void flush();
    };

    // Interfaz de una clase que tiene un logger
    class ILoggerHolder
    {
        private:
            Logger logger;

        public:
            ILoggerHolder() = delete;
            explicit ILoggerHolder(const Logger &logger);
            ILoggerHolder& operator=(const Logger&) = delete;
            virtual ~ILoggerHolder() = default;

        protected:
            virtual const Logger &getLogger() const;
    };
};
