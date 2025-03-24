#pragma once

#include <windows.h>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "utils/filesystem/FileTools.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/IThreadedLogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/logging/LogMsg.h"

//////////////
// Defines  //
//////////////

#define FILE_LOGGER(file) Utils::Logging::FileLogger::getInstance((file))

namespace Utils
{
    namespace Logging
    {
        //////////////////////////////////////////////////////
        // Logger para la impresion en un fichero por hilo  //
        //////////////////////////////////////////////////////
        
        class FileLogger: public IThreadedLogger
        {
            // Constantes
            public:
                static const char  *MUX_NAME;
                static const DWORD TIMEOUT_MS_MUX_WAIT;
            
            private:
                static const char *LOGGER_NAME;
    
            // Tipos, estructuras y enums
            private:
                using LoggerMap = std::map<std::string, SharedLogger>;

            // Constructor/Destructor
            public:
                virtual ~FileLogger() = default;
                
            private:
                explicit FileLogger(const std::string &fileBaseName, const std::string &fileDir = FileSystem::FileTools::OUTPUT_PATH, const SharedLogger &logger = BASIC_LOGGER());

            // Deleted
            public:
                FileLogger()                             = delete;
                explicit FileLogger(const FileLogger&)   = delete;
                FileLogger &operator=(const FileLogger&) = delete;

            // Final virtual
            private:
                virtual bool validateStream(const LogMsg &message)            final;
                virtual bool processPrintToStream(const std::string &message) final;

            // Funciones de clase
            public:
                static SharedLogger getInstance(const std::string &fileBaseName, const std::string &fileDir = FileSystem::FileTools::OUTPUT_PATH, const SharedLogger &logger = BASIC_LOGGER());

            // Funciones miembro
            private:
                void clearFileStream();

            // Variables de clase
            private:
                static LoggerMap  fileLoggers;
                static std::mutex instanceMux;

            // Variables miembro
            private:    
                std::string fileBaseName;
                std::string fileDir;
    
                SYSTEMTIME                     lastDate = {};
                std::string                    lastFilePath;
                std::unique_ptr<std::ofstream> lastFileStream;
        };
    };
};
