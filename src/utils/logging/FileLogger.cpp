#include "utils/logging/FileLogger.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include "utils/logging/LogEntry.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    namespace Logging
    {
        //////////////////////////////////////////////////////
        // Logger para la impresion en un fichero por hilo  //
        //////////////////////////////////////////////////////
    
        //------------//
        // Constantes //
        //------------//

        const char  *FileLogger::MUX_NAME           = "Utils/Logging/FileMutex";
        const DWORD FileLogger::TIMEOUT_MS_MUX_WAIT = 1000;
        const char  *FileLogger::LOGGER_NAME        = "FileLogger";
        
        //------------------------//
        // Constructor/Destructor //
        //------------------------//
    
        FileLogger::FileLogger(const std::string &fileBaseName, const std::string &fileDir, const SharedLogger &logger)
            : IThreadedLogger(Params{THREAD_PRIORITY_NORMAL, TIMEOUT_MS_LOOP_WAIT, TIMEOUT_MS_STOP_WAIT, TIMEOUT_MS_MUX_WAIT, std::string(LOGGER_NAME) + std::string("/") + fileBaseName}
                            , logger)
            , fileBaseName(fileBaseName)
            , fileDir(fileDir)
        {
            // Formamos un nombre de mutex unico en base a la ruta del fichero
            std::string muxName = std::string(MUX_NAME) + std::string("/") + this->fileDir + this->fileBaseName;
            std::replace(muxName.begin(), muxName.end(), '\\', '/');
            this->ostreamMux = CreateMutex(nullptr, FALSE, muxName.c_str());
        }

        //----------------//
        // Final virtual  //
        //----------------//

        bool FileLogger::validateStream(const LogMsg &message)
        {
            if (this->fileDir.empty() || this->fileBaseName.empty()) return false;
    
            // Verificamos si tenemos nombre de fichero previo valido
            if ((!this->lastFilePath.empty()) &&
                (this->lastDate.wYear == message.date.wYear && this->lastDate.wMonth == message.date.wMonth && message.date.wDay == this->lastDate.wDay) &&
                (this->lastFileStream && this->lastFileStream->good()))
            {
                return true;
            }
    
            // Limpiamos
            this->clearFileStream();
            this->lastDate = message.date;
    
            // Obtenemos ruta absoluta
            std::string loggerPath = FileSystem::FileTools::getDirAbsolutePath(fileDir, true);
            if (loggerPath.empty())
            {
                LOGGER_THIS_LOG_ERROR() << "Ruta de fichero NO valida: " << GetLastError();
                return false;
            }
    
            // Montamos la ruta completa
            std::stringstream ssFilePath;
            ssFilePath << loggerPath;
            ssFilePath << std::setfill('0')                 << this->lastDate.wYear;
            ssFilePath << std::setfill('0') << std::setw(2) << this->lastDate.wMonth;
            ssFilePath << std::setfill('0') << std::setw(2) << this->lastDate.wDay;
            ssFilePath << "_" << fileBaseName << ".log";
            this->lastFilePath = ssFilePath.str();
    
            // Creamos el file stream
            this->lastFileStream.reset(new std::ofstream(this->lastFilePath, std::ios_base::app));
            if (!this->lastFileStream || !this->lastFileStream->good())
            {
                this->clearFileStream();
                return false;
            }
    
            return true;
        }

        bool FileLogger::processPrintToStream(const std::string &message)
        {
            // Printamos el mensaje
            bool printResult = false;
            for (size_t idx = 0; idx < message.size(); idx += LOGGER_PRINT_BUFFER_SIZE)
            {
                *this->lastFileStream << std::string(message, idx, LOGGER_PRINT_BUFFER_SIZE);
            }
            *this->lastFileStream << std::endl;

            // Verificamos si el stream de salida esta vivo
            if (this->lastFileStream->good()) printResult = true;
            else                              LOGGER_THIS_LOG_ERROR() << "ERROR escribiendo mensaje";
    
            // Liberamos el stream de salida si ha habido fallo
            if (!printResult) this->lastFileStream.reset();
            return printResult;
        }
    
        //--------------------//
        // Funciones de clase //
        //--------------------//

        SharedLogger FileLogger::getInstance(const std::string &fileBaseName, const std::string &fileDir, const SharedLogger &logger)
        {
            // Obtenemos la ruta del logger
            SharedLogger fileLogger;
            if (fileBaseName.empty())
            {
                LOGGER_LOG_WARNING(logger) << "Nombre de fichero NO valido";
                return fileLogger;
            }
            
            std::string loggerPath = FileSystem::FileTools::getDirAbsolutePath(fileDir, true);
            if (loggerPath.empty())
            {
                LOGGER_LOG_WARNING(logger) << "Ruta de fichero NO valida: " << GetLastError();
                return fileLogger;
            }
    
            std::string filePath = loggerPath + fileBaseName;
    
            // Obtenemos la instancia asociada al fichero
            std::lock_guard<std::mutex> lock(instanceMux);
            if (fileLoggers.find(filePath) == fileLoggers.end())
            {
                fileLogger.reset(new FileLogger(fileBaseName, loggerPath, logger));
                if (fileLogger) fileLoggers[filePath] = fileLogger;
            }
            else
            {
                fileLogger = fileLoggers[filePath];   
            }
            return fileLogger;
        }
        
        //--------------------//
        // Funciones miembro  //
        //--------------------//

        void FileLogger::clearFileStream()
        {
            this->lastFilePath.clear();
            if (this->lastFileStream) this->lastFileStream.reset();
            this->lastDate = {};
        }

        //--------------------//
        // Variables de clase //
        //--------------------//

        FileLogger::LoggerMap FileLogger::fileLoggers;
        std::mutex            FileLogger::instanceMux;
    };
};
