#include "utils/thread/IThread.h"

namespace Utils
{
    namespace Thread
    {
        //////////////////////////
        // Interfaz de un hilo  //
        //////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        IThread::IThread(const std::string &threadName)
            : threadName(threadName)
        {
        }

        //----------//
        // Virtual  //
        //----------//

        ExitCode IThread::workerWrapper()
        {
            return this->worker();
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        HANDLE IThread::getHandle() const
        {
            return this->threadHandle;
        }
    
        void IThread::setHandle(HANDLE threadHandle)
        {
            this->threadHandle = threadHandle;
        }
    
        DWORD IThread::getId() const
        {
            return this->threadId;
        }
    
        void IThread::setId(DWORD threadId)
        {
            this->threadId = threadId;
        }
    
        bool IThread::isRunning() const
        {
            return this->running;
        }
    
        void IThread::setRunning(bool running)
        {
            this->running = running;
        }

        std::string IThread::getName() const
        {
            return this->threadName;
        }

        void IThread::setName(const std::string &threadName)
        {
            this->threadName = threadName;
        }
    };
};
