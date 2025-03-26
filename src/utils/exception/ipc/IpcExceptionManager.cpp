#include "utils/exception/ipc/IpcExceptionManager.h"

#include "Name.h"
#include "utils/ExitCode.h"
#include "utils/logging/LogEntry.h"
#include "utils/exception/ipc/ExceptionPointers.h"
#include "utils/exception/minidump/MiniDumpInfo.h"
#include "utils/exception/minidump/MiniDumpTools.h"
#include "utils/filesystem/FileTools.h"

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////
        // Manejador de excepciones entre procesos  //
        //////////////////////////////////////////////

        //------------//
        // Constantes //
        //------------//

        const char *IpcExceptionManager::EXTERNAL_IDENTIFIER_PARAM = "ExternalIdentifier";

        const char *IpcExceptionManager::MANAGER_NAME             = "IpcExceptionManager";
        const char *IpcExceptionManager::MANAGER_EVENT_START_NAME = "EventStart";
        const char *IpcExceptionManager::MANAGER_EVENT_END_NAME   = "EventEnd";
        const char *IpcExceptionManager::MANAGER_EVENT_CLOSE_NAME = "EventClose";

        const DWORD IpcExceptionManager::EXTERNAL_APP_WAIT_INTERVAL = 5000;
        const DWORD IpcExceptionManager::EXTERNAL_APP_ANALYSIS_TIME = 2000;
        const DWORD IpcExceptionManager::EXTERNAL_APP_CLOSE_TIME    = 30000;

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        IpcExceptionManager::IpcExceptionManager(const std::string &analysisAppTag, bool isSender, const SharedLogger &logger)
            : Logging::LoggerHolder(logger)
            , analysisAppTag(analysisAppTag)
            , isSender(isSender)
            , requiredDumpInfo(std::string(MANAGER_NAME) + std::string("/") + analysisAppTag, isSender, logger)
        {
            // Verificamos la memoria compartida
            if (!this->requiredDumpInfo.isValid()) return;

            // Creamos los eventos
            if (!this->createEventHandles()) return;

            // Creamos el proceso de ser necesario
            this->createAnalysisProcess();
        }
        
        IpcExceptionManager::~IpcExceptionManager()
        {
            this->closeManager();
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        bool IpcExceptionManager::isValid()
        {
            std::lock_guard<std::recursive_mutex> lock(this->analysisMutex);
            return this->startOfAnalysisHandle && this->endOfAnalysisHandle && this->closeAnalysisHandle &&
                   (!this->isSender || this->isSender && this->threadHandle) &&
                   this->requiredDumpInfo.isValid();
        }

        bool IpcExceptionManager::sendException(PEXCEPTION_POINTERS exception)
        {
            std::lock_guard<std::recursive_mutex> lock(this->analysisMutex);
            if (!exception || !this->isValid()) return false;

            LOGGER_THIS_LOG_INFO() << "Enviando excepcion " << this->analysisAppTag;

            ExceptionPointers       exceptionPointers(*exception);
            LimitedExceptionPointer limitedExceptionPointer;
            limitedExceptionPointer.isValid       = true;
            limitedExceptionPointer.contextRecord = exceptionPointers.contextRecord;

            for (auto itRecord = exceptionPointers.exceptionRecords.begin(); itRecord != exceptionPointers.exceptionRecords.end(); ++itRecord)
            {
                limitedExceptionPointer.exceptionRecord = *itRecord;
                
                // Solicitamos una escritura a la memoria compartida
                if (!this->requiredDumpInfo.writeData(limitedExceptionPointer))
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR enviando datos de excepcion " << this->analysisAppTag;
                    return false;
                }

                // Notificamos al proceso externo
                if (!SetEvent(this->startOfAnalysisHandle))
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR notificando inicio de analisis externo " << this->analysisAppTag << ": " << GetLastError();
                    return false;
                }

                // Esperamos al proceso externo
                switch (WaitForSingleObject(this->endOfAnalysisHandle, EXTERNAL_APP_ANALYSIS_TIME))
                {
                    case WAIT_OBJECT_0:
                        break;
                    
                    case WAIT_TIMEOUT:
                        LOGGER_THIS_LOG_ERROR() << "TIMEOUT esperando proceso de analisis externo " << this->analysisAppTag;
                        return false;

                    default:
                        LOGGER_THIS_LOG_ERROR() << "ERROR esperando proceso de analisis externo " << this->analysisAppTag << ": " << GetLastError();
                        return false;
                }
            }

            // Esperamos al cierre notificado por el proceso externo
            switch (WaitForSingleObject(this->closeAnalysisHandle, EXTERNAL_APP_ANALYSIS_TIME))
            {
                case WAIT_OBJECT_0:
                    break;
                
                case WAIT_TIMEOUT:
                    LOGGER_THIS_LOG_ERROR() << "TIMEOUT esperando fin del proceso de analisis externo " << this->analysisAppTag;
                    return false;

                default:
                    LOGGER_THIS_LOG_ERROR() << "ERROR esperando fin del proceso de analisis externo " << this->analysisAppTag << ": " << GetLastError();
                    return false;
            }

            LOGGER_THIS_LOG_INFO() << "Excepcion " << this->analysisAppTag << " enviada exitosamente";
            return true;
        }

        bool IpcExceptionManager::receiveException()
        {
            std::lock_guard<std::recursive_mutex> lock(this->analysisMutex);
            if (!this->isValid()) return false;

            LOGGER_THIS_LOG_INFO() << "Esperando excepcion " << this->analysisAppTag;

            // Esperamos al proceso principal
            MiniDumpInfo            miniDumpInfo;
            ExceptionPointers       exceptionPointers;
            LimitedExceptionPointer limitedExceptionPointer;
            bool exitLoop = false;
            while (!exitLoop)
            {
                switch (WaitForSingleObject(this->startOfAnalysisHandle, EXTERNAL_APP_WAIT_INTERVAL))
                {
                    case WAIT_OBJECT_0:
                        break;
                    
                    case WAIT_TIMEOUT:
                        if (!exceptionPointers.exceptionRecords.empty())
                        {
                            LOGGER_THIS_LOG_ERROR() << "TIMEOUT esperando proceso principal " << this->analysisAppTag;
                            return false;
                        }
                        break;
        
                    default:
                        LOGGER_THIS_LOG_ERROR() << "ERROR esperando proceso principal " << this->analysisAppTag << ": " << GetLastError();
                        return false;
                }

                // Solicitamos una lectura a la memoria compartida
                if (!this->requiredDumpInfo.readData(limitedExceptionPointer))
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo datos de excepcion " << this->analysisAppTag;
                    return false;
                }
        
                // Verificamos si los datos son validos
                if (!limitedExceptionPointer.isValid)
                {
                    if (!exceptionPointers.exceptionRecords.empty())
                    {
                        LOGGER_THIS_LOG_ERROR() << "Datos de excepcion " << this->analysisAppTag << " no validos";
                        return false;
                    }
                    
                    LOGGER_THIS_LOG_INFO() << "No existe excepcion " << this->analysisAppTag;
                    return true;
                }

                // Anadimos el registro a la lista
                exceptionPointers.exceptionRecords.push_back(limitedExceptionPointer.exceptionRecord);
                if (limitedExceptionPointer.exceptionRecord.isLast) exitLoop = true;
                
                // Notificamos al proceso principal
                if (!SetEvent(this->endOfAnalysisHandle))
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR notificando fin de analisis externo " << this->analysisAppTag << ": " << GetLastError();
                    return false;
                }
            }

            // Preparamos los datos de la excepcion
            exceptionPointers.contextRecord = limitedExceptionPointer.contextRecord;
            miniDumpInfo.processId          = limitedExceptionPointer.processId;
            miniDumpInfo.threadId           = limitedExceptionPointer.threadId;
            miniDumpInfo.exception          = exceptionPointers();

            // Obtenemos el handle asociado al id del proceso
            HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS
                                             , FALSE
                                             , miniDumpInfo.processId);
            if (!processHandle)
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo handle del proceso " << this->analysisAppTag;
                return false;
            }
            miniDumpInfo.process = processHandle;

            // Gestionamos la excepcion
            bool noDumpError = true;
            if (!MiniDumpTools::createDumpFile(miniDumpInfo, THIS_LOGGER()))
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR generando mini dump " << this->analysisAppTag;
                noDumpError = false;
            }
            else
            {
                LOGGER_THIS_LOG_INFO() << "Excepcion " << this->analysisAppTag << " tratada exitosamente";
            }

            // Cerramos el handle del proceso principal
            CloseHandle(processHandle);
            return noDumpError;
        }

        std::string IpcExceptionManager::getStartOfAnalysysHandleName() const
        {
            return std::string(MANAGER_NAME) + std::string("/") + this->analysisAppTag + std::string("/") + std::string(MANAGER_EVENT_START_NAME);
        }

        std::string IpcExceptionManager::getEndOfAnalysysHandleName() const
        {
            return std::string(MANAGER_NAME) + std::string("/") + this->analysisAppTag + std::string("/") + std::string(MANAGER_EVENT_END_NAME);
        }

        std::string IpcExceptionManager::getCloseAnalysysHandleName() const
        {
            return std::string(MANAGER_NAME) + std::string("/") + this->analysisAppTag + std::string("/") + std::string(MANAGER_EVENT_CLOSE_NAME);
        }

        std::string IpcExceptionManager::getAnalysisProcessPath() const
        {
            return FileSystem::FileTools::getExecutablePath() + std::string(NAME_ANALYSIS_APP) + ".exe";
        }

        std::string IpcExceptionManager::getAnalysisProcessParams() const
        {
            return std::string(EXTERNAL_IDENTIFIER_PARAM) + std::string(" ") + std::string(this->analysisAppTag);
        }
        
        bool IpcExceptionManager::createEventHandles()
        {
            std::lock_guard<std::recursive_mutex> lock(this->analysisMutex);

            // Creamos el evento de inicio de analisis de registro
            std::string handleName      = this->getStartOfAnalysysHandleName();
            this->startOfAnalysisHandle = CreateEvent(nullptr
                                                    , FALSE
                                                    , FALSE
                                                    , handleName.c_str());
            if (!this->startOfAnalysisHandle || (this->isSender && !ResetEvent(this->startOfAnalysisHandle)))
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR creando evento de inicio de analisis " << this->analysisAppTag << ": " << GetLastError();
                this->closeManager();
                return false;
            }

            // Creamos el evento de fin de analisis de registro
            handleName                = this->getEndOfAnalysysHandleName();
            this->endOfAnalysisHandle = CreateEvent(nullptr
                                                  , FALSE
                                                  , FALSE
                                                  , handleName.c_str());
            if (!this->endOfAnalysisHandle || (this->isSender && !ResetEvent(this->endOfAnalysisHandle)))
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR creando evento de fin de analisis " << this->analysisAppTag << ": " << GetLastError();
                this->closeManager();
                return false;
            }

            // Creamos el evento de cierre del analisis
            handleName                = this->getCloseAnalysysHandleName();
            this->closeAnalysisHandle = CreateEvent(nullptr
                                                  , FALSE
                                                  , FALSE
                                                  , handleName.c_str());
            if (!this->closeAnalysisHandle || (this->isSender && !ResetEvent(this->closeAnalysisHandle)))
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR creando evento de cierre de analisis " << this->analysisAppTag << ": " << GetLastError();
                this->closeManager();
                return false;
            }

            return true;
        }

        bool IpcExceptionManager::createAnalysisProcess()
        {
            // Verificamos los minimos para seguir adelante
            if (!this->isSender) return false;

            std::lock_guard<std::recursive_mutex> lock(this->analysisMutex);
            if (!this->startOfAnalysisHandle || !this->endOfAnalysisHandle || !this->closeAnalysisHandle)
            {
                this->closeManager();
                return false;
            }
            if (this->threadHandle) return true;

            // Creamos el proceso
            STARTUPINFO         startInfo;
            PROCESS_INFORMATION processInfo;
            ZeroMemory(&startInfo, sizeof(STARTUPINFO));
            ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

            std::string cmdCommand = this->getAnalysisProcessPath() + " " + this->getAnalysisProcessParams();
            std::vector<char> charCmdCommand(cmdCommand.begin(), cmdCommand.end());
            charCmdCommand.push_back(0);
            if (!CreateProcess(nullptr
                , charCmdCommand.data()
                , nullptr
                , nullptr
                , TRUE
                , 0
                , nullptr
                , nullptr
                , &startInfo
                , &processInfo))
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR lanzando proceso de analisis externo " << NAME_ANALYSIS_APP << ": " << GetLastError();
                return false;
            }

            // Guardamos el handle del hilo asociado al nuevo proceso (cerramos el del proceso al no ser util)
            this->threadHandle = processInfo.hThread;
            CloseHandle(processInfo.hProcess);
            return true;
        }

        void IpcExceptionManager::closeManager()
        {
            std::lock_guard<std::recursive_mutex> lock(this->analysisMutex);

            // Cerramos el proceso externo
            if (this->threadHandle)
            {
                if (this->startOfAnalysisHandle && !SetEvent(this->startOfAnalysisHandle))
                    LOGGER_THIS_LOG_INFO() << "ERROR notificando evento de fin de analisis " << this->analysisAppTag << ": " << GetLastError();

                bool terminateThread = false;
                switch (WaitForSingleObject(this->threadHandle, EXTERNAL_APP_CLOSE_TIME))
                {
                    case WAIT_OBJECT_0:
                        break;
                        
                    case WAIT_TIMEOUT:
                        LOGGER_THIS_LOG_WARNING() << "TIMEOUT esperando fin de proceso de analisis externo " << this->analysisAppTag;
                        terminateThread = true;
                        break;
                        
                    case WAIT_FAILED:
                    default:
                        LOGGER_THIS_LOG_WARNING() << "ERROR esperando fin de proceso de analisis externo " << this->analysisAppTag << ": " << GetLastError();
                        terminateThread = true;
                        break;
                }
    
                // Forzamos el cierre del hilo (si fuera necesario)
                if (terminateThread && !TerminateThread(this->threadHandle, static_cast<DWORD>(ExitCode::EXIT_CODE_TERMINATE)))
                    LOGGER_THIS_LOG_WARNING() << "ERROR forzando detencion del proceso " << this->analysisAppTag;
                
                // Cerramos el handle
                if (!CloseHandle(this->threadHandle)) LOGGER_THIS_LOG_WARNING() << "ERROR cerrando proceso de analisis externo " << this->analysisAppTag << ": " << GetLastError();
                this->threadHandle = nullptr;
            }

            // Cerramos los eventos
            if (this->closeAnalysisHandle)
            {
                if (!this->isSender && !SetEvent(this->closeAnalysisHandle))
                    LOGGER_THIS_LOG_WARNING() << "ERROR notificando evento de cierre de analisis " << this->analysisAppTag << ": " << GetLastError();

                if (!CloseHandle(this->closeAnalysisHandle)) LOGGER_THIS_LOG_WARNING() << "ERROR cerrando evento de cierre de analisis " << this->analysisAppTag << ": " << GetLastError();
                this->closeAnalysisHandle = nullptr;
            }

            if (this->endOfAnalysisHandle)
            {
                if (!CloseHandle(this->endOfAnalysisHandle)) LOGGER_THIS_LOG_WARNING() << "ERROR cerrando evento de fin de analisis " << this->analysisAppTag << ": " << GetLastError();
                this->endOfAnalysisHandle = nullptr;
            }

            if (this->startOfAnalysisHandle)
            {
                if (!CloseHandle(this->startOfAnalysisHandle)) LOGGER_THIS_LOG_WARNING() << "ERROR cerrando evento de inicio de analisis " << this->analysisAppTag << ": " << GetLastError();
                this->startOfAnalysisHandle = nullptr;
            }
        }
    };
};
