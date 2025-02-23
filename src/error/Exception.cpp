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
            manageMsvcException();
            // throw std::runtime_error("C++ Exception in ExceptionManager");
            int *p = nullptr;
            *p = 20;
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
    }

    void ExceptionManager::manageMsvcException()
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Excepcion detectada";
    }

    LONG ExceptionManager::manageCriticalMsvcException(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ConsoleLogger> l_logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(l_logger) << "Excepcion CRITICA detectada";

        return EXCEPTION_EXECUTE_HANDLER;
    }
};
