#pragma once

#include <windows.h>
#include <string>
#include "error/Exception.h"
#include "utils/container/SemaphoredQueue.hpp"
#include "utils/Thread.h"

namespace Logger
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
            IThreadedLogger();
            IThreadedLogger(const IThreadedLogger&) = delete;
            IThreadedLogger& operator=(const IThreadedLogger&) = delete;
            virtual ~IThreadedLogger() = default;

            bool print(const LogMsg &message) final;

        private:
            virtual bool printEnqueued(const LogMsg &message);

            Error::ExitCode worker() final;
    };
};