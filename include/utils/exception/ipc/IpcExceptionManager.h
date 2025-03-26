#pragma once

#include <windows.h>
#include <memory>
#include <mutex>
#include <string>
#include "utils/exception/ipc/LimitedExceptionPointer.h"
#include "utils/ipc/SharedMemory.hpp"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////
        // Manejador de excepciones entre procesos  //
        //////////////////////////////////////////////

        class IpcExceptionManager: public Logging::LoggerHolder
        {
            // Constantes
            public:
                static const char *EXTERNAL_IDENTIFIER_PARAM;

            private:
                static const char *MANAGER_NAME;
                static const char *MANAGER_EVENT_START_NAME;
                static const char *MANAGER_EVENT_END_NAME;
                static const char *MANAGER_EVENT_CLOSE_NAME;

                static const DWORD EXTERNAL_APP_WAIT_INTERVAL;
                static const DWORD EXTERNAL_APP_ANALYSIS_TIME;
                static const DWORD EXTERNAL_APP_CLOSE_TIME;

            // Constructor/Destructor
            public:
                IpcExceptionManager(const std::string &analysisAppName, bool isSender = true, const SharedLogger &logger = BASIC_LOGGER());
                virtual ~IpcExceptionManager();
            
            // Deleted
            public:
                IpcExceptionManager(const IpcExceptionManager&)            = delete;
                IpcExceptionManager &operator=(const IpcExceptionManager&) = delete;

            // Funciones miembro
            public:
                bool isValid();

                bool sendException(PEXCEPTION_POINTERS exception);
                bool receiveException();

            private:
                std::string getStartOfAnalysysHandleName() const;
                std::string getEndOfAnalysysHandleName() const;
                std::string getCloseAnalysysHandleName() const;
                
                std::string getAnalysisProcessPath() const;
                std::string getAnalysisProcessParams() const;

                bool createEventHandles();
                bool createAnalysisProcess();

                void closeManager();

            // Variables miembro
            private:
                bool                                       isSender;
                std::string                                analysisAppTag;
                Ipc::SharedMemory<LimitedExceptionPointer> requiredDumpInfo;
                
                HANDLE startOfAnalysisHandle = nullptr;
                HANDLE endOfAnalysisHandle   = nullptr;
                HANDLE closeAnalysisHandle   = nullptr;
                HANDLE threadHandle          = nullptr;

                std::recursive_mutex analysisMutex;
        };
    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using UniqueIpcExceptionManager = std::unique_ptr<Utils::Exception::IpcExceptionManager>;
