#include "main/exception/SehExceptionThread.h"

namespace Main
{
    namespace Exception
    {
        ////////////////////////////////////////
        // Hilo que genera una excepcion SEH  //
        ////////////////////////////////////////

        //------------//
        // Constantes //
        //------------//

        const char *SehExceptionThread::THREAD_NAME = "SehExceptionThread";

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        SehExceptionThread::SehExceptionThread(const std::string &threadName)
            : Utils::Thread::ISafeThread(threadName)
        {
        }

        //----------------//
        // Final virtual  //
        //----------------//

        Utils::ExitCode SehExceptionThread::worker()
        {
            int *p = nullptr;
            *p = 20;
            return Utils::ExitCode::EXIT_CODE_OK;
        }
    }
};
