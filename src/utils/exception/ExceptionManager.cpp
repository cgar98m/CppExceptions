#include "utils/exception/ExceptionManager.h"

#include <cstdlib>
#include "utils/exception/minidump/MiniDumpInfo.h"
#include "utils/exception/minidump/MiniDumpTools.h"
#include "utils/logging/LogEntry.h"
#include "utils/logging/LogTypes.h"

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////
        // Manejo de excepciones  //
        ////////////////////////////
    
        //------------//
        // Constantes //
        //------------//

        const char *ExceptionManager::PROGRAM_IDENTIFIER = "ExceptionManager";

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ExceptionManager::ExceptionManager(bool isGlobal, const Params& params)
            : topTerminateHandler(std::set_terminate(manageSafeTerminate))
            , topExceptionHandler(isGlobal ? SetUnhandledExceptionFilter(manageUnhandledException) : nullptr)
        {
            // Comprobamos si se quiere modificar el comportamiento global
            if (isGlobal)
            {
                // Cambiamos el comportamiento de exit
                std::atexit(manageSafeExit);
    
                // Actualizamos el logger
                std::lock_guard<std::mutex> lock(loggerMutex);
                logger = params.logger;
    
                // Verificamos si se quiere externalizar la gestion de la excepcion
                if (params.externalize)
                {
                    std::lock_guard<std::recursive_mutex> lockExternal(externalizeMutex);
                    if (!externalExceptionManager) externalExceptionManager.reset(new IpcExceptionManager(params.programIdentifier, true, logger));
                }
            }
        }
    
        ExceptionManager::~ExceptionManager()
        {
            if (this->topExceptionHandler) SetUnhandledExceptionFilter(this->topExceptionHandler);
            if (this->topTerminateHandler) std::set_terminate(this->topTerminateHandler);
        }
    
        //--------------------//
        // Funciones de clase //
        //--------------------//

        LONG WINAPI ExceptionManager::manageUnhandledException(PEXCEPTION_POINTERS exception)
        {
            return manageNamedUnhandledException(exception);
        }

        LONG WINAPI ExceptionManager::manageNamedUnhandledException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag)
        {
            // Protegemos la gestion de la excepcion
            __try
            {
                manageException(exception, exceptionTag);
            }
            __except (manageSafeCriticalMsvcException(GetExceptionInformation(), exceptionTag))
            {
            }
    
            return EXCEPTION_EXECUTE_HANDLER;
        }
    
        void ExceptionManager::manageSafeTerminate()
        {
            // Protegemos la gestion del terminate
            __try
            {
                manageTerminate();
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
            }

            // Si llegamos aqui, ha ocurrido algun error
            std::abort();
        }
    
        void ExceptionManager::manageSafeExit()
        {
            // Protegemos la gestion del exit
            __try
            {
                manageExit();
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
            }

            // Si llegamos aqui, ha ocurrido algun error
            std::abort();
        }
    
        LONG ExceptionManager::manageException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag)
        {
            // Obtenemos el logger
            SharedLogger tmpLogger;
            {
                std::lock_guard<std::mutex> lock(loggerMutex);
                tmpLogger = logger;
            }
            
            // Identificamos la excepcion
            if (exceptionTag.empty()) LOGGER_LOG_INFO(tmpLogger) << "Excepcion detectada";
            else                      LOGGER_LOG_INFO(tmpLogger) << "Excepcion detectada: " << exceptionTag;
         
            // Verificamos si es la primera excepcion (ignoramos el resto)
            {
                std::lock_guard<std::mutex> lock(firstExceptionMutex);
                if (!firstException)
                {
                    LOGGER_LOG_WARNING(tmpLogger) << "Excepcion descartada";
                    return EXCEPTION_EXECUTE_HANDLER;
                }
    
                firstException = false;
            }
    
            // Externalizamos el analisis (de ser requerido)
            bool manageLocally = true;
            {
                std::lock_guard<std::recursive_mutex> lock(externalizeMutex);
                if (externalExceptionManager && externalExceptionManager->isValid())
                    manageLocally = !externalExceptionManager->sendException(exception);
            }
    
            // Generamos dump si es necesario
            MiniDumpInfo miniDumpInfo;
            miniDumpInfo.exception = exception;
            if (manageLocally && !MiniDumpTools::createDumpFile(miniDumpInfo)) LOGGER_LOG_ERROR(tmpLogger) << "ERROR creando mini dump";
            
            std::terminate();
            return EXCEPTION_EXECUTE_HANDLER;
        }
    
        LONG ExceptionManager::manageSafeCriticalMsvcException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag)
        {
            LONG result = EXCEPTION_EXECUTE_HANDLER;

            __try
            {
                result = manageCriticalMsvcException(exception, exceptionTag);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                result = EXCEPTION_EXECUTE_HANDLER;
            }
    
            exceptionError = true;
            std::terminate();
            return result;
        }
    
        LONG ExceptionManager::manageCriticalMsvcException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag)
        {
            // Obtenemos el logger
            SharedLogger tmpLogger;
            {
                std::lock_guard<std::mutex> lock(loggerMutex);
                tmpLogger = logger;
            }

            // Identificamos la excepcion
            if (exceptionTag.empty()) LOGGER_LOG_INFO(tmpLogger) << "Excepcion CRITICA detectada";
            else                      LOGGER_LOG_INFO(tmpLogger) << "Excepcion CRITICA detectada: " << exceptionTag;
    
            return EXCEPTION_EXECUTE_HANDLER;
        }
    
        void ExceptionManager::manageTerminate()
        {
            // Veriricamos si venimos de un error critico
            if (exceptionError)
            {
                // Damos tiempo a que se escriban todos los logs antes de cerrar
                Sleep(Logging::LOGGER_FLUSH_TIMEOUT);
                std::abort();
            }
    
            // Obtenemos el logger
            SharedLogger tmpLogger;
            {
                std::lock_guard<std::mutex> lock(loggerMutex);
                tmpLogger = logger;
            }

            // Identificamos el terminate
            LOGGER_LOG_INFO(tmpLogger) << "Terminate ejecutado";
            std::exit(0);
        }
        
        void ExceptionManager::manageExit()
        {
            // Obtenemos el logger
            SharedLogger tmpLogger;
            {
                std::lock_guard<std::mutex> lock(loggerMutex);
                tmpLogger = logger;
            }

            // Identificamos el exit
            LOGGER_LOG_INFO(tmpLogger) << "Exit ejecutado";
    
            // Damos tiempo a que se escriban todos los logs
            Sleep(Logging::LOGGER_FLUSH_TIMEOUT);
        }

        //--------------------//
        // Variables de clase //
        //--------------------//

        std::unique_ptr<IpcExceptionManager> ExceptionManager::externalExceptionManager;
        std::recursive_mutex                 ExceptionManager::externalizeMutex;
    
        SharedLogger ExceptionManager::logger = BASIC_LOGGER();
        std::mutex   ExceptionManager::loggerMutex;
    
        bool       ExceptionManager::firstException = true;
        std::mutex ExceptionManager::firstExceptionMutex;
    
        bool ExceptionManager::exceptionError = false;
    };
};
