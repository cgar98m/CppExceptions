#pragma once

#include <windows.h>
#include <ostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

#define LOGGER_LOG(logger) Logger::LogEntry((logger), (__func__), __FILE__, __LINE__)()
#define LOGGER_THIS_LOG() LOGGER_LOG(getLogger())

namespace Logger
{
    // Constantes globales
    const size_t LOGGER_BUFFER_SIZE = 1024;

    const std::string LOGGER_STANDARD_OUTPUT_MUX_NAME      = "StdCoutMutex";
    const DWORD       LOGGER_STANDARD_OUTPUT_MUX_TIMEOUT   = 1000;
    const DWORD       LOGGER_STANDARD_OUTPUT_FLUSH_TIMEOUT = 1000;
    
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
            
            HANDLE     printMutex;
            std::mutex printMutexMux;

        public:
            static Logger getInstance();

            BasicLogger(const BasicLogger&) = delete;
            BasicLogger& operator=(const BasicLogger&) = delete;
            virtual ~BasicLogger();
            
            bool print(const std::string& message) final;
            
        private:
            BasicLogger();

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
