#pragma once

#include <windows.h>
#include <atomic>
#include <string>
#include "utils/ExitCode.h"

namespace Utils
{
    namespace Thread
    {
        //////////////////////////
        // Interfaz de un hilo  //
        //////////////////////////
        
        class IThread
        {
            // Constructor/Destructor
            public:
                IThread(const std::string &threadName = std::string());
                virtual ~IThread() = default;
            
            // Deleted
            public:
                IThread(const IThread&)            = delete;
                IThread &operator=(const IThread&) = delete;

            // Pure virtual
            protected:
                virtual ExitCode worker() = 0;
            
            // Virtual
            public:
                virtual ExitCode workerWrapper();

            // Funciones miembro
            public:
                HANDLE getHandle() const;
                void setHandle(HANDLE threadHandle = nullptr);

                DWORD getId() const;
                void setId(DWORD threadId = 0);
    
                bool isRunning() const;
                void setRunning(bool running = false);
    
                std::string getName() const;
                void setName(const std::string &threadName);

            // Variables miembro
            private:
                HANDLE            threadHandle = nullptr;
                DWORD             threadId     = 0;
                std::atomic<bool> running      = false;
                std::string       threadName;
            };
    };
};
