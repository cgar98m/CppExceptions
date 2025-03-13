#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "logger/IThreadedLogger.h"

namespace Logger
{
    // Logger para un fichero
    class FileLogger: public IThreadedLogger
    {
        public:
            static const std::string MUX_PREFIX;
            static const DWORD       MUX_TIMEOUT;

        private:
            using LoggerMap = std::map<std::string, Logger>;

            static LoggerMap  fileLoggers;
            static std::mutex muxInstance;

            HANDLE     printMutex = nullptr;
            std::mutex printMutexMux;

            std::string fileBaseName;
            std::string fileDir;

            SYSTEMTIME                     lastDate = {};
            std::string                    lastFilePath;
            std::unique_ptr<std::ofstream> lastFileStream;

        public:
            static Logger getInstance(const std::string& fileBaseName, const std::string& fileDir = ".");
            
            FileLogger(const FileLogger&) = delete;
            FileLogger& operator=(const FileLogger&) = delete;
            virtual ~FileLogger();
            
        private:
            FileLogger(const std::string& fileBaseName, const std::string& fileDir);
            
            bool printEnqueued(const LogMsg &message) final;

            bool validateFileStream();

    };
};
