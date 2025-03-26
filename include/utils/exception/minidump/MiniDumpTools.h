#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <string>
#include "utils/exception/RequiredExceptionInfo.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////////////////////////////
        // Utilidades para el uso de la funcion MiniDump  //
        ////////////////////////////////////////////////////

        class MiniDumpTools
        {
            // Constantes
            public:
                static const char *MINI_DUMP_DLL_NAME;

                static const char *MINI_DUMP_FUNC_MINIDUMPWRITEDUMP;
                
                static const char *MINI_DUMP_DUMP_FILE_NAME;

            // Tipos, estructuras y enums
            private:
                using MiniDumpWriteDump = BOOL (WINAPI *)(HANDLE                            processHandle
                                                        , DWORD                             processId
                                                        , HANDLE                            fileHandle
                                                        , MINIDUMP_TYPE                     dumpType
                                                        , PMINIDUMP_EXCEPTION_INFORMATION   exceptionInfo
                                                        , PMINIDUMP_USER_STREAM_INFORMATION userStreamInfo
                                                        , PMINIDUMP_CALLBACK_INFORMATION    callbackInfo);

            // Funciones de clase
            public:
                static bool createDumpFile(const RequiredExceptionInfo &dumpInfo, const SharedLogger &logger = BASIC_LOGGER());
            
            private:
                static std::string getDumpFileName();
        };
    };
};
