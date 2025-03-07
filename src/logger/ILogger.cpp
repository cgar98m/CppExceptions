#include "logger/ILogger.h"

#include <windows.h>
#include <iostream>
#include <iomanip>

namespace Logger
{
    //////////////
    // LogEntry //
    //////////////

    LogEntry::LogEntry(const std::shared_ptr<ILogger> logger,
                       const std::string              &func,
                       const std::string              &file,
                       size_t                         line)
        : logger(logger)
        , func(func)
        , file(file)
        , line(line)
    {
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
    
    std::ostream& LogEntry::operator()()
    {
        return sstream;
    }

    void LogEntry::flush()
    {
        // Verificamos que exista contenido
        std::string message(sstream.str());
        if (message.empty()) return;

        // Verificamos que tengamos un logger valido
        if (!logger) return;

        // Montamos el timestamp
        SYSTEMTIME stLocal;
        GetLocalTime(&stLocal);

        std::stringstream ssMessage;
        ssMessage << std::setfill('0')                 << stLocal.wYear         << "/";
        ssMessage << std::setfill('0') << std::setw(2) << stLocal.wMonth        << "/";
        ssMessage << std::setfill('0') << std::setw(2) << stLocal.wDay          << " ";
        ssMessage << std::setfill('0') << std::setw(2) << stLocal.wHour         << ":";
        ssMessage << std::setfill('0') << std::setw(2) << stLocal.wMinute       << ":";
        ssMessage << std::setfill('0') << std::setw(2) << stLocal.wSecond       << ".";
        ssMessage << std::setfill('0') << std::setw(3) << stLocal.wMilliseconds << " ";

        // Montamos el contexto
        ssMessage << file << ":" << line << ": ";
        if (!func.empty()) ssMessage << func << ": ";

        // Montamos el mensaje final
        ssMessage << message;

        // Pintamos el mensaje
        logger->print(ssMessage.str());
    }
};
