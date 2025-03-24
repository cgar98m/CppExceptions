#pragma once

#include <string>
#include "utils/ExitCode.h"
#include "utils/thread/ISafeThread.h"

namespace Main
{
    namespace Exception
    {
        ////////////////////////////////////////
        // Hilo que genera una excepcion C++  //
        ////////////////////////////////////////
        
        class CppExceptionThread: public Utils::Thread::ISafeThread
        {
            // Constantes
            private:
                static const char *THREAD_NAME;

            // Constructor/Destructor
            public:
                CppExceptionThread(const std::string &threadName = THREAD_NAME);
                virtual ~CppExceptionThread() = default;
            
            // Deleted
            public:
                CppExceptionThread(const CppExceptionThread&)            = delete;
                CppExceptionThread& operator=(const CppExceptionThread&) = delete;
    
            // Final virtual
            private:
                virtual Utils::ExitCode worker() final;
        };
    };
};
