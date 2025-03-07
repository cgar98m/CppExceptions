#include "error/Exception.h"

#include <dbghelp.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "logger/ConsoleLogger.h"
#include "utils/DllManager.h"

namespace Error
{
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
    
    ExceptionManager::ExceptionManager(bool isGlobal)
        : topTerminateHandler(std::set_terminate(manageTerminate))
        , topExceptionHandler(isGlobal ? SetUnhandledExceptionFilter(manageUnhandledException) : nullptr)
    {
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
        std::shared_ptr<Logger::ILogger> logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(logger) << "Terminate ejecutado";

        // Si se quiere esperar a que se cierre el programa adecuadamente, este es el punto de no retorno
        std::abort();
    }

    LONG ExceptionManager::manageException(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ILogger> logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(logger) << "Excepcion detectada";
        
        if (!createDumpFile(exception)) LOGGER_LOG(logger) << "Error creando dump";
        
        std::terminate();
        return EXCEPTION_EXECUTE_HANDLER;
    }

    LONG ExceptionManager::manageCriticalMsvcException(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ILogger> logger = Logger::ConsoleLogger::getInstance();
        LOGGER_LOG(logger) << "Excepcion CRITICA detectada";
        std::terminate();
        return EXCEPTION_EXECUTE_HANDLER;
    }

    bool ExceptionManager::createDumpFile(PEXCEPTION_POINTERS exception)
    {
        std::shared_ptr<Logger::ILogger> logger = Logger::ConsoleLogger::getInstance();
        
        // Cargamos la DLL
        std::shared_ptr<Utils::DllWrapper> dllWrapper = Utils::DllManager::getInstance(DUMP_DLL_NAME);
        if (!dllWrapper || !dllWrapper->isValid())
        {
            LOGGER_LOG(logger) << "Error cargando libreria " << DUMP_DLL_NAME;
            return false;
        }

        // Obtenemos la funcion de interes
        std::shared_ptr<Utils::DllFunctionWrapper> funcWrapper = dllWrapper->getFunction(DUMP_FUNC_MINIDUMP);
        if (!funcWrapper || !funcWrapper->isValid())
        {
            LOGGER_LOG(logger) << "Error cargando funcion " << DUMP_FUNC_MINIDUMP;
            return false;
        }

        MiniDumpWriteDump funcAddress = reinterpret_cast<MiniDumpWriteDump>(funcWrapper->getAddress());
        if (!funcAddress)
        {
            LOGGER_LOG(logger) << "Error traduciendo funcion " << DUMP_FUNC_MINIDUMP;
            return false;
        }

        // Preparamos el nombre del fichero de dump
        SYSTEMTIME stNow;
        GetLocalTime(&stNow);

        std::stringstream ssFileName;
        ssFileName << std::setw(4) << std::setfill('0') << stNow.wYear
                   << std::setw(2) << std::setfill('0') << stNow.wMonth
                   << std::setw(2) << std::setfill('0') << stNow.wDay
                   << "_"
                   << std::setw(2) << std::setfill('0') << stNow.wSecond 
                   << std::setw(3) << std::setfill('0') << stNow.wMilliseconds
                   << "_crashdump.dmp";
        std::string fileName = ssFileName.str();
        
        // Creamos el fichero
        HANDLE handleFichero = CreateFile(fileName.c_str()
                                        , GENERIC_READ | GENERIC_WRITE
                                        , FILE_SHARE_WRITE | FILE_SHARE_READ
                                        , nullptr
                                        , CREATE_ALWAYS
                                        , 0
                                        , nullptr);
        if (!handleFichero)
        {
            LOGGER_LOG(logger) << "Error creando fichero " << fileName << ": " << GetLastError();
            return false;
        }

        // Preparamos los datos de la llamada
        MINIDUMP_EXCEPTION_INFORMATION miniDumpInfo;
        miniDumpInfo.ThreadId          = GetCurrentThreadId();
        miniDumpInfo.ExceptionPointers = exception;
        miniDumpInfo.ClientPointers    = FALSE;

        // Realizamos la llamada para generar el mini dump (scoped para limitar el alcance del lock)
        bool resultado = true;
        {
            std::lock_guard<std::mutex> lock(funcWrapper->getMutex());
            if (!funcAddress(GetCurrentProcess(), GetCurrentProcessId(), handleFichero, MiniDumpNormal, &miniDumpInfo, nullptr, nullptr))
            {
                LOGGER_LOG(logger) << "Error generando dump: " << GetLastError();
                resultado = false;
            }
        }

        // Cerramos el fichero
        CloseHandle(handleFichero);

        // Descargamos la DLL y terminamos
        Utils::DllManager::deleteInstance(DUMP_DLL_NAME);
        return resultado;
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
        ExceptionManager exceptionManager(false);
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
