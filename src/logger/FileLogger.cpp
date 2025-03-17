#include "logger/FileLogger.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include "utils/files/FileTools.h"

namespace Logger
{
    ////////////////////////////
    // Logger para un fichero //
    ////////////////////////////

    const std::string FileLogger::MUX_PREFIX  = "Logger/FileMutex";
    const DWORD       FileLogger::MUX_TIMEOUT = 1000;
    
    FileLogger::LoggerMap FileLogger::fileLoggers;
    std::mutex            FileLogger::muxInstance;

    Logger FileLogger::getInstance(const std::string& fileBaseName, const std::string& fileDir)
    {
        Logger logger;
        if (fileBaseName.empty()) return logger;
        
        std::string loggerPath = Utils::FileTools::getAbsolutePath(fileDir);
        if (loggerPath.empty()) return logger;

        std::string filePath = loggerPath + fileBaseName;

        std::lock_guard<std::mutex> lock(muxInstance);
        if (fileLoggers.find(filePath) == fileLoggers.end())
        {
            logger.reset(new FileLogger(fileBaseName, loggerPath));
            if (logger) fileLoggers[filePath] = logger;
        }
        return logger;
    }

    FileLogger::~FileLogger()
    {
        std::lock_guard<std::mutex> lockPrint(printMutexMux);
        if (printMutex) CloseHandle(printMutex);

        std::lock_guard<std::mutex> lockLoggers(muxInstance);
        fileLoggers.clear();
    }

    FileLogger::FileLogger(const std::string& fileBaseName, const std::string& fileDir)
        : IThreadedLogger()
        , fileBaseName(fileBaseName)
        , fileDir(fileDir)
    {
        std::string muxName = fileDir + fileBaseName;
        std::replace(muxName.begin(), muxName.end(), '\\', '/');
        printMutex = CreateMutex(nullptr, FALSE, muxName.c_str());
    }

    bool FileLogger::printEnqueued(const LogMsg &message)
    {
        if (message.text.empty()) return true;

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

        if (WaitForSingleObject(localPrintMutex, MUX_TIMEOUT) != WAIT_OBJECT_0) return false;

        // Comprobamos el file stream
        bool printResult = false;
        if (validateFileStream())
        {
            *lastFileStream << "[" << message.processId << "]: ";
            for (size_t idx = 0; idx < message.text.size(); idx += LOGGER_BUFFER_SIZE)
            {
                *lastFileStream << std::string(message.text, idx, LOGGER_BUFFER_SIZE);
            }
            *lastFileStream << std::endl;

            if (lastFileStream->good()) printResult = true;
        }

        if (!printResult) lastFileStream.reset();
        
        ReleaseMutex(localPrintMutex);
        CloseHandle(localPrintMutex);
        return printResult;
    }
    
    bool FileLogger::validateFileStream()
    {
        if (fileDir.empty() || fileBaseName.empty()) return false;
        
        // Obtenemos la fecha
        SYSTEMTIME date;
        GetLocalTime(&date);

        // Verificamos si tenemos nombre de fichero previo valido
        if ((!lastFilePath.empty()) &&
            (lastDate.wYear == date.wYear && lastDate.wMonth == date.wMonth && date.wDay == lastDate.wDay) &&
            (lastFileStream && lastFileStream->good()))
        {
            return true;
        }

        // Limpiamos
        lastFilePath = std::string();
        if (lastFileStream) lastFileStream.reset();

        // Obtenemos ruta absoluta
        std::string loggerPath = Utils::FileTools::getAbsolutePath(fileDir);
        if (loggerPath.empty()) return false;

        // Montamos la ruta completa
        std::stringstream ssFilePath;
        ssFilePath << loggerPath;
        ssFilePath << std::setfill('0')                 << date.wYear;
        ssFilePath << std::setfill('0') << std::setw(2) << date.wMonth;
        ssFilePath << std::setfill('0') << std::setw(2) << date.wDay;
        ssFilePath << "_" << fileBaseName << ".log";
        std::string absolutePath = ssFilePath.str();

        // Creamos el file stream
        lastFileStream.reset(new std::ofstream(absolutePath, std::ios_base::app));
        if (!lastFileStream) return false;
        if (!lastFileStream->good())
        {
            lastFileStream.reset();
            return false;
        }

        return true;
    }
};
