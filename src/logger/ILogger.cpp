#include "logger/ILogger.h"

#include <windows.h>
#include <iostream>
#include <iomanip>

namespace Logger
{
    ////////////////////////////
    // Interfaz de un logger  //
    ////////////////////////////
    
    bool ILogger::print(const std::string& message)
    {
        return printEnqueued(message);
    }

    bool ILogger::printEnqueued(const std::string& message)
    {
        (void) message;
        return false;
    }

    ////////////////////////////////////////////////
    // Interfaz de un logger por salida estandar  //
    ////////////////////////////////////////////////

    Logger     BasicLogger::basicLogger;
    std::mutex BasicLogger::muxInstance;

    Logger BasicLogger::getInstance()
    {
        std::lock_guard<std::mutex> lock(muxInstance);
        if (!basicLogger) basicLogger.reset(new BasicLogger());
        return basicLogger;
    }

    BasicLogger::~BasicLogger()
    {
        std::lock_guard<std::mutex> lock(printMutexMux);
        if (printMutex) CloseHandle(printMutex);
    }

    bool BasicLogger::print(const std::string& message)
    {
        return printEnqueued(message);
    }

    BasicLogger::BasicLogger()
        : ILogger()
        , printMutex(CreateMutex(nullptr, FALSE, LOGGER_STANDARD_OUTPUT_MUX_NAME.c_str()))
    {
    }

    bool BasicLogger::printEnqueued(const std::string& message)
    {
        HANDLE localPrintMutex = nullptr;
        {
            std::lock_guard<std::mutex> lock(printMutexMux);
            if (!printMutex) return false;

            HANDLE processHandle = GetCurrentProcess();
            if (!DuplicateHandle(processHandle
                , printMutex
                , processHandle
                , &localPrintMutex
                , 0
                , FALSE
                , DUPLICATE_SAME_ACCESS))
            return false;
        }
        if (!localPrintMutex) return false;

        if (WaitForSingleObject(localPrintMutex, LOGGER_STANDARD_OUTPUT_MUX_TIMEOUT) != WAIT_OBJECT_0) return false;

        for (size_t idx = 0; idx < message.size(); idx += LOGGER_BUFFER_SIZE)
        {
            std::cout << std::string(message, idx, LOGGER_BUFFER_SIZE) << std::flush;
        }
        std::cout << std::endl;
        
        ReleaseMutex(localPrintMutex);
        CloseHandle(localPrintMutex);
        return true;
    }

    //////////////
    // LogEntry //
    //////////////

    LogEntry::LogEntry(const Logger      &logger,
                       const std::string &func,
                       const std::string &file,
                       size_t            line)
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
        if (!logger->print(ssMessage.str())) std::cout << "TRAZA PERDIDA" << std::endl;
    }

    ////////////////////////////////////////////////
    // Interfaz de una clase que tiene un logger  //
    ////////////////////////////////////////////////

    ILoggerHolder::ILoggerHolder(const Logger& logger)
        : logger(logger)
    {
    }

    const Logger& ILoggerHolder::getLogger() const
    {
        return logger;
    }
};
