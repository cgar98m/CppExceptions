#pragma once

#include <windows.h>
#include "error/Exception.h"
#include "error/Types.h"
#include "utils/Thread.h"
#include "utils/container/SemaphoredQueue.hpp"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    // Interfaz de un logger que gestiona los mensajes en un hilo
    class IThreadedLogger: public ILogger, public Error::SafeThread
    {
        private:
            static const DWORD TIMEOUT_MS_STOP_WAIT;
            static const DWORD TIMEOUT_MS_LOOP_WAIT;
            static const DWORD TIMEOUT_MS_PRINT_WAIT;

            Utils::ThreadHolder            printThread;
            Utils::SemaphoredQueue<LogMsg> printQueue;

        public:
            IThreadedLogger(const Logger& errorLogger);
            IThreadedLogger(const IThreadedLogger&) = delete;
            IThreadedLogger& operator=(const IThreadedLogger&) = delete;
            virtual ~IThreadedLogger() = default;

            bool print(const LogMsg &message) final;

        private:
            virtual bool printEnqueued(const LogMsg &message) = 0;

            Error::ExitCode worker() final;
    };
};
