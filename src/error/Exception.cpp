#include "error/Exception.h"

#include <algorithm>
#include <cstdlib>
#include <dbghelp.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include "utils/DllManager.h"

namespace Error
{
    //////////////////////////////////////////////////
    // Informacion minima para generar un mini dump //
    //////////////////////////////////////////////////

    bool MiniDumpRequiredInfo::isValid() const
    {
        return process && processId && threadId;
    }

    //////////////////////////////////////////////////
    // Manejar de excepciones de distintos procesos //
    //////////////////////////////////////////////////

    const std::string ExternalExceptionManager::MANAGER_NAME      = "ExternalException";
    const std::string ExternalExceptionManager::EXTERNAL_APP_NAME = "CppExceptionsAnalysis";

    const DWORD ExternalExceptionManager::EXTERNAL_APP_WAIT_INTERVAL = 5000;
    const DWORD ExternalExceptionManager::EXTERNAL_APP_ANALYSIS_TIME = 2000;
    const DWORD ExternalExceptionManager::EXTERNAL_APP_CLOSE_TIME    = 30000;

    ExternalExceptionManager::ExternalExceptionManager(bool isSender, const Logger::Logger& logger)
        : ILoggerHolder(logger)
        , isSender(isSender)
        , requiredDumpInfo(MANAGER_NAME, isSender, logger)
    {
        // Creamos los eventos
        if (!createEventHandles()) return;

        // Creamos el proceso de ser necesario
        if (isSender && !createAnalysisProcess()) closeManager();
    }
    
    ExternalExceptionManager::~ExternalExceptionManager()
    {
        closeManager();
    }
    
    bool ExternalExceptionManager::isValid()
    {
        std::lock_guard<std::recursive_mutex> lock(analysisMutex);
        return startOfAnalysisHandle && endOfAnalysisHandle && closeAnalysisHandle && (!isSender || isSender && processHandle && threadHandle) && requiredDumpInfo.isValid();
    }

    bool ExternalExceptionManager::sendException(PEXCEPTION_POINTERS exception)
    {
        std::lock_guard<std::recursive_mutex> lock(analysisMutex);
        if (!exception || !isValid()) return false;

        LOGGER_THIS_LOG() << "Enviando excepcion...";

        ExceptionPointers exceptionPointers(*exception);
        LimitedExceptionPointer limitedExceptionPointer;
        limitedExceptionPointer.isValid       = true;
        limitedExceptionPointer.contextRecord = exceptionPointers.contextRecord;

        for (auto itRecord = exceptionPointers.exceptionRecords.begin(); itRecord != exceptionPointers.exceptionRecords.end(); ++itRecord)
        {
            limitedExceptionPointer.exceptionRecord = *itRecord;
            
            // Solicitamos una escritura a la memoria compartida
            if (!requiredDumpInfo.writeData(limitedExceptionPointer))
            {
                LOGGER_THIS_LOG() << "Error enviando datos de excepcion";
                return false;
            }

            // Notificamos al proceso externo
            if (!SetEvent(startOfAnalysisHandle))
            {
                LOGGER_THIS_LOG() << "Error notificando inicio de analisis externo: " << GetLastError();
                return false;
            }

            // Esperamos al proceso externo
            switch (WaitForSingleObject(endOfAnalysisHandle, EXTERNAL_APP_ANALYSIS_TIME))
            {
                case WAIT_OBJECT_0:
                    break;
                
                case WAIT_TIMEOUT:
                    LOGGER_THIS_LOG() << "Timeout esperando proceso de analisis externo " << EXTERNAL_APP_NAME;
                    return false;

                default:
                    LOGGER_THIS_LOG() << "Error esperando proceso de analisis externo " << EXTERNAL_APP_NAME << ": " << GetLastError();
                    return false;
            }
        }

        // Esperamos al cierre notificado por el proceso externo
        switch (WaitForSingleObject(closeAnalysisHandle, EXTERNAL_APP_ANALYSIS_TIME))
        {
            case WAIT_OBJECT_0:
                break;
            
            case WAIT_TIMEOUT:
                LOGGER_THIS_LOG() << "Timeout esperando fin del proceso de analisis externo " << EXTERNAL_APP_NAME;
                return false;

            default:
                LOGGER_THIS_LOG() << "Error esperando fin del proceso de analisis externo " << EXTERNAL_APP_NAME << ": " << GetLastError();
                return false;
        }

        LOGGER_THIS_LOG() << "Excepcion enviada exitosamente";
        return true;
    }

    bool ExternalExceptionManager::receiveException()
    {
        std::lock_guard<std::recursive_mutex> lock(analysisMutex);
        if (!isValid()) return false;

        LOGGER_THIS_LOG() << "Esperando excepcion...";

        // Esperamos al proceso principal
        MiniDumpRequiredInfo           miniDumpInfo;
        Error::ExceptionPointers       exceptionPointers;
        Error::LimitedExceptionPointer limitedExceptionPointer;
        bool exitLoop = false;
        while (!exitLoop)
        {
            switch (WaitForSingleObject(startOfAnalysisHandle, EXTERNAL_APP_WAIT_INTERVAL))
            {
                case WAIT_OBJECT_0:
                    break;
                
                case WAIT_TIMEOUT:
                    if (!exceptionPointers.exceptionRecords.empty())
                    {
                        LOGGER_THIS_LOG() << "Timeout esperando proceso principal";
                        return false;
                    }
                    break;
    
                default:
                    LOGGER_THIS_LOG() << "Error esperando proceso principal: " << GetLastError();
                    return false;
            }

            // Solicitamos una lectura a la memoria compartida
            if (!requiredDumpInfo.readData(limitedExceptionPointer))
            {
                LOGGER_THIS_LOG() << "Error obteniendo datos de excepcion";
                return false;
            }
    
            // Verificamos si los datos son validos
            if (!limitedExceptionPointer.isValid)
            {
                if (!exceptionPointers.exceptionRecords.empty())
                {
                    LOGGER_THIS_LOG() << "Datos de excepcion no validos";
                    return false;
                }
                
                LOGGER_THIS_LOG() << "No existe excepcion";
                return true;
            }

            // Anadimos el registro a la lista
            exceptionPointers.exceptionRecords.push_back(limitedExceptionPointer.exceptionRecord);
            if (limitedExceptionPointer.exceptionRecord.isLast) exitLoop = true;
            
            // Notificamos al proceso principal
            if (!SetEvent(endOfAnalysisHandle))
            {
                LOGGER_THIS_LOG() << "Error notificando fin de analisis externo: " << GetLastError();
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
            LOGGER_THIS_LOG() << "Error obteniendo handle del proceso";
            return false;
        }
        miniDumpInfo.process = processHandle;

        // Gestionamos la excepcion
        bool noDumpError = true;
        if (!ExceptionManager::createDumpFile(miniDumpInfo))
        {
            LOGGER_THIS_LOG() << "Error generando mini dump";
            noDumpError = false;
        }
        CloseHandle(processHandle);

        LOGGER_THIS_LOG() << "Excepcion tratrada exitosamente";
        return noDumpError;
    }

    std::string ExternalExceptionManager::getStartOfAnalysysHandleName() const
    {
        return std::string("Event_Start") + MANAGER_NAME;
    }

    std::string ExternalExceptionManager::getEndOfAnalysysHandleName() const
    {
        return std::string("Event_End") + MANAGER_NAME;
    }

    std::string ExternalExceptionManager::getCloseAnalysysHandleName() const
    {
        return std::string("Event_Close") + MANAGER_NAME;
    }

    std::string ExternalExceptionManager::getAnalysisProcessPath() const
    {
        std::vector<char> exePath(MAX_PATH, 0);

        // Obtenemos el path del .exe principal
        DWORD pathLength = GetModuleFileName(nullptr, exePath.data(), MAX_PATH);
        if (!pathLength)
        {
            LOGGER_THIS_LOG() << "Error obteniendo ruta del proceso de analisis externo: " << GetLastError();
            return std::string();
        }
        exePath.resize(pathLength);
        
        // Recortamos el nombre del ejecutable
        auto itPath = std::find(exePath.rbegin(), exePath.rend(), '\\');
        if (itPath != exePath.rend()) exePath.resize(std::distance(itPath, exePath.rend()));

        return std::string(exePath.begin(), exePath.end()) + EXTERNAL_APP_NAME + ".exe";
    }
    
    bool ExternalExceptionManager::createEventHandles()
    {
        std::lock_guard<std::recursive_mutex> lock(analysisMutex);

        std::string handleName = getStartOfAnalysysHandleName();
        startOfAnalysisHandle = CreateEvent(nullptr
                                          , FALSE
                                          , FALSE
                                          , handleName.c_str());
        if (!startOfAnalysisHandle)
        {
            LOGGER_THIS_LOG() << "Error creando evento de inicio de analisis " << MANAGER_NAME << ": " << GetLastError();
            return false;
        }

        handleName = getEndOfAnalysysHandleName();
        endOfAnalysisHandle = CreateEvent(nullptr
                                        , FALSE
                                        , FALSE
                                        , handleName.c_str());
        if (!endOfAnalysisHandle)
        {
            LOGGER_THIS_LOG() << "Error creando evento de fin de analisis " << MANAGER_NAME << ": " << GetLastError();
            return false;
        }

        handleName = getCloseAnalysysHandleName();
        closeAnalysisHandle = CreateEvent(nullptr
                                        , FALSE
                                        , FALSE
                                        , handleName.c_str());
        if (!closeAnalysisHandle)
        {
            LOGGER_THIS_LOG() << "Error creando evento de cierre de analisis " << MANAGER_NAME << ": " << GetLastError();
            return false;
        }

        return true;
    }

    bool ExternalExceptionManager::createAnalysisProcess()
    {
        if (!isSender) return false;

        std::lock_guard<std::recursive_mutex> lock(analysisMutex);
        if (!startOfAnalysisHandle || !endOfAnalysisHandle || !closeAnalysisHandle) return false;
        if (processHandle && threadHandle) return true;
        processHandle = nullptr;
        threadHandle  = nullptr;

        // Creamos el proceso
        STARTUPINFO         startInfo;
        PROCESS_INFORMATION processInfo;
        ZeroMemory(&startInfo, sizeof(STARTUPINFO));
        ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

        std::string externalAppPath = getAnalysisProcessPath();
        if (!CreateProcess(externalAppPath.c_str()
                         , nullptr
                         , nullptr
                         , nullptr
                         , TRUE
                         , 0
                         , nullptr
                         , nullptr
                         , &startInfo
                         , &processInfo))
        {
            LOGGER_THIS_LOG() << "Error lanzando proceso de analisis externo " << EXTERNAL_APP_NAME << ": " << GetLastError();
            return false;
        }

        // Guardamos los handles asociados al nuevo proceso
        processHandle = processInfo.hProcess;
        threadHandle  = processInfo.hThread;
        return true;
    }

    void ExternalExceptionManager::closeManager()
    {
        std::lock_guard<std::recursive_mutex> lock(analysisMutex);

        // Cerramos el proceso externo
        if (processHandle)
        {
            if (startOfAnalysisHandle && !SetEvent(startOfAnalysisHandle))
                LOGGER_THIS_LOG() << "Error notificando evento de fin de analisis " << MANAGER_NAME << ": " << GetLastError();

            switch (WaitForSingleObject(processHandle, EXTERNAL_APP_CLOSE_TIME))
            {
                case WAIT_OBJECT_0:
                    break;
                
                case WAIT_TIMEOUT:
                    LOGGER_THIS_LOG() << "Timeout esperando fin de proceso de analisis externo " << EXTERNAL_APP_NAME;
                    break;

                default:
                    LOGGER_THIS_LOG() << "Error esperando fin de proceso de analisis externo " << EXTERNAL_APP_NAME << ": " << GetLastError();
                    break;
            }
            
            if (!CloseHandle(processHandle)) LOGGER_THIS_LOG() << "Error cerrando proceso de analisis externo " << EXTERNAL_APP_NAME << ": " << GetLastError();
            processHandle = nullptr;
        }

        if (threadHandle)
        {
            if (!CloseHandle(threadHandle)) LOGGER_THIS_LOG() << "Error cerrando hilo de analisis externo " << EXTERNAL_APP_NAME << ": " << GetLastError();
            threadHandle = nullptr;
        }

        // Cerramos los eventos
        if (closeAnalysisHandle)
        {
            if (!isSender && !SetEvent(closeAnalysisHandle))
                LOGGER_THIS_LOG() << "Error notificando evento de cierre de analisis " << MANAGER_NAME << ": " << GetLastError();

            if (!CloseHandle(closeAnalysisHandle)) LOGGER_THIS_LOG() << "Error cerrando evento de cierre de analisis " << MANAGER_NAME << ": " << GetLastError();
            closeAnalysisHandle = nullptr;
        }

        if (endOfAnalysisHandle)
        {
            if (!CloseHandle(endOfAnalysisHandle)) LOGGER_THIS_LOG() << "Error cerrando evento de fin de analisis " << MANAGER_NAME << ": " << GetLastError();
            endOfAnalysisHandle = nullptr;
        }

        if (startOfAnalysisHandle)
        {
            if (!CloseHandle(startOfAnalysisHandle)) LOGGER_THIS_LOG() << "Error cerrando evento de inicio de analisis " << MANAGER_NAME << ": " << GetLastError();
            startOfAnalysisHandle = nullptr;
        }
    }

    ////////////////////////////
    // Manejo de excepciones  //
    ////////////////////////////

    typedef BOOL (WINAPI *MiniDumpWriteDump)(HANDLE                            processHandle
                                           , DWORD                             processId
                                           , HANDLE                            fileHandle
                                           , MINIDUMP_TYPE                     dumpType
                                           , PMINIDUMP_EXCEPTION_INFORMATION   exceptionInfo
                                           , PMINIDUMP_USER_STREAM_INFORMATION userStreamInfo
                                           , PMINIDUMP_CALLBACK_INFORMATION    callbackInfo);

    const std::string ExceptionManager::DUMP_DLL_NAME      = "dbghelp.dll";
    const std::string ExceptionManager::DUMP_FUNC_MINIDUMP = "MiniDumpWriteDump";
    
    std::unique_ptr<ExternalExceptionManager> ExceptionManager::externalExceptionManager;
    std::recursive_mutex                      ExceptionManager::externalizeMutex;

    Logger::Logger ExceptionManager::logger = Logger::BasicLogger::getInstance();
    std::mutex     ExceptionManager::loggerMutex;

    bool       ExceptionManager::firstException = true;
    std::mutex ExceptionManager::firstExceptionMutex;

    bool ExceptionManager::exceptionError = false;

    ExceptionManager::ExceptionManager(bool isGlobal, bool externalize, const Logger::Logger& logger)
        : topTerminateHandler(std::set_terminate(manageTerminate))
        , topExceptionHandler(isGlobal ? SetUnhandledExceptionFilter(manageUnhandledException) : nullptr)
    {
        if (isGlobal)
        {
            std::atexit(manageSafeExit);

            {
                std::lock_guard<std::mutex> lock(loggerMutex);
                this->logger = logger;
            }

            if (externalize)
            {
                std::lock_guard<std::recursive_mutex> lockExternal(externalizeMutex);
                if (!externalExceptionManager) externalExceptionManager.reset(new ExternalExceptionManager(true, logger));
            }
        }
    }

    ExceptionManager::~ExceptionManager()
    {
        if (topExceptionHandler) SetUnhandledExceptionFilter(topExceptionHandler);
        if (topTerminateHandler) std::set_terminate(topTerminateHandler);
    }

    LONG WINAPI ExceptionManager::manageUnhandledException(PEXCEPTION_POINTERS exception)
    {
        __try
        {
            manageException(exception);
        }
        __except (manageCriticalMsvcException(GetExceptionInformation()))
        {
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void ExceptionManager::manageTerminate()
    {
        if (exceptionError)
        {
            // Damos tiempo a que se escriban todos los logs
            Sleep(Logger::LOGGER_FLUSH_TIMEOUT);
            std::abort();
        }

        Logger::Logger tmpLogger;
        {
            std::lock_guard<std::mutex> lock(loggerMutex);
            tmpLogger = logger;
        }
        LOGGER_LOG(tmpLogger) << "Terminate ejecutado";
        std::exit(0);
    }

    void ExceptionManager::manageSafeExit()
    {
        __try
        {
            manageExit();
        }
        __except (manageCriticalMsvcException(GetExceptionInformation()))
        {
        }
    }

    bool ExceptionManager::createDumpFile(const MiniDumpRequiredInfo& requiredInfo)
    {
        Logger::Logger tmpLogger;
        {
            std::lock_guard<std::mutex> lock(loggerMutex);
            tmpLogger = logger;
        }
        
        // Verificamos los datos obtenidos
        if (!requiredInfo.isValid())
        {
            LOGGER_LOG(tmpLogger) << "Informacion para mini dump incompleta";
            return false;
        }

        // Cargamos la DLL
        std::shared_ptr<Utils::DllWrapper> dllWrapper = Utils::DllManager::getInstance(DUMP_DLL_NAME, tmpLogger);
        if (!dllWrapper || !dllWrapper->isValid())
        {
            LOGGER_LOG(tmpLogger) << "Error cargando libreria " << DUMP_DLL_NAME;
            return false;
        }

        // Obtenemos la funcion de interes
        std::shared_ptr<Utils::DllFunctionWrapper> funcWrapper = dllWrapper->getFunction(DUMP_FUNC_MINIDUMP);
        if (!funcWrapper || !funcWrapper->isValid())
        {
            LOGGER_LOG(tmpLogger) << "Error cargando funcion " << DUMP_FUNC_MINIDUMP;
            return false;
        }

        MiniDumpWriteDump funcAddress = reinterpret_cast<MiniDumpWriteDump>(funcWrapper->getAddress());
        if (!funcAddress)
        {
            LOGGER_LOG(tmpLogger) << "Error traduciendo funcion " << DUMP_FUNC_MINIDUMP;
            return false;
        }
        
        // Creamos el fichero
        std::string fileName = getDumpFileName();
        HANDLE handleFichero = CreateFile(fileName.c_str()
                                        , GENERIC_READ | GENERIC_WRITE
                                        , FILE_SHARE_WRITE | FILE_SHARE_READ
                                        , nullptr
                                        , CREATE_ALWAYS
                                        , 0
                                        , nullptr);
        if (!handleFichero)
        {
            LOGGER_LOG(tmpLogger) << "Error creando fichero " << fileName << ": " << GetLastError();
            return false;
        }

        // Preparamos los datos de la llamada
        MINIDUMP_EXCEPTION_INFORMATION miniDumpInfo;
        miniDumpInfo.ThreadId          = requiredInfo.threadId;
        miniDumpInfo.ExceptionPointers = requiredInfo.exception;
        miniDumpInfo.ClientPointers    = FALSE;

        // Realizamos la llamada para generar el mini dump (scoped para limitar el alcance del lock)
        bool resultado = true;
        {
            std::lock_guard<std::mutex> lock(funcWrapper->getMutex());
            if (!funcAddress(requiredInfo.process, requiredInfo.processId, handleFichero, MiniDumpNormal, &miniDumpInfo, nullptr, nullptr))
            {
                LOGGER_LOG(tmpLogger) << "Error generando dump: " << GetLastError();
                resultado = false;
            }
        }

        // Cerramos el fichero
        CloseHandle(handleFichero);

        // Descargamos la DLL y terminamos
        Utils::DllManager::deleteInstance(DUMP_DLL_NAME);
        return resultado;
    }

    LONG ExceptionManager::manageException(PEXCEPTION_POINTERS exception)
    {
        Logger::Logger tmpLogger;
        {
            std::lock_guard<std::mutex> lock(loggerMutex);
            tmpLogger = logger;
        }
        LOGGER_LOG(tmpLogger) << "Excepcion detectada";
     
        // Verificamos si es la primera excepcion (ignoramos el resto)
        {
            std::lock_guard<std::mutex> lock(firstExceptionMutex);
            if (!firstException)
            {
                LOGGER_LOG(tmpLogger) << "Excepcion descartada";
                return EXCEPTION_EXECUTE_HANDLER;
            }

            firstException = false;
        }

        // Externalizamos el analisis?
        bool manageLocally = true;
        {
            std::lock_guard<std::recursive_mutex> lock(externalizeMutex);
            if (externalExceptionManager && externalExceptionManager->isValid())
                manageLocally = !externalExceptionManager->sendException(exception);
        }

        // Generamos dump si es necesario
        MiniDumpRequiredInfo miniDumpInfo;
        miniDumpInfo.exception = exception;
        if (manageLocally && !createDumpFile(miniDumpInfo)) LOGGER_LOG(tmpLogger) << "Error creando mini dump";
        
        std::terminate();
        return EXCEPTION_EXECUTE_HANDLER;
    }

    LONG ExceptionManager::manageCriticalMsvcException(PEXCEPTION_POINTERS exception)
    {
        Logger::Logger tmpLogger;
        {
            std::lock_guard<std::mutex> lock(loggerMutex);
            tmpLogger = logger;
        }
        LOGGER_LOG(tmpLogger) << "Excepcion CRITICA detectada";

        exceptionError = true;

        std::terminate();
        return EXCEPTION_EXECUTE_HANDLER;
    }
    
    void ExceptionManager::manageExit()
    {
        Logger::Logger tmpLogger;
        {
            std::lock_guard<std::mutex> lock(loggerMutex);
            tmpLogger = logger;
        }
        LOGGER_LOG(tmpLogger) << "Exit ejecutado";

        // Damos tiempo a que se escriban todos los logs
        Sleep(Logger::LOGGER_FLUSH_TIMEOUT);
    }

    std::string ExceptionManager::getDumpFileName()
    {
        SYSTEMTIME stNow;
        GetLocalTime(&stNow);

        std::stringstream ssFileName;
        ssFileName << std::setw(4) << std::setfill('0') << stNow.wYear
                   << std::setw(2) << std::setfill('0') << stNow.wMonth
                   << std::setw(2) << std::setfill('0') << stNow.wDay
                   << "_"
                   << std::setw(2) << std::setfill('0') << stNow.wHour
                   << std::setw(2) << std::setfill('0') << stNow.wMinute
                   << std::setw(2) << std::setfill('0') << stNow.wSecond
                   << std::setw(3) << std::setfill('0') << stNow.wMilliseconds
                   << "_crashdump.dmp";
        
        return ssFileName.str();
    }

    //////////////////////////////////////////
    // Thread con proteccion de excepciones //
    //////////////////////////////////////////

    SafeThread::SafeThread()
        : Thread()
    {
    }

    Error::ExitCode SafeThread::workerWrapper()
    {
        Error::ExitCode resultado = Error::ExitCode::EXIT_CODE_OK;

        // Protegemos la ejecucion
        __try
        {
            resultado = intermidiateWorker();
        }
        __except (Error::ExceptionManager::manageUnhandledException(GetExceptionInformation()))
        {
            resultado = Error::ExitCode::EXIT_CODE_EXCEPTION;
        }

        return resultado;
    }

    Error::ExitCode SafeThread::worker()
    {
        return Error::ExitCode::EXIT_CODE_NOT_IMPLEMENTED;
    }
    
    Error::ExitCode SafeThread::intermidiateWorker()
    {
        ExceptionManager exceptionManager;
        return this->worker();
    }

    //////////////////////////////////////////////////////
    // Clase para generar excepciones C++ en un thread  //
    //////////////////////////////////////////////////////

    Error::ExitCode CppExceptionThread::worker()
    {
        throw std::runtime_error("Threaded C++ Exception");
        return Error::ExitCode::EXIT_CODE_OK;
    }

    ////////////////////////////////////////
    // Thread que genera excepciones SEH  //
    ////////////////////////////////////////
    
    Error::ExitCode SehExceptionThread::worker()
    {
        int *p = nullptr;
        *p = 20;
        return Error::ExitCode::EXIT_CODE_OK;
    }
};
