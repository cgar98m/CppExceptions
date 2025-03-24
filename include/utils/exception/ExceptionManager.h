#pragma once

#include <windows.h>
#include <exception>
#include <mutex>
#include <string>
#include "utils/exception/ipc/IpcExceptionManager.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////
        // Manejo de excepciones  //
        ////////////////////////////
    
        class ExceptionManager
        {
            // Constantes
            private:
                static const char *PROGRAM_IDENTIFIER;

            // Tipos, estructuras y enums
            public:
                struct Params
                {
                    bool         externalize       = false;
                    SharedLogger logger            = BASIC_LOGGER();
                    std::string  programIdentifier = PROGRAM_IDENTIFIER;
                };

            // Constructor/Destructor
            public:
                ExceptionManager(bool isGlobal = false, const Params& params = Params());
                virtual ~ExceptionManager();
            
            // Deleted
            public:
                ExceptionManager &operator=(const ExceptionManager&) = delete;

            // Funciones de clase
            public:    
                static LONG WINAPI manageUnhandledException(PEXCEPTION_POINTERS exception);
                static LONG WINAPI manageNamedUnhandledException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag = std::string());
                static void manageSafeTerminate();
                static void manageSafeExit();
                
            private:
                static LONG manageException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag);
                static LONG manageSafeCriticalMsvcException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag = std::string());
                static LONG manageCriticalMsvcException(PEXCEPTION_POINTERS exception, const std::string &exceptionTag = std::string());
                static void manageTerminate();
                static void manageExit();

            // Variables de clase
            private:
                static UniqueIpcExceptionManager externalExceptionManager;
                static std::recursive_mutex      externalizeMutex;

                static SharedLogger logger;
                static std::mutex   loggerMutex;

                static bool       firstException;
                static std::mutex firstExceptionMutex;

                static bool exceptionError;

            // Variables miembro
            private:    
                LPTOP_LEVEL_EXCEPTION_FILTER topExceptionHandler = nullptr;
                std::terminate_handler       topTerminateHandler = nullptr;
        };
    };
};
