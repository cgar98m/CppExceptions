#include "utils/Thread.h"

#include "error/Exception.h"

namespace Utils
{
    ////////////
    // Thread //
    ////////////

    HANDLE Thread::getHandle() const
    {
        return threadHandle;
    }

    void Thread::setHandle(HANDLE newThreadHandle)
    {
        threadHandle = newThreadHandle;
    }

    DWORD Thread::getId() const
    {
        return threadId;
    }

    void Thread::setId(DWORD newThreadId)
    {
        threadId = newThreadId;
    }

    bool Thread::isRunning() const
    {
        return running;
    }

    void Thread::setRunning(bool newRunning)
    {
        running = newRunning;
    }

    Error::ExitCode Thread::workerWrapper()
    {
        return this->worker();
    }

    Error::ExitCode Thread::worker()
    {
        return Error::ExitCode::EXIT_CODE_NOT_IMPLEMENTED;
    }

    //////////////////////////
    // Gestor de un thread  //
    //////////////////////////

    const DWORD ThreadHolder::TIMEOUT_MS_STOP_WAIT = 5000;
    const DWORD ThreadHolder::TIMEOUT_MS_LOOP_WAIT = 100;

    ThreadHolder::ThreadHolder(Thread &thread, const Params& params, const Utils::Logger& logger)
        : ILoggerHolder(logger)
        , thread(thread)
        , params(params)
    {
    }

    ThreadHolder::ThreadHolder::~ThreadHolder()
    {
        stop();
    }

    bool ThreadHolder::run()
    {
        std::lock_guard<std::recursive_mutex> lock(threadMux);
        if (thread.getHandle()) return true;

        // Creamos thread
        DWORD  threadId;
        HANDLE threadHandle = CreateThread(nullptr, 0, threadWrapper, this, CREATE_SUSPENDED, &threadId);
        if (!threadHandle)
        {
            LOGGER_THIS_LOG_INFO() << "ERROR creando hilo: " << GetLastError();
            return false;
        }

        // Actualizamos los datos del hilo
        thread.setHandle(threadHandle);
        thread.setId(threadId);

        // Asignamos prioridad
        if (!SetThreadPriority(threadHandle, params.threadPriority))
            LOGGER_THIS_LOG_INFO() << "ERROR especificando prioridad: " << GetLastError();
        
        // Reanudamos el thread
        thread.setRunning(true);
        if (ResumeThread(threadHandle) == static_cast<DWORD>(-1))
        {
            thread.setRunning(false);
            LOGGER_THIS_LOG_INFO() << "ERROR reanudando ejecucion: " << GetLastError() << ". Forzamos el cierre del hilo...";
            stop();
            return false;
        }
        
        return true;
    }

    bool ThreadHolder::requestStop()
    {
        // Verificamos si existe hilo
        std::lock_guard<std::recursive_mutex> l_lock(threadMux);
        if (!thread.getHandle()) return false;

        // Verificamos si el hilo esta ejecutandose
        if (!thread.isRunning()) return false;

        // Marcamos el cierre
        thread.setRunning(false);
        return true;
    }
    
    Error::ExitCode ThreadHolder::waitStop()
    {
        // Verificamos si existe hilo
        std::lock_guard<std::recursive_mutex> l_lock(threadMux);
        HANDLE threadHandle = thread.getHandle();
        DWORD  threadId     = thread.getId();
        if (!threadHandle) return Error::ExitCode::EXIT_CODE_OK;

        // Verificamos si se ha solicitado su detencion
        if (!thread.isRunning()) thread.setRunning(false);

        // Cerramos el hilo en ejecucion
        DWORD resultadoFinHilo = WaitForSingleObject(threadHandle, params.stopWait);
        switch (resultadoFinHilo)
        {
            case WAIT_OBJECT_0:
                break;
                
            case WAIT_TIMEOUT:
                LOGGER_THIS_LOG_INFO() << "Timeout esperando finalizacion del hilo [" << threadId << "]. Forzando cerrado...";
                break;
                
            case WAIT_FAILED:
            default:
                LOGGER_THIS_LOG_INFO() << "ERROR finalizando el hilo [" << threadId << "]: " << GetLastError() << ". Forzando cerrado...";
                break;
        }

        // Forzamos el cierre del hilo por error o suspension
        bool resultadoTerminate = true;
        if (resultadoFinHilo != WAIT_OBJECT_0)
        {
            if (!TerminateThread(threadHandle, static_cast<DWORD>(Error::ExitCode::EXIT_CODE_TERMINATE)))
            {
                LOGGER_THIS_LOG_INFO() << "ERROR forzando cierre de hilo [" << threadId << "]";
                resultadoTerminate = false;
            }
        }

        CloseHandle(threadHandle);
        thread.setHandle();
        thread.setId();

        if (!resultadoTerminate) return Error::ExitCode::EXIT_CODE_OK;
        return Error::ExitCode::EXIT_CODE_KO;
    }

    Error::ExitCode ThreadHolder::stop()
    {
        return waitStop();
    }

    Error::ExitCode ThreadHolder::threadLoop()
    {
        Error::ExitCode resultado = Error::ExitCode::EXIT_CODE_OK;

        do
        {
            Sleep(params.loopWait);
            resultado = thread.workerWrapper();
            if (resultado != Error::ExitCode::EXIT_CODE_OK) thread.setRunning(false);
        }
        while (thread.isRunning());

        return resultado;
    }

    DWORD WINAPI ThreadHolder::threadWrapper(LPVOID param)
    {
        Error::ExitCode resultado = Error::ExitCode::EXIT_CODE_KO;

        ThreadHolder *threadHolder = static_cast<ThreadHolder*>(param);
        if (threadHolder) resultado = threadHolder->threadLoop();

        return static_cast<DWORD>(resultado);
    }
};
