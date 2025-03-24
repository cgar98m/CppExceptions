#include "main/exception/CppExceptionThread.h"

#include <stdexcept>

namespace Main
{
    namespace Exception
    {
        ////////////////////////////////////////
        // Hilo que genera una excepcion C++  //
        ////////////////////////////////////////

        //------------//
        // Constantes //
        //------------//

        const char *CppExceptionThread::THREAD_NAME = "CppExceptionThread";

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        CppExceptionThread::CppExceptionThread(const std::string &threadName)
            : Utils::Thread::ISafeThread(threadName)
        {
        }

        //----------------//
        // Final virtual  //
        //----------------//

        Utils::ExitCode CppExceptionThread::worker()
        {
            throw std::runtime_error("Threaded C++ Exception");
            return Utils::ExitCode::EXIT_CODE_OK;
        }
    }
};
