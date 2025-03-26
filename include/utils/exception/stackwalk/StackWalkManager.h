#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <mutex>
#include "utils/exception/RequiredExceptionInfo.h"
#include "utils/exception/stackwalk/StackFrameEntry.h"
#include "utils/library/DllObject.hpp"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////////////////
        // Herramienta para la obtencion del arbol de llamadas  //
        //////////////////////////////////////////////////////////

        class StackWalkManager: public Logging::LoggerHolder
        {
            // Constantes
            private:
                static const DWORD MACHINE_TYPE;
                
                static const char *STACK_WALK_DLL_NAME;

                static const char *STACK_WALK_FUNC_SYMINITIALIZE;
                static const char *STACK_WALK_FUNC_SYMCLEANUP;
                static const char *STACK_WALK_FUNC_SYMGETOPTIONS;
                static const char *STACK_WALK_FUNC_SYMSETOPTIONS;
                static const char *STACK_WALK_FUNC_STACKWALK64;
                static const char *STACK_WALK_FUNC_SYMFUNCTIONTABLEACCESS64;
                static const char *STACK_WALK_FUNC_SYMGETMODULEBASE64;
                static const char *STACK_WALK_FUNC_SYMFROMADDR;
                static const char *STACK_WALK_FUNC_SYMGETLINEFROMADDR64;
                static const char *STACK_WALK_FUNC_SYMGETMODULEINFO64;

                static const int STACK_WALK_DEPTH;

                static const char *STACK_WALK_MODULE_NOT_FOUND;
                static const char *STACK_WALK_FUNC_NOT_FOUND;
                static const char *STACK_WALK_FILE_NOT_FOUND;

            // Tipos, estructuras y enums
            private:
                using SymInitialize = BOOL (WINAPI *)(HANDLE processHandle
                                                    , PCSTR  symbolSearchPaths
                                                    , BOOL   invadeProcess);

                using SymCleanup = BOOL (WINAPI *)(HANDLE processHandle);
                
                using SymGetOptions = DWORD (WINAPI *)();

                using SymSetOptions = DWORD (WINAPI *)(DWORD options);

                using StackWalk64 = BOOL (WINAPI *)(DWORD                            machineType
                                                  , HANDLE                           processHandle
                                                  , HANDLE                           threadHandle
                                                  , LPSTACKFRAME64                   stackFrame
                                                  , PVOID                            context
                                                  , PREAD_PROCESS_MEMORY_ROUTINE64   readMemoryRoutine
                                                  , PFUNCTION_TABLE_ACCESS_ROUTINE64 functionTableAccessRoutine
                                                  , PGET_MODULE_BASE_ROUTINE64       getModuleBaseRoutine
                                                  , PTRANSLATE_ADDRESS_ROUTINE64     translateAddressRoutine);

                using SymFunctionTableAccess64 = PVOID (WINAPI *)(HANDLE  processHandle
                                                                , DWORD64 baseAddress);

                using SymGetModuleBase64 = DWORD64 (WINAPI *)(HANDLE  processHandle
                                                            , DWORD64 address);

                using SymFromAddr = BOOL (WINAPI *)(HANDLE       processHandle
                                                  , DWORD64      symAddress
                                                  , PDWORD64     symOffset
                                                  , PSYMBOL_INFO symbol);

                using SymGetLineFromAddr64 = BOOL (WINAPI *)(HANDLE           processHandle
                                                           , DWORD64          lineAddress
                                                           , PDWORD           lineOffset
                                                           , PIMAGEHLP_LINE64 line);
                
                using SymGetModuleInfo64 = BOOL (WINAPI *)(HANDLE             processHandle
                                                         , DWORD64            moduleAddress
                                                         , PIMAGEHLP_MODULE64 module);

            // Constructor/Destructor
            public:
                StackWalkManager(const SharedLogger &logger = BASIC_LOGGER());
                virtual ~StackWalkManager();
            
            // Deleted
            public:
                StackWalkManager(const StackWalkManager&)            = delete;
                StackWalkManager &operator=(const StackWalkManager&) = delete;

            // Funciones miembro
            public:
                void showStackWalk(const RequiredExceptionInfo &exceptionInfo);

            private:
                bool prepareEnvironment(HANDLE processHandle);
                bool prepareDllFunctions();
                void cleanEnvironment();

                bool prepareStackFrame(STACKFRAME64 &stackFrame, const CONTEXT &context);

                void showStackEntry(const StackFrameEntry &stackEntry);

            // Variables de clase
            private:
                static std::mutex stackWalkMux;

            // Variables miembro
            private:
                bool   readyToUse      = false;
                HANDLE analyzedProcess = nullptr;

                SharedDllObject dllWrapper;

                SharedDllFunction symInitWrapper;
                SharedDllFunction symCleanupWrapper;
                SharedDllFunction symGetOptWrapper;
                SharedDllFunction symSetOptWrapper;
                SharedDllFunction stackWalkWrapper;
                SharedDllFunction funcTableAccessWrapper;
                SharedDllFunction getModuleBaseWrapper;
                SharedDllFunction symFromAddrWrapper;
                SharedDllFunction symGetLineFromAddrWrapper;
                SharedDllFunction symGetModuleInfoWrapper;

                SymInitialize            symInitFunc            = nullptr;
                SymCleanup               symCleanupFunc         = nullptr;
                SymGetOptions            symGetOptFunc          = nullptr;
                SymSetOptions            symSetOptFunc          = nullptr;
                StackWalk64              stackWalkFunc          = nullptr;
                SymFunctionTableAccess64 funcTableAccessFunc    = nullptr;
                SymGetModuleBase64       getModuleBaseFunc      = nullptr;
                SymFromAddr              symFromAddrFunc        = nullptr;
                SymGetLineFromAddr64     symGetLineFromAddrFunc = nullptr;
                SymGetModuleInfo64       symGetModuleInfoFunc   = nullptr;
        };
    };
};
