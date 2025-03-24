#pragma once

#include <windows.h>
#include <string>
#include "utils/container/SemaphoredQueue.hpp"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/thread/ISafeThread.h"

namespace Utils
{
    namespace Thread
    {
        //////////////////////////////////////////////
        // Hilo con gestion de elementos encolados  //
        //////////////////////////////////////////////

        template <typename T>
        class IQueueThread: public ISafeThread
        {
            //------------//
            // Constantes //
            //------------//

            public:
                static const DWORD TIMEOUT_MS_DATA_WAIT;
            
            private:
                static const char *THREAD_NAME;

            //------------------------//
            // Constructor/Destructor //
            //------------------------//

            public:
                IQueueThread(const std::string &threadName = THREAD_NAME, DWORD dataWait = TIMEOUT_MS_DATA_WAIT, const SharedLogger &logger = BASIC_LOGGER())
                    : ISafeThread(threadName)
                    , dataQueue(logger)
                    , dataWait(dataWait)
                {
                }

                virtual ~IQueueThread() = default;
            
            //----------//
            // Deleted  //
            //----------//

            public:
                IQueueThread(const IQueueThread&)            = delete;
                IQueueThread &operator=(const IQueueThread&) = delete;
                
            //--------------//
            // Pure virtual //
            //--------------//

            private:
                virtual bool processData(const T &data) = 0;
    
            //----------------//
            // Final virtual  //
            //----------------//
        
            public:
                virtual ExitCode worker() final
                {
                    T data;
                    switch (this->dataQueue.top(data, this->dataWait))
                    {
                        case WAIT_OBJECT_0:
                            break;
            
                        case WAIT_TIMEOUT:
                            return ExitCode::EXIT_CODE_OK;
                        
                        case WAIT_ABANDONED:
                        case WAIT_FAILED:
                        default:
                            return ExitCode::EXIT_CODE_KO;
                    }
            
                    if (!this->processData(data))                       return ExitCode::EXIT_CODE_KO;
                    if (this->dataQueue.pop(data, 0) != WAIT_OBJECT_0) return ExitCode::EXIT_CODE_KO;
                    return ExitCode::EXIT_CODE_OK;
                }
                
            //--------------------//
            // Funciones miembro  //
            //--------------------//

            protected:
                bool pushData(const T &data)
                {
                    return this->dataQueue.push(data);
                }
            
            //--------------------//
            // Variables miembro  //
            //--------------------//

            private:
                DWORD                         dataWait = TIMEOUT_MS_DATA_WAIT;
                Container::SemaphoredQueue<T> dataQueue;
        };

        //////////////////////////////////
        // Instanciacion de constantes  //
        //////////////////////////////////
        
        template <typename T> const DWORD IQueueThread<T>::TIMEOUT_MS_DATA_WAIT = 1000;
        template <typename T> const char  *IQueueThread<T>::THREAD_NAME         = "IQueueThread";
    };
};
