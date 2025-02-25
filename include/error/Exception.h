#pragma once

#include <windows.h>

#include <exception>
#include <memory>
#include <mutex>
#include <string>

namespace Error
{
    // Manejo de excepciones
    class ExceptionManager
    {
        public:
            static LONG WINAPI manageMsvcException(PEXCEPTION_POINTERS exception);
            static void manageTerminate();
        
        private:
            static LONG manageException(PEXCEPTION_POINTERS exception);
            static LONG manageCriticalMsvcException(PEXCEPTION_POINTERS exception);
    };

    // Wrapper de un thread que captura excepciones
    class SafeThread
    {
        public:
            static const DWORD TIMEOUT_MS_STOP_WAIT;
            static const DWORD TIMEOUT_MS_LOOP_WAIT;
            
            struct Params
            {
                int                          iThreadPriority    = THREAD_PRIORITY_NORMAL;
                DWORD                        dwMaxStopMsTimeout = SafeThread::TIMEOUT_MS_STOP_WAIT;
                DWORD                        dwLoopWaitTimeout  = SafeThread::TIMEOUT_MS_LOOP_WAIT;
                LPTOP_LEVEL_EXCEPTION_FILTER exceptionHandler   = nullptr;
                std::terminate_handler       terminateHandler   = nullptr;
            };

        private:
            static const DWORD THREAD_EXIT_CODE_EXCEPTION;
            static const DWORD THREAD_EXIT_CODE_TERMINATE;
            
            Params m_params;

            std::recursive_mutex m_threadMux;
            HANDLE m_thread     = nullptr;
            DWORD  m_dwThreadId = 0;
            bool   m_bRunning   = false;

        protected:
            static const DWORD THREAD_EXIT_CODE_OK;
            static const DWORD THREAD_EXIT_CODE_KO;

        public:
            explicit SafeThread(const Params& params);
            virtual ~SafeThread();

            bool run();
            bool stop();
        
            static DWORD __stdcall wrapperThread(LPVOID param);
            
        private:
            DWORD configureThread();
            DWORD protectedThread();
            DWORD loopThread();

        protected:
            virtual DWORD workingThread();
    };

    // Clase para generar excepciones C++ en un thread
    class CppExceptionThread: public SafeThread
    {
        private:
            static const std::string EXCEPTION_MSG;

        public:
            CppExceptionThread();
            virtual ~CppExceptionThread();
        
        private:
            DWORD workingThread() override;
    };

    // Clase para generar excepciones SEH en un thread
    class SehExceptionThread: public SafeThread
    { 
        public:
            SehExceptionThread();
            virtual ~SehExceptionThread();
        
        private:
            DWORD workingThread() override;
    };
};
