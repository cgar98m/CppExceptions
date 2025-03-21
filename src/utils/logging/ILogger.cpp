#include "utils/logging/ILogger.h"

#include <iomanip>

namespace Utils
{
    ////////////////////////////
    // Interfaz de un logger  //
    ////////////////////////////

    LogLevel::Level ILogger::getLogLevel() const
    {
        return logLevel;
    }
    
    bool ILogger::print(const LogMsg &message)
    {
        return this->printEnqueued(message);
    }

    //////////////
    // LogEntry //
    //////////////

    LogEntry::LogEntry(const Logger      &logger,
                       LogLevel::Level   logLevel,
                       const std::string &func,
                       const std::string &file,
                       size_t            line)
        : logger(logger)
        , func(func)
        , file(file)
        , line(line)
    {
        // Actualizamos los datos esenciales del mensaje
        GetLocalTime(&message.date);
        message.logLevel = logLevel;

        // Nos quedamos con el nombre del fichero, quitando el resto
        if (!file.empty())
        {
            size_t pos = this->file.find_last_of("\\/");
            if (pos != std::string::npos && pos < this->file.size() - 1) this->file = this->file.substr(pos + 1);
        }
    }

    LogEntry::~LogEntry()
    {
        flush();
    }
    
    std::ostream &LogEntry::operator()()
    {
        return sstream;
    }

    void LogEntry::flush()
    {
        // Verificamos que tengamos un logger valido
        if (!logger) return;

        // Verificamos el nivel de log
        if (message.logLevel > logger->getLogLevel()) return;

        // Verificamos que exista contenido
        std::string messageText(sstream.str());
        if (messageText.empty()) return;

        // Montamos la fecha
        std::stringstream ssMessage;
        ssMessage << std::setfill('0')                 << message.date.wYear         << "/";
        ssMessage << std::setfill('0') << std::setw(2) << message.date.wMonth        << "/";
        ssMessage << std::setfill('0') << std::setw(2) << message.date.wDay          << " ";
        ssMessage << std::setfill('0') << std::setw(2) << message.date.wHour         << ":";
        ssMessage << std::setfill('0') << std::setw(2) << message.date.wMinute       << ":";
        ssMessage << std::setfill('0') << std::setw(2) << message.date.wSecond       << ".";
        ssMessage << std::setfill('0') << std::setw(3) << message.date.wMilliseconds << " ";

        // Montamos el nivel de log
        ssMessage << "[" << LogLevel::getLogLevelDescription(message.logLevel) << "]";

        // Montamos la identificacion del proceso e hilo
        ssMessage << "[" << message.processId << "|" << message.threadId << "] ";

        // Montamos el contexto
        ssMessage << file << ":" << line << ": ";
        if (!func.empty()) ssMessage << func << ": ";

        // Montamos el mensaje final (limitando su tamano)
        ssMessage << messageText;
        message.text = std::string(ssMessage.str(), 0, LOGGER_PRINT_LIMIT_SIZE);

        // Pintamos el mensaje
        logger->print(message);
    }

    ////////////////////////////////////////////////
    // Interfaz de una clase que tiene un logger  //
    ////////////////////////////////////////////////

    ILoggerHolder::ILoggerHolder(const Logger &logger)
        : logger(logger)
    {
    }

    const Logger &ILoggerHolder::getLogger() const
    {
        return logger;
    }
};
