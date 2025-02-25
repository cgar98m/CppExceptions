#include "error/Exception.h"

#include <iostream>

#include "logger/ConsoleLogger.h"

namespace Error
{
    ////////////////////////////
    // Manejo de excepciones  //
    ////////////////////////////

    LONG WINAPI ExceptionManager::manageMsvcException(PEXCEPTION_POINTERS exception)
    {
        __try
        {
            manageException(exception);
        }
        __except (manageCriticalMsvcException(GetExceptionInformation()))
        {
        }

        // Terminamos la ejecución a la fuerza
        std::terminate();
        return EXCEPTION_EXECUTE_HANDLER;
    }

    void ExceptionManager::manageTerminate()
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Terminate ejecutado";
        Sleep(5000);
    }

    LONG ExceptionManager::manageException(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Excepcion detectada";

        return EXCEPTION_EXECUTE_HANDLER;
    }

    LONG ExceptionManager::manageCriticalMsvcException(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Excepcion CRITICA detectada";

        return EXCEPTION_EXECUTE_HANDLER;
    }

    //////////////////////////////////////////////////
    // Wrapper de un thread que captura excepciones //
    //////////////////////////////////////////////////

    const DWORD SafeThread::TIMEOUT_MS_STOP_WAIT = 5000;
    const DWORD SafeThread::TIMEOUT_MS_LOOP_WAIT = 100;
    
    const DWORD SafeThread::THREAD_EXIT_CODE_EXCEPTION = 2;
    const DWORD SafeThread::THREAD_EXIT_CODE_TERMINATE = 3;

    const DWORD SafeThread::THREAD_EXIT_CODE_OK = 0;
    const DWORD SafeThread::THREAD_EXIT_CODE_KO = 1;

    SafeThread::SafeThread(const Params& params)
        : m_params(params)
    {
        if (m_params.dwMaxStopMsTimeout == 0) m_params.dwMaxStopMsTimeout = TIMEOUT_MS_STOP_WAIT;
        if (!m_params.exceptionHandler)       m_params.exceptionHandler   = ExceptionManager::manageMsvcException;
        if (!m_params.terminateHandler)       m_params.terminateHandler   = ExceptionManager::manageTerminate;
    }

    SafeThread::~SafeThread()
    {
        std::lock_guard<std::recursive_mutex> l_lock(m_threadMux);
        stop();
    }

    bool SafeThread::run()
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        bool l_bResultado = false;

        std::lock_guard<std::recursive_mutex> l_lock(m_threadMux);
        if (!m_thread)
        {
            // Creamos thread
            m_thread = CreateThread(nullptr, 0, wrapperThread, this, CREATE_SUSPENDED, &m_dwThreadId);
            if (!m_thread)
            {
                LOGGER_LOG(l_logger) << "Error creando hilo: GetLastError: " << GetLastError();
                m_dwThreadId = 0;
            }
            else
            {
                // Asignamos prioridad
                if (!SetThreadPriority(m_thread, m_params.iThreadPriority))
                    LOGGER_LOG(l_logger) << "Error especificando prioridad: GetLastError: " << GetLastError();
                
                // Reanudamos el thread
                if (ResumeThread(m_thread) == -1)
                {
                    LOGGER_LOG(l_logger) << "Error reanudando ejecucion: GetLastError: " << GetLastError() << ". Forzamos el cierre del hilo...";
                    stop();
                }
            }
        }

        return l_bResultado;
    }

    bool SafeThread::stop()
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        bool l_bResultado = true;

        std::lock_guard<std::recursive_mutex> l_lock(m_threadMux);
        if (m_thread)
        {
            DWORD l_dwResultado = WAIT_FAILED;

            // Cerramos el hilo en ejecucion
            if (m_bRunning)
            {
                m_bRunning = false;
                
                l_dwResultado = WaitForSingleObject(m_thread, m_params.dwMaxStopMsTimeout);
                switch (l_dwResultado)
                {
                    case WAIT_OBJECT_0:
                        break;
                        
                    case WAIT_TIMEOUT:
                        LOGGER_LOG(l_logger) << "Timeout esperando finalizacion del hilo [" << m_dwThreadId << "]. Forzando cerrado...";
                        break;
                        
                    case WAIT_FAILED:
                        LOGGER_LOG(l_logger) << "Error finalizando el hilo [" << m_dwThreadId << "]: " << l_dwResultado << ", GetLastError: " << GetLastError() << ". Forzando cerrado...";
                        break;
                        
                    default:
                        LOGGER_LOG(l_logger) << "Error finalizando el hilo [" << m_dwThreadId << "]: N/A, GetLastError: " << GetLastError() << ". Forzando cerrado...";
                        break;
                }
            }

            // Forzamos el cierre del hilo por error o suspension
            if (l_dwResultado != WAIT_OBJECT_0)
            {
                if (!TerminateThread(m_thread, THREAD_EXIT_CODE_TERMINATE))
                {
                    LOGGER_LOG(l_logger) << "Error forzando cierre de hilo [" << m_dwThreadId << "]";
                    l_bResultado = false;
                }
            }

            CloseHandle(m_thread);
            m_thread     = nullptr;
            m_dwThreadId = 0;
        }

        return l_bResultado;
    }
    
    DWORD __stdcall SafeThread::wrapperThread(LPVOID param)
    {
        DWORD l_dwResultado = THREAD_EXIT_CODE_KO;

        SafeThread *l_objectPtr = static_cast<SafeThread*>(param);
        if (l_objectPtr) l_dwResultado = l_objectPtr->configureThread();

        return l_dwResultado;
    }

    DWORD SafeThread::configureThread()
    {
        DWORD l_dwResultado = THREAD_EXIT_CODE_OK;

        // Actualizamos la funcion terminate
        if (!std::set_terminate(m_params.terminateHandler))
            LOGGER_LOG(Logger::ConsoleLogger::getInstance()) << "Error configurando terminate en hilo [" << m_dwThreadId << "]";

        // Ejecutamos el thread protegido
        return protectedThread();
    }

    DWORD SafeThread::protectedThread()
    {
        DWORD l_dwResultado = THREAD_EXIT_CODE_OK;
        
        // Protegemos la ejecucion
        __try
        {
            l_dwResultado = loopThread();
        }
        __except (m_params.exceptionHandler(GetExceptionInformation()))
        {
            l_dwResultado = THREAD_EXIT_CODE_EXCEPTION;
        }

        return l_dwResultado;
    }

    DWORD SafeThread::loopThread()
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        DWORD l_dwResultado = THREAD_EXIT_CODE_OK;
        DWORD l_dwThreadId  = 0;
        bool  l_bRunning    = true;

        // Obtenemos el id del hilo
        {
            std::lock_guard<std::recursive_mutex> l_lock(m_threadMux);
            l_dwThreadId = m_dwThreadId;
        }
        if (!l_dwThreadId) LOGGER_LOG(l_logger) << "Error identificando hilo";

        // Ejecutamos el hilo
        do
        {
            l_dwResultado = workingThread();
            if (l_dwResultado != THREAD_EXIT_CODE_OK)
            {
                LOGGER_LOG(l_logger) << "Error durante ejecucion de hilo [" << l_dwThreadId << "]";
                l_dwResultado = THREAD_EXIT_CODE_KO;
                break;
            }

            // Realizamos la espera
            Sleep(m_params.dwLoopWaitTimeout);
            std::lock_guard<std::recursive_mutex> l_lock(m_threadMux);
            l_bRunning = m_bRunning;
        }
        while (l_bRunning);

        return l_dwResultado;
    }

    DWORD SafeThread::workingThread()
    {
        return THREAD_EXIT_CODE_KO;
    }

    //////////////////////////////////////////////////////
    // Clase para generar excepciones C++ en un thread  //
    //////////////////////////////////////////////////////

    const std::string CppExceptionThread::EXCEPTION_MSG = "Threaded C++ Exception";

    CppExceptionThread::CppExceptionThread()
        : SafeThread(Params())
    {
    }

    CppExceptionThread::~CppExceptionThread()
    {
    }

    DWORD CppExceptionThread::workingThread()
    {
        throw std::runtime_error(EXCEPTION_MSG);
        return SafeThread::THREAD_EXIT_CODE_OK;
    }

    //////////////////////////////////////////////////////
    // Clase para generar excepciones SEH en un thread  //
    //////////////////////////////////////////////////////

    SehExceptionThread::SehExceptionThread()
        : SafeThread(Params())
    {
    }

    SehExceptionThread::~SehExceptionThread()
    {
    }
    
    DWORD SehExceptionThread::workingThread()
    {
        int *p = nullptr;
        *p = 20;
        return SafeThread::THREAD_EXIT_CODE_OK;
    }
};
