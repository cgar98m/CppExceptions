#pragma once

#include <ostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#define LOGGER_LOG(logger) Logger::LogEntry((logger), (__func__), __FILE__, __LINE__)()
#define LOGGER_THIS_LOG() LOGGER_LOG(getLogger())

namespace Logger
{
    // Interfaz de un logger
    class ILogger
    {
        public:
            ILogger() = default;
            ILogger(const ILogger&) = delete;
            ILogger& operator=(const ILogger&) = delete;
            virtual ~ILogger() = default;

            virtual bool print(const std::string& message);

        private:
            virtual bool printEnqueued(const std::string& message);
    };
    using Logger = std::shared_ptr<ILogger>;

    // Interfaz de un logger por salida estandar
    class BasicLogger: public ILogger
    {
        private:
            static Logger     basicLogger;
            static std::mutex muxInstance;
            
            std::mutex printMutex;

        public:
            static Logger getInstance();

            BasicLogger(const BasicLogger&) = delete;
            BasicLogger& operator=(const BasicLogger&) = delete;
            virtual ~BasicLogger() = default;
            
            bool print(const std::string& message) final;
            
        private:
            BasicLogger() = default;

            bool printEnqueued(const std::string& message) final;
    };

    // Entrada de log (NO es thread safe)
    class LogEntry
    {
        private:
            Logger            logger;
            std::string       func;
            std::string       file;
            size_t            line;
            std::stringstream sstream;

        public:
            LogEntry() = delete;
            explicit LogEntry(const Logger      &logger,
                              const std::string &func = std::string(),
                              const std::string &file = __FILE__,
                              size_t            line  = __LINE__);
            LogEntry& operator=(const LogEntry&) = delete;
            ~LogEntry();

            std::ostream& operator()();

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
            explicit ILoggerHolder(const Logger& logger);
            ILoggerHolder& operator=(const Logger& logger) = delete;
            virtual ~ILoggerHolder() = default;

        protected:
            virtual const Logger& getLogger() const;
    };
};
