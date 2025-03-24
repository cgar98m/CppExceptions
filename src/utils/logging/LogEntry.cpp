#include "utils/logging/LogEntry.h"

#include <windows.h>
#include <iomanip>
#include "utils/logging/LogTypes.h"
#include "utils/filesystem/FileTools.h"
#include "utils/text/TextTools.h"

namespace Utils
{
    namespace Logging
    {
        ////////////////////////////////////////////
        // Entrada de un log (NO es thread safe)  //
        ////////////////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        LogEntry::LogEntry(const SharedLogger &logger
                         , LogLevel::Level    logLevel
                         , const std::string  &func
                         , const std::string  &file
                         , size_t             line)
            : logger(logger)
            , func(Text::TextTools::getUndecoratedFunctionName(func))
            , file(FileSystem::FileTools::getUndecoratedFileName(file))
            , line(line)
        {
            // Actualizamos los datos esenciales del mensaje
            GetLocalTime(&this->message.date);
            this->message.logLevel = logLevel;
        }

        LogEntry::~LogEntry()
        {
            this->flush();
        }

        //--------------------//
        // Variables miembro  //
        //--------------------//

        std::ostream &LogEntry::operator()()
        {
            return this->sstream;
        }

        void LogEntry::flush()
        {
            // Verificamos que tengamos un logger valido
            if (!this->logger) return;

            // Verificamos el nivel de log
            if (this->message.logLevel > this->logger->getLogLevel()) return;

            // Verificamos que exista contenido
            std::string messageText(this->sstream.str());
            if (messageText.empty()) return;

            // Montamos la fecha
            std::stringstream ssMessage;
            ssMessage << std::setfill('0')                 << this->message.date.wYear         << "/";
            ssMessage << std::setfill('0') << std::setw(2) << this->message.date.wMonth        << "/";
            ssMessage << std::setfill('0') << std::setw(2) << this->message.date.wDay          << " ";
            ssMessage << std::setfill('0') << std::setw(2) << this->message.date.wHour         << ":";
            ssMessage << std::setfill('0') << std::setw(2) << this->message.date.wMinute       << ":";
            ssMessage << std::setfill('0') << std::setw(2) << this->message.date.wSecond       << ".";
            ssMessage << std::setfill('0') << std::setw(3) << this->message.date.wMilliseconds << " ";

            // Montamos el nivel de log
            ssMessage << "[" << LogLevel::getLogLevelDescription(this->message.logLevel) << "]";

            // Montamos la identificacion del proceso e hilo
            ssMessage << "[" << this->message.processId << "|" << this->message.threadId << "] ";

            // Montamos el contexto
            ssMessage << this->file << ":" << this->line << ": ";
            if (!this->func.empty()) ssMessage << this->func << ": ";

            // Montamos el mensaje final (limitando su tamano)
            ssMessage << messageText;
            this->message.text = std::string(ssMessage.str(), 0, LOGGER_PRINT_LIMIT_SIZE);

            // Pintamos el mensaje
            this->logger->printRequest(this->message);
        }
    };
};
