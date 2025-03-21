#include "utils/logging/FileLogger.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include "utils/filesystem/FileTools.h"

namespace Utils
{
    ////////////////////////////
    // Logger para un fichero //
    ////////////////////////////

    const char  *FileLogger::MUX_NAME   = "Utils/Logging/FileMutex";
    const DWORD FileLogger::MUX_TIMEOUT = 1000;
    
    FileLogger::LoggerMap FileLogger::fileLoggers;
    std::mutex            FileLogger::instanceMux;

    Logger FileLogger::getInstance(const std::string &fileBaseName, const std::string &fileDir, const Logger& errorLogger)
    {
        // Obtenemos la ruta del logger
        Logger logger;
        if (fileBaseName.empty())
        {
            LOGGER_LOG_INFO(errorLogger) << "Nombre de fichero NO valido";
            return logger;
        }
        
        std::string loggerPath = Utils::FileTools::getAbsolutePath(fileDir);
        if (loggerPath.empty())
        {
            LOGGER_LOG_INFO(errorLogger) << "Ruta de fichero NO valida: " << GetLastError();
            return logger;
        }

        std::string filePath = loggerPath + fileBaseName;

        // Obtenemos la instancia asociada al fichero
        std::lock_guard<std::mutex> lock(instanceMux);
        if (fileLoggers.find(filePath) == fileLoggers.end())
        {
            logger.reset(new FileLogger(fileBaseName, loggerPath, errorLogger));
            if (logger) fileLoggers[filePath] = logger;
        }
        else
        {
            logger = fileLoggers[filePath];   
        }
        return logger;
    }

    FileLogger::~FileLogger()
    {
        std::lock_guard<std::mutex> lockPrint(internalMux);
        if (ostreamMux)
        {
            CloseHandle(ostreamMux);
            ostreamMux = nullptr;
        }

        std::lock_guard<std::mutex> lockLoggers(instanceMux);
        fileLoggers.clear();
    }

    FileLogger::FileLogger(const std::string &fileBaseName, const std::string &fileDir, const Logger& errorLogger)
        : IThreadedLogger(errorLogger)
        , ILoggerHolder(errorLogger)
        , fileBaseName(fileBaseName)
        , fileDir(fileDir)
    {
        std::string muxName = std::string(MUX_NAME) + std::string("/") + fileDir + fileBaseName;
        std::replace(muxName.begin(), muxName.end(), '\\', '/');
        ostreamMux = CreateMutex(nullptr, FALSE, muxName.c_str());
    }

    bool FileLogger::printEnqueued(const LogMsg &message)
    {
        if (message.text.empty()) return true;

        HANDLE localPrintMutex = nullptr;
        {
            std::lock_guard<std::mutex> lock(internalMux);
            if (!ostreamMux)
            {
                LOGGER_THIS_LOG_INFO() << "Mutex NO inicializado";
                return false;
            }

            HANDLE processHandle = GetCurrentProcess();
            if (!DuplicateHandle(processHandle
                , ostreamMux
                , processHandle
                , &localPrintMutex
                , 0
                , FALSE
                , DUPLICATE_SAME_ACCESS))
            {
                LOGGER_THIS_LOG_INFO() << "Mutex NO duplicado: " << GetLastError();
                return false;
            }
        }
        if (!localPrintMutex)
        {
            LOGGER_THIS_LOG_INFO() << "Mutex NO valido";
            return false;
        }

        DWORD waitResult = WaitForSingleObject(localPrintMutex, MUX_TIMEOUT);
        if (waitResult != WAIT_OBJECT_0)
        {
            if (waitResult == WAIT_TIMEOUT) LOGGER_THIS_LOG_INFO() << "TIMEOUT esperando mutex";
            else                            LOGGER_THIS_LOG_INFO() << "ERROR esperando mutex: " << GetLastError();
            return false;
        }

        // Comprobamos el file stream
        bool printResult = false;
        if (validateFileStream())
        {
            // Limitamos el tamano del texto
            std::string shortenedText(message.text, 0, LOGGER_PRINT_LIMIT_SIZE);
            
            // Printamos el mensaje
            for (size_t idx = 0; idx < shortenedText.size(); idx += LOGGER_PRINT_BUFFER_SIZE)
            {
                *lastFileStream << std::string(shortenedText, idx, LOGGER_PRINT_BUFFER_SIZE);
            }
            *lastFileStream << std::endl;

            if (lastFileStream->good()) printResult = true;
            else                        LOGGER_THIS_LOG_INFO() << "ERROR escribiendo mensaje";
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
        if (loggerPath.empty())
        {
            LOGGER_THIS_LOG_INFO() << "Ruta de fichero NO valida: " << GetLastError();
            return false;
        }

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
