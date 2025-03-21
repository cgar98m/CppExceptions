#pragma once

#include <windows.h>
#include <mutex>
#include <string>
#include "utils/logging/ILogger.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    // Interfaz de un logger por salida estandar
    class BasicLogger: public ILogger
    {
        public:
            static const char  *MUX_NAME;
            static const DWORD MUX_TIMEOUT;

        private:
            static Logger     basicLogger;
            static std::mutex instanceMux;
            
            HANDLE     ostreamMux = nullptr;
            std::mutex internalMux;

        public:
            static Logger getInstance();

            BasicLogger(const BasicLogger&) = delete;
            BasicLogger& operator=(const BasicLogger&) = delete;
            virtual ~BasicLogger();
            
            bool print(const LogMsg &message) final;
            
        private:
            BasicLogger();

            bool printEnqueued(const LogMsg &message) final;
    };
};
