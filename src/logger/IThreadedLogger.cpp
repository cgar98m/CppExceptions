#include "logger/IThreadedLogger.h"

namespace Logger
{
    ////////////////////////////////////////////////////////////////
    // Interfaz de un logger que gestiona los mensajes en un hilo //
    ////////////////////////////////////////////////////////////////
    
    const DWORD IThreadedLogger::TIMEOUT_MS_STOP_WAIT  = 10000;
    const DWORD IThreadedLogger::TIMEOUT_MS_LOOP_WAIT  = 0;
    const DWORD IThreadedLogger::TIMEOUT_MS_PRINT_WAIT = 1000;
    
    IThreadedLogger::IThreadedLogger()
        : ILogger()
        , SafeThread()
        , printThread(*this
                    , { THREAD_PRIORITY_NORMAL, TIMEOUT_MS_LOOP_WAIT, TIMEOUT_MS_STOP_WAIT }
                    , BasicLogger::getInstance())
    {
    }

    bool IThreadedLogger::print(const std::string& message)
    {
        if (!printThread.run()) return false;
        return printQueue.push(message);
    }

    bool IThreadedLogger::printEnqueued(const std::string& message)
    {
        return false;
    }

    Error::ExitCode IThreadedLogger::worker()
    {
        std::string message;
        switch (printQueue.top(message, TIMEOUT_MS_PRINT_WAIT))
        {
            case WAIT_OBJECT_0:
                break;

            case WAIT_TIMEOUT:
                return Error::ExitCode::EXIT_CODE_OK;
            
            case WAIT_ABANDONED:
            case WAIT_FAILED:
            default:
                return Error::ExitCode::EXIT_CODE_KO;
        }

        if (!printEnqueued(message)) return Error::ExitCode::EXIT_CODE_KO;
        if (printQueue.pop(message, 0) != WAIT_OBJECT_0) return Error::ExitCode::EXIT_CODE_KO;
        return Error::ExitCode::EXIT_CODE_OK;
    }
};
