#include "utils/thread/ISafeThread.h"

#include "utils/exception/ExceptionManager.h"

namespace Utils
{
    namespace Thread
    {
        ////////////////////////////////////////
        // Hilo con proteccion de excepciones //
        ////////////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ISafeThread::ISafeThread(const std::string &threadName)
            : IThread(threadName)
        {
        }

        //----------------//
        // Final virtual  //
        //----------------//

        ExitCode ISafeThread::workerWrapper()
        {
            // Sobreescribimos la gestion de excepciones
            Utils::Exception::ExceptionManager exceptionManager;
            return this->intermidiateWorker(this->getName());
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        ExitCode ISafeThread::intermidiateWorker(const std::string &threadName)
        {
            ExitCode resultado = ExitCode::EXIT_CODE_OK;
            
            // Protegemos la ejecucion
            __try
            {
                resultado = this->worker();
            }
            __except (Utils::Exception::ExceptionManager::manageNamedUnhandledException(GetExceptionInformation(), threadName))
            {
                resultado = ExitCode::EXIT_CODE_EXCEPTION;
            }
            
            return resultado;
        }
    };
};
