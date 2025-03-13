#pragma once

#include <windows.h>
#include <ostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>


namespace Logger
{
    // Defines globales (no les afecta el namespace)
    #define LOGGER_LOG(logger) Logger::LogEntry((logger), (__func__), __FILE__, __LINE__)()
    #define LOGGER_THIS_LOG() LOGGER_LOG(getLogger())

    #define STDCOUT_MUX_PREFIX std::string("StdCoutMutex")
    
    // Constantes globales
    const size_t LOGGER_BUFFER_SIZE   = 1024;
    const DWORD  LOGGER_FLUSH_TIMEOUT = 2000;
    
    // Datos necesarios de una traza
    struct LogMsg
    {
        SYSTEMTIME  date = {};
        std::string text;
    };

    // Interfaz de un logger
    class ILogger
    {
        public:
            ILogger() = default;
            ILogger(const ILogger&) = delete;
            ILogger& operator=(const ILogger&) = delete;
            virtual ~ILogger() = default;

            virtual bool print(const LogMsg &message);

        private:
            virtual bool printEnqueued(const LogMsg &message);
    };
    using Logger = std::shared_ptr<ILogger>;

    // Interfaz de un logger por salida estandar
    class BasicLogger: public ILogger
    {
        public:
            static const std::string MUX_PREFIX;
            static const DWORD       MUX_TIMEOUT;

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
            
            bool print(const LogMsg &message) final;
            
        private:
            BasicLogger();

            bool printEnqueued(const LogMsg &message) final;
    };

    // Entrada de log (NO es thread safe)
    class LogEntry
    {
        private:
            Logger logger;

            SYSTEMTIME  date = {};
            std::string func;
            std::string file;
            size_t      line;

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
            explicit ILoggerHolder(const Logger &logger);
            ILoggerHolder& operator=(const Logger&) = delete;
            virtual ~ILoggerHolder() = default;

        protected:
            virtual const Logger& getLogger() const;
    };
};
