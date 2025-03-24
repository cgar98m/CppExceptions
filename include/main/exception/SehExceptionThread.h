#pragma once

#include <string>
#include "utils/ExitCode.h"
#include "utils/thread/ISafeThread.h"

namespace Main
{
    namespace Exception
    {
        ////////////////////////////////////////
        // Hilo que genera una excepcion SEH  //
        ////////////////////////////////////////
        
        class SehExceptionThread: public Utils::Thread::ISafeThread
        {
            // Constantes
            private:
                static const char *THREAD_NAME;

            // Constructor/Destructor
            public:
                SehExceptionThread(const std::string &threadName = THREAD_NAME);
                virtual ~SehExceptionThread() = default;
            
            // Deleted
            public:
                SehExceptionThread(const SehExceptionThread&)            = delete;
                SehExceptionThread& operator=(const SehExceptionThread&) = delete;
    
            // Final virtual
            private:
                virtual Utils::ExitCode worker() final;
        };
    };
};
