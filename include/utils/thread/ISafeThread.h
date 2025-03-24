#pragma once

#include <string>
#include "utils/ExitCode.h"
#include "utils/thread/IThread.h"

namespace Utils
{
    namespace Thread
    {
        ////////////////////////////////////////
        // Hilo con proteccion de excepciones //
        ////////////////////////////////////////
        
        class ISafeThread: public IThread
        {
            // Constructor/Destructor
            public:
                ISafeThread(const std::string &threadName = std::string());
                virtual ~ISafeThread() = default;
            
            // Deleted
            public:
                ISafeThread(const ISafeThread&)            = delete;
                ISafeThread &operator=(const ISafeThread&) = delete;
                
            // Pure virtual
            protected:
                virtual ExitCode worker() = 0;
    
            // Final virtual
            public:
                virtual ExitCode workerWrapper() final;
                
            // Funciones miembro
            private:
                ExitCode intermidiateWorker(const std::string &threadName);
        };
    };
};
