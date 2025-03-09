#pragma once

#include <windows.h>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include "error/MsvcException.h"
#include "error/Types.h"
#include "logger/ConsoleLogger.h"
#include "utils/SharedMemory.hpp"
#include "utils/Thread.h"

namespace Error
{
    // Informacion minima para generar un mini dump
    struct MiniDumpRequiredInfo
    {
        HANDLE              process   = GetCurrentProcess();
        DWORD               processId = GetCurrentProcessId();
        DWORD               threadId  = GetCurrentThreadId();
        PEXCEPTION_POINTERS exception = nullptr;

        bool isValid() const;
    };
    
    // Manejar de excepciones de distintos procesos
    class ExternalExceptionManager: public Logger::ILoggerHolder
    {
        private:
            static const std::string MANAGER_NAME;
            static const std::string EXTERNAL_APP_NAME;

            static const DWORD EXTERNAL_APP_WAIT_INTERVAL;
            static const DWORD EXTERNAL_APP_ANALYSIS_TIME;
            static const DWORD EXTERNAL_APP_CLOSE_TIME;

            bool isSender;
            Utils::SharedMemory<LimitedExceptionPointer> requiredDumpInfo;
            
            HANDLE     startOfAnalysisHandle = nullptr;
            HANDLE     endOfAnalysisHandle   = nullptr;
            HANDLE     closeAnalysisHandle   = nullptr;
            HANDLE     processHandle         = nullptr;
            HANDLE     threadHandle          = nullptr;
            std::recursive_mutex analysisMutex;

        public:
            ExternalExceptionManager(bool isSender = true, const Logger::Logger& logger = Logger::ConsoleLogger::getInstance());
            ExternalExceptionManager(const ExternalExceptionManager&) = delete;
            ExternalExceptionManager& operator=(const ExternalExceptionManager&) = delete;
            virtual ~ExternalExceptionManager();

            bool isValid();

            bool sendException(PEXCEPTION_POINTERS exception);
            bool receiveException();

        private:
            std::string getStartOfAnalysysHandleName() const;
            std::string getEndOfAnalysysHandleName() const;
            std::string getCloseAnalysysHandleName() const;
            std::string getAnalysisProcessPath() const;

            bool createEventHandles();
            bool createAnalysisProcess();

            void closeManager();
    };

    // Manejo de excepciones
    class ExceptionManager
    {
        private:
            static const std::string DUMP_DLL_NAME;
            static const std::string DUMP_FUNC_MINIDUMP;

            static std::unique_ptr<ExternalExceptionManager> externalExceptionManager;
            static std::recursive_mutex                      externalizeMutex;

            static Logger::Logger logger;
            static std::mutex     loggerMutex;

            static bool       firstException;
            static std::mutex firstExceptionMutex;

            static bool exceptionError;

            const LPTOP_LEVEL_EXCEPTION_FILTER topExceptionHandler = nullptr;
            const std::terminate_handler       topTerminateHandler = nullptr;

        public:
            ExceptionManager(bool isGlobal = false, bool externalize = false, const Logger::Logger& logger = Logger::ConsoleLogger::getInstance());
            ExceptionManager& operator=(const ExceptionManager&) = delete;
            virtual ~ExceptionManager();

            static LONG WINAPI manageUnhandledException(PEXCEPTION_POINTERS exception);
            static void manageTerminate();
        
            static bool createDumpFile(const MiniDumpRequiredInfo& requiredInfo);
            
        private:
            static LONG manageException(PEXCEPTION_POINTERS exception);
            static LONG manageCriticalMsvcException(PEXCEPTION_POINTERS exception);

            static std::string getDumpFileName();
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
};
