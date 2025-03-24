#include "utils/thread/ThreadHolder.h"

#include "utils/logging/LogEntry.h"

namespace Utils
{
    namespace Thread
    {
        ////////////////////////
        // Gestor de un hilo  //
        ////////////////////////

        //------------//
        // Constantes //
        //------------//

        const DWORD ThreadHolder::TIMEOUT_MS_STOP_WAIT = 5000;
        const DWORD ThreadHolder::TIMEOUT_MS_LOOP_WAIT = 100;

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ThreadHolder::ThreadHolder(IThread &thread, const Params &params, const SharedLogger &logger)
            : Logging::LoggerHolder(logger)
            , thread(thread)
            , params(params)
        {
        }

        ThreadHolder::~ThreadHolder()
        {
            std::lock_guard<std::recursive_mutex> lock(this->threadMux);
            this->stop();
        }

        //--------------------//
        // Funciones de clase //
        //--------------------//

        DWORD WINAPI ThreadHolder::threadWrapper(LPVOID param)
        {
            ExitCode resultado = ExitCode::EXIT_CODE_KO;

            // Ejecutamos el bucle del hilo (si tenemos un valor valido)
            ThreadHolder *threadHolder = static_cast<ThreadHolder*>(param);
            if (threadHolder) resultado = threadHolder->threadLoop();

            return static_cast<DWORD>(resultado);
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        bool ThreadHolder::run()
        {
            std::lock_guard<std::recursive_mutex> lock(this->threadMux);

            // Verificamos si esta ejecutandose
            HANDLE threadHandle = this->thread.getHandle();
            if (threadHandle)
            {
                // Verificamos si el hilo sigue vivo
                ExitCode waitResult = this->waitStop(0);
                if (waitResult == ExitCode::EXIT_CODE_KO)      return false;
                if (waitResult == ExitCode::EXIT_CODE_TIMEOUT) return true;
            }

            // Creamos hilo
            DWORD threadId = 0;
            threadHandle   = CreateThread(nullptr, 0, threadWrapper, this, CREATE_SUSPENDED, &threadId);
            if (!threadHandle)
            {
                LOGGER_THIS_LOG_ERROR() << "Error creando hilo: " << GetLastError();
                return false;
            }

            // Actualizamos los datos del hilo
            this->thread.setHandle(threadHandle);
            this->thread.setId(threadId);

            // Asignamos prioridad
            if (!SetThreadPriority(threadHandle, this->params.threadPriority))
                LOGGER_THIS_LOG_WARNING() << "ERROR especificando prioridad: " << GetLastError();
            
            // Iniciamos el hilo
            this->thread.setRunning(true);
            if (ResumeThread(threadHandle) == static_cast<DWORD>(-1L))
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR iniciando hilo: " << GetLastError();
                this->stop();
                return false;
            }
            
            return true;
        }

        void ThreadHolder::requestStop()
        {
            std::lock_guard<std::recursive_mutex> lock(this->threadMux);

            // Verificamos si existe hilo
            if (!this->thread.getHandle()) return;

            // Marcamos el cierre
            this->thread.setRunning(false);
        }
        
        ExitCode ThreadHolder::waitStop(DWORD timeout, bool closeThread)
        {
            std::lock_guard<std::recursive_mutex> lock(this->threadMux);

            // Verificamos si existe hilo
            HANDLE threadHandle = this->thread.getHandle();
            if (!threadHandle) return ExitCode::EXIT_CODE_OK;
            
            // Esperamos al cierre del hilo en ejecucion
            DWORD    threadId        = this->thread.getId();
            bool     terminateThread = false;
            ExitCode waitResult      = ExitCode::EXIT_CODE_OK;
            switch (WaitForSingleObject(threadHandle, timeout))
            {
                case WAIT_OBJECT_0:
                    break;
                    
                case WAIT_TIMEOUT:
                    if (closeThread)
                    {
                        LOGGER_THIS_LOG_WARNING() << "TIMEOUT esperando finalizacion del hilo " << threadId;
                        terminateThread = true;
                    }
                    else
                    {
                        waitResult = ExitCode::EXIT_CODE_TIMEOUT;
                    }
                    break;
                    
                case WAIT_FAILED:
                default:
                    LOGGER_THIS_LOG_ERROR() << "ERROR esperando finalizacion del hilo " << threadId << ": " << GetLastError();
                    terminateThread = true;
                    break;
            }

            // Forzamos el cierre del hilo (si fuera necesario)
            if (terminateThread && !TerminateThread(threadHandle, static_cast<DWORD>(ExitCode::EXIT_CODE_TERMINATE)))
                LOGGER_THIS_LOG_WARNING() << "ERROR forzando cierre de hilo " << threadId;
            
            // Limpiamos los datos del hilo (si fuera necesario)
            if (closeThread || terminateThread) this->cleanThreadData();

            return waitResult;
        }

        ExitCode ThreadHolder::stop()
        {
            this->requestStop();
            return this->waitStop(this->params.stopWait, true);
        }

        DWORD ThreadHolder::getStopTimeout()
        {
            return this->params.stopWait;
        }

        ExitCode ThreadHolder::threadLoop()
        {
            ExitCode resultado = ExitCode::EXIT_CODE_OK;

            do
            {
                Sleep(this->params.loopWait);
                resultado = this->thread.workerWrapper();
                if (resultado != ExitCode::EXIT_CODE_OK) this->thread.setRunning(false);
            }
            while (this->thread.isRunning());

            return resultado;
        }

        void ThreadHolder::cleanThreadData()
        {
            this->thread.setRunning();
            this->thread.setId();

            HANDLE threadHandle = this->thread.getHandle();
            if (threadHandle) CloseHandle(threadHandle);
            this->thread.setHandle();
        }
    };
};
