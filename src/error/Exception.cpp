#include "error/Exception.h"

#include <iostream>
#include "logger/ConsoleLogger.h"

namespace Error
{
    ////////////////////////////
    // Manejo de excepciones  //
    ////////////////////////////
    
    ExceptionManager::ExceptionManager(bool isGlobal)
        : topTerminateHandler(std::set_terminate(manageTerminate))
        , topExceptionHandler(isGlobal ? SetUnhandledExceptionFilter(manageUnhandledException) : nullptr)
    {
    }

    ExceptionManager::~ExceptionManager()
    {
        if (topExceptionHandler) SetUnhandledExceptionFilter(topExceptionHandler);
        if (topTerminateHandler) std::set_terminate(topTerminateHandler);
    }

    LONG WINAPI ExceptionManager::manageUnhandledException(PEXCEPTION_POINTERS exception)
    {
        __try
        {
            manageException(exception);
        }
        __except (manageCriticalMsvcException(GetExceptionInformation()))
        {
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void ExceptionManager::manageTerminate()
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Terminate ejecutado";

        // Si se quiere esperar a que se cierre el programa adecuadamente, este es el punto de no retorno
        std::abort();
    }

    LONG ExceptionManager::manageException(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Excepcion detectada";
        std::terminate();
        return EXCEPTION_EXECUTE_HANDLER;
    }

    LONG ExceptionManager::manageCriticalMsvcException(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Excepcion CRITICA detectada";
        std::terminate();
        return EXCEPTION_EXECUTE_HANDLER;
    }

    //////////////////////////////////////////
    // Thread con proteccion de excepciones //
    //////////////////////////////////////////

    SafeThread::SafeThread()
        : Thread()
    {
    }

    Error::ExitCode SafeThread::workerWrapper()
    {
        Error::ExitCode resultado = Error::ExitCode::EXIT_CODE_OK;

        // Protegemos la ejecucion
        __try
        {
            resultado = intermidiateWorker();
        }
        __except (Error::ExceptionManager::manageUnhandledException(GetExceptionInformation()))
        {
            resultado = Error::ExitCode::EXIT_CODE_EXCEPTION;
        }

        return resultado;
    }

    Error::ExitCode SafeThread::worker()
    {
        return Error::ExitCode::EXIT_CODE_NOT_IMPLEMENTED;
    }
    
    Error::ExitCode SafeThread::intermidiateWorker()
    {
        ExceptionManager exceptionManager(false);
        return this->worker();
    }

    //////////////////////////////////////////////////////
    // Clase para generar excepciones C++ en un thread  //
    //////////////////////////////////////////////////////

    Error::ExitCode CppExceptionThread::worker()
    {
        throw std::runtime_error("Threaded C++ Exception");
        return Error::ExitCode::EXIT_CODE_OK;
    }

    ////////////////////////////////////////
    // Thread que genera excepciones SEH  //
    ////////////////////////////////////////
    
    Error::ExitCode SehExceptionThread::worker()
    {
        int *p = nullptr;
        *p = 20;
        return Error::ExitCode::EXIT_CODE_OK;
    }
};
