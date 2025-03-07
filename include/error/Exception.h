#pragma once

#include <windows.h>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include "error/Types.h"
#include "utils/Thread.h"

namespace Error
{
    // Manejo de excepciones
    class ExceptionManager
    {
        private:
            static const std::string DUMP_DLL_NAME;
            static const std::string DUMP_FUNC_MINIDUMP;

            const LPTOP_LEVEL_EXCEPTION_FILTER topExceptionHandler = nullptr;
            const std::terminate_handler       topTerminateHandler = nullptr;

        public:
            ExceptionManager() = delete;
            explicit ExceptionManager(bool isGlobal = false);
            ExceptionManager& operator=(const ExceptionManager&) = delete;
            virtual ~ExceptionManager();

            static LONG WINAPI manageUnhandledException(PEXCEPTION_POINTERS exception);
            static void manageTerminate();
        
        private:
            static LONG manageException(PEXCEPTION_POINTERS exception);
            static LONG manageCriticalMsvcException(PEXCEPTION_POINTERS exception);

            static bool createDumpFile(PEXCEPTION_POINTERS exception);
    };
    
    // Thread con proteccion de excepciones
    class SafeThread: public Utils::Thread
    {
        public:
            SafeThread();
            SafeThread(const SafeThread&) = delete;
            SafeThread& operator=(const SafeThread&) = delete;
            virtual ~SafeThread() = default;

            virtual Error::ExitCode workerWrapper() final;

        protected:
            virtual Error::ExitCode worker();
        
        private:
            Error::ExitCode intermidiateWorker();
    };

    // Thread que genera excepciones C++
    class CppExceptionThread: public SafeThread
    {
        public:
            CppExceptionThread() = default;
            CppExceptionThread(const CppExceptionThread&) = delete;
            CppExceptionThread& operator=(const CppExceptionThread&) = delete;
            virtual ~CppExceptionThread() = default;

        private:
            Error::ExitCode worker() final;
    };

    // Thread que genera excepciones SEH
    class SehExceptionThread: public SafeThread
    { 
        public:
            SehExceptionThread() = default;
            SehExceptionThread(const SehExceptionThread&) = delete;
            SehExceptionThread& operator=(const SehExceptionThread&) = delete;
            virtual ~SehExceptionThread() = default;
        
        private:
            Error::ExitCode worker() final;
    };

    // 
};
