#pragma once

#include <ostream>
#include <memory>
#include <sstream>
#include <string>

#define LOGGER_LOG(logger) Logger::LogEntry((logger), (__func__), __FILE__, __LINE__)()

namespace Logger
{
    // Interfaz de un logger
    class ILogger
    {
        public:
            ILogger() = default;
            ~ILogger() = default;

            virtual void print(const std::string &message) = 0;

        private:
            ILogger& operator=(const ILogger&) = delete;
    };

    // Entrada de log (NO es thread safe)
    class LogEntry
    {
        public:
            explicit LogEntry(const std::shared_ptr<ILogger> logger,
                              const std::string              &func = std::string(),
                              const std::string              &file = __FILE__,
                              size_t                         line  = __LINE__);
            ~LogEntry();

            std::ostream& operator()();

        private:
            LogEntry() = delete;
            LogEntry& operator=(const LogEntry&) = delete;

            void flush();

            // Variables
            std::shared_ptr<ILogger> logger;
            std::string              func;
            std::string              file;
            size_t                   line;

            std::stringstream        sstream;
    };
};
