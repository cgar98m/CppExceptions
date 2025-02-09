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
            size_t l_uiPos = this->file.find_last_of("\\/");
            if (l_uiPos != std::string::npos && l_uiPos < this->file.size() - 1) this->file = this->file.substr(l_uiPos + 1);
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
        std::string l_sMessage(sstream.str());
        if (l_sMessage.empty()) return;

        // Verificamos que tengamos un logger valido
        if (!logger) return;

        // Montamos el timestamp
        SYSTEMTIME l_stLocal;
        GetLocalTime(&l_stLocal);

        std::stringstream l_ssMessage;
        l_ssMessage << std::setfill('0')                 << l_stLocal.wYear         << "/";
        l_ssMessage << std::setfill('0') << std::setw(2) << l_stLocal.wMonth        << "/";
        l_ssMessage << std::setfill('0') << std::setw(2) << l_stLocal.wDay          << " ";
        l_ssMessage << std::setfill('0') << std::setw(2) << l_stLocal.wHour         << ":";
        l_ssMessage << std::setfill('0') << std::setw(2) << l_stLocal.wMinute       << ":";
        l_ssMessage << std::setfill('0') << std::setw(2) << l_stLocal.wSecond       << ".";
        l_ssMessage << std::setfill('0') << std::setw(3) << l_stLocal.wMilliseconds << " ";

        // Montamos el contexto
        l_ssMessage << file << ":" << line << ": ";
        if (!func.empty()) l_ssMessage << func << ": ";

        // Montamos el mensaje final
        l_ssMessage << l_sMessage;

        // Pintamos el mensaje
        logger->print(l_ssMessage.str());
    }
};
