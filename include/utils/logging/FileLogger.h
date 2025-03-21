#pragma once

#include <windows.h>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/IThreadedLogger.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    // Logger para un fichero
    class FileLogger: public IThreadedLogger, public ILoggerHolder
    {
        public:
            static const char  *MUX_NAME;
            static const DWORD MUX_TIMEOUT;

        private:
            using LoggerMap = std::map<std::string, Logger>;

            static LoggerMap  fileLoggers;
            static std::mutex instanceMux;

            HANDLE     ostreamMux = nullptr;
            std::mutex internalMux;

            std::string fileBaseName;
            std::string fileDir;

            SYSTEMTIME                     lastDate = {};
            std::string                    lastFilePath;
            std::unique_ptr<std::ofstream> lastFileStream;

        public:
            static Logger getInstance(const std::string &fileBaseName, const std::string &fileDir = ".", const Logger& errorLogger = BasicLogger::getInstance());
            
            FileLogger(const FileLogger&) = delete;
            FileLogger& operator=(const FileLogger&) = delete;
            virtual ~FileLogger();
            
        private:
            FileLogger(const std::string &fileBaseName, const std::string &fileDir, const Logger& errorLogger = BasicLogger::getInstance());
            
            bool printEnqueued(const LogMsg &message) final;

            bool validateFileStream();

    };
};
