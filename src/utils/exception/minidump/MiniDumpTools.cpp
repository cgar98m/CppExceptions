#include "utils/exception/minidump/MiniDumpTools.h"

#include <iomanip>
#include <mutex>
#include <sstream>
#include "utils/filesystem/FileTools.h"
#include "utils/library/DllManager.h"
#include "utils/library/DllObject.hpp"
#include "utils/logging/LogEntry.h"

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////////////////////////////
        // Utilidades para el uso de la funcion MiniDump  //
        ////////////////////////////////////////////////////

        //------------//
        // Constantes //
        //------------//

        const char *MiniDumpTools::MINI_DUMP_DLL_NAME = "dbghelp.dll";

        const char *MiniDumpTools::MINI_DUMP_FUNC_MINIDUMPWRITEDUMP = "MiniDumpWriteDump";

        const char *MiniDumpTools::MINI_DUMP_DUMP_FILE_NAME = "crashdump.dmp";

        //--------------------//
        // Funciones de clase //
        //--------------------//

        bool MiniDumpTools::createDumpFile(const RequiredExceptionInfo &dumpInfo, const SharedLogger &logger)
        {
            // Verificamos los datos obtenidos
            if (!dumpInfo.isValid())
            {
                LOGGER_LOG_ERROR(logger) << "Informacion para mini dump incompleta";
                return false;
            }
    
            // Cargamos la DLL
            SharedDllObject dllWrapper = Library::DllManager::getInstance(MINI_DUMP_DLL_NAME, logger);
            if (!dllWrapper || !dllWrapper->isValid())
            {
                LOGGER_LOG_ERROR(logger) << "ERROR cargando libreria " << MINI_DUMP_DLL_NAME;
                return false;
            }

            // Bloqueamos el uso de la DLL
            std::lock_guard<std::mutex> dllUsageLock(dllWrapper->getUsageMutex());
    
            // Obtenemos la funcion de interes
            SharedDllFunction funcWrapper;
            MiniDumpWriteDump funcAddress = dllWrapper->getCastedFunction<MiniDumpWriteDump>(MINI_DUMP_FUNC_MINIDUMPWRITEDUMP, funcWrapper);
            if (!funcAddress || !funcWrapper)
            {
                LOGGER_LOG_ERROR(logger) << "ERROR obteniendo funcion " << MINI_DUMP_FUNC_MINIDUMPWRITEDUMP;
                return false;
            }
            
            // Creamos el fichero
            std::string fileName = getDumpFileName();
            if (fileName.empty())
            {
                LOGGER_LOG_ERROR(logger) << "ERROR obteniendo nombre de fichero: " << GetLastError();
                return false;
            }
    
            HANDLE handleFichero = CreateFile(fileName.c_str()
                                            , GENERIC_WRITE
                                            , 0
                                            , nullptr
                                            , CREATE_ALWAYS
                                            , 0
                                            , nullptr);
            if (!handleFichero)
            {
                LOGGER_LOG_ERROR(logger) << "ERROR creando fichero " << fileName << ": " << GetLastError();
                return false;
            }
    
            // Preparamos los datos de la llamada
            MINIDUMP_EXCEPTION_INFORMATION miniDumpInfo;
            miniDumpInfo.ThreadId          = dumpInfo.threadId;
            miniDumpInfo.ExceptionPointers = dumpInfo.exception;
            miniDumpInfo.ClientPointers    = FALSE;
    
            // Realizamos la llamada para generar el mini dump (scoped para limitar el alcance del lock)
            bool resultado = true;
            {
                std::lock_guard<std::mutex> lock(funcWrapper->getMutex());
                if (!funcAddress(dumpInfo.process, dumpInfo.processId, handleFichero, MiniDumpNormal, &miniDumpInfo, nullptr, nullptr))
                {
                    LOGGER_LOG_ERROR(logger) << "ERROR generando dump: " << GetLastError();
                    resultado = false;
                }
            }
    
            // Cerramos el fichero
            CloseHandle(handleFichero);
            return resultado;
        }

        std::string MiniDumpTools::getDumpFileName()
        {
            // Obtenemos la ruta del directorio
            std::string dumpDir = FileSystem::FileTools::getDirAbsolutePath(FileSystem::FileTools::OUTPUT_PATH, true);
            if (dumpDir.empty()) return std::string();
            
            // Obtenemos la fecha
            SYSTEMTIME stNow;
            GetLocalTime(&stNow);
    
            // Montamos la ruta completa
            std::stringstream ssFileName;
            if (!dumpDir.empty()) ssFileName << dumpDir;
            ssFileName << std::setw(4) << std::setfill('0') << stNow.wYear;
            ssFileName << std::setw(2) << std::setfill('0') << stNow.wMonth;
            ssFileName << std::setw(2) << std::setfill('0') << stNow.wDay;
            ssFileName << "_";
            ssFileName << std::setw(2) << std::setfill('0') << stNow.wHour;
            ssFileName << std::setw(2) << std::setfill('0') << stNow.wMinute;
            ssFileName << std::setw(2) << std::setfill('0') << stNow.wSecond;
            ssFileName << std::setw(3) << std::setfill('0') << stNow.wMilliseconds;
            ssFileName << "_" << MINI_DUMP_DUMP_FILE_NAME;
            
            return ssFileName.str();
        }
    };
};
