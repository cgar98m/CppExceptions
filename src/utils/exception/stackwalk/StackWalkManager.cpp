#include "utils/exception/stackwalk/StackWalkManager.h"

#include <cstring>
#include <sstream>
#include "utils/exception/stackwalk/ExtendedSymbolInfo.h"
#include "utils/filesystem/FileTools.h"
#include "utils/library/DllManager.h"
#include "utils/logging/LogEntry.h"
#include "utils/logging/LogLevel.h"
#include "utils/text/TextTools.h"

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////////////////
        // Herramienta para la obtencion del arbol de llamadas  //
        //////////////////////////////////////////////////////////

        ////////////////
        // Constantes //
        ////////////////

#if defined(IMAGE_FILE_MACHINE_I386)
        const DWORD StackWalkManager::MACHINE_TYPE = IMAGE_FILE_MACHINE_I386;
#elif defined(IMAGE_FILE_MACHINE_IA64)
        const DWORD StackWalkManager::MACHINE_TYPE = IMAGE_FILE_MACHINE_IA64;
#elif defined(IMAGE_FILE_MACHINE_AMD64)
        const DWORD StackWalkManager::MACHINE_TYPE = IMAGE_FILE_MACHINE_AMD64;
#else
        const DWORD StackWalkManager::MACHINE_TYPE = IMAGE_FILE_MACHINE_UNKNOWN;
#endif

        const char *StackWalkManager::STACK_WALK_DLL_NAME = "dbghelp.dll";

        const char *StackWalkManager::STACK_WALK_FUNC_SYMINITIALIZE            = "SymInitialize";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMCLEANUP               = "SymCleanup";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMGETOPTIONS            = "SymGetOptions";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMSETOPTIONS            = "SymSetOptions";
        const char *StackWalkManager::STACK_WALK_FUNC_STACKWALK64              = "StackWalk64";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMFUNCTIONTABLEACCESS64 = "SymFunctionTableAccess64";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMGETMODULEBASE64       = "SymGetModuleBase64";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMFROMADDR              = "SymFromAddr";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMGETLINEFROMADDR64     = "SymGetLineFromAddr64";
        const char *StackWalkManager::STACK_WALK_FUNC_SYMGETMODULEINFO64       = "SymGetModuleInfo64";

        const int StackWalkManager::STACK_WALK_DEPTH = 1024;

        const char *StackWalkManager::STACK_WALK_MODULE_NOT_FOUND = "(modulo-no-encontrado)";
        const char *StackWalkManager::STACK_WALK_FUNC_NOT_FOUND   = "(funcion-no-encontrada)";
        const char *StackWalkManager::STACK_WALK_FILE_NOT_FOUND   = "(fichero-no-encontrado)";

        ////////////////////////////
        // Constructor/Destructor //
        ////////////////////////////

        StackWalkManager::StackWalkManager(const SharedLogger &logger)
            : Logging::LoggerHolder(logger)
        {
        }

        StackWalkManager::~StackWalkManager()
        {
            std::lock_guard<std::mutex> lock(stackWalkMux);
            this->cleanEnvironment();
        }

        ////////////////////////
        // Funciones miembro  //
        ////////////////////////

        void StackWalkManager::showStackWalk(const RequiredExceptionInfo &exceptionInfo)
        {
            // Verificamos si tenemos datos validos
            if (!exceptionInfo.isFullyValid())
            {
                LOGGER_THIS_LOG_WARNING() << "Excepcion con datos NO validos";
                return;
            }

            // Verificamos la arquitectura
            if (MACHINE_TYPE == IMAGE_FILE_MACHINE_UNKNOWN)
            {
                LOGGER_THIS_LOG_WARNING() << "Arquitectura desconocida";
                return;
            }

            // Preparamos el entorno
            std::lock_guard<std::mutex> lock(stackWalkMux);
            if (!this->prepareEnvironment(exceptionInfo.process))
            {
                LOGGER_THIS_LOG_WARNING() << "ERROR preparando analisis";
                return;
            }
            
            // Obtenemos una copia del contexto
            CONTEXT context;
            std::memcpy(&context, exceptionInfo.exception->ContextRecord, sizeof(context));

            // Preparamos el strack frame
            STACKFRAME64 stackFrame;
            if (!this->prepareStackFrame(stackFrame, context)) return;

            // Bloqueamos el uso de la DLL
            std::lock_guard<std::mutex> dllUsageLock(this->dllWrapper->getUsageMutex());

            // Navegamos por los stack frame
            for (int curStackDepth = 0; curStackDepth < STACK_WALK_DEPTH; ++curStackDepth)
            {
                // Obtenemos el siguiente stack frame
                {
                    std::lock_guard<std::mutex> lockStackWalk(this->stackWalkWrapper->getMutex());
                    if (!this->stackWalkFunc(MACHINE_TYPE
                                           , exceptionInfo.process
                                           , exceptionInfo.thread
                                           , &stackFrame
                                           , &context
                                           , nullptr
                                           , funcTableAccessFunc
                                           , getModuleBaseFunc
                                           , nullptr))
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR ejecutando " << STACK_WALK_FUNC_STACKWALK64;
                        break;
                    }
                }

                // Verificamos si la direccion del PC es valida
                if (stackFrame.AddrPC.Offset)
                {
                    StackFrameEntry stackEntry;

                    {
                        // Obtenemos la informacion del simbolo
                        DWORD64            symOffset = 0;
                        ExtendedSymbolInfo symData   = {};
                        symData.SizeOfStruct = sizeof(SYMBOL_INFO);
                        symData.MaxNameLen   = sizeof(symData.extendedName);

                        std::lock_guard<std::mutex> lockSymFromAddr(this->symFromAddrWrapper->getMutex());
                        if (this->symFromAddrFunc(exceptionInfo.process
                                                , stackFrame.AddrPC.Offset
                                                , &symOffset
                                                , &symData))
                        {
                            stackEntry.funcName = Text::TextTools::getUndecoratedFunctionName(symData.Name);
                        }
                        else
                        {
                            LOGGER_THIS_LOG_DEBUG() << "ERROR ejecutando " << STACK_WALK_FUNC_SYMFROMADDR << ": " << GetLastError();
                        }
                    }

                    {
                        // Obtenemos la informacion del fichero y linea
                        DWORD           lineOffset = 0;
                        IMAGEHLP_LINE64 lineData   = {};
                        lineData.SizeOfStruct      = sizeof(lineData);
    
                        std::lock_guard<std::mutex> lockSymGetLineFromAddr(this->symGetLineFromAddrWrapper->getMutex());
                        if (this->symGetLineFromAddrFunc(exceptionInfo.process
                                                       , stackFrame.AddrPC.Offset
                                                       , &lineOffset
                                                       , &lineData))
                        {
                            stackEntry.line     = lineData.LineNumber;
                            stackEntry.fileName = FileSystem::FileTools::getUndecoratedFileName(lineData.FileName);
                        }
                        else
                        {
                            LOGGER_THIS_LOG_DEBUG() << "ERROR ejecutando " << STACK_WALK_FUNC_SYMGETLINEFROMADDR64 << ": " << GetLastError();
                        }
                    }

                    {
                        // Obtenemos la informacion del modulo
                        IMAGEHLP_MODULE64 moduleData = {};
                        moduleData.SizeOfStruct = sizeof(moduleData);

                        std::lock_guard<std::mutex> lockSymGetModuleInfo(this->symGetModuleInfoWrapper->getMutex());
                        if (this->symGetModuleInfoFunc(exceptionInfo.process
                                                     , stackFrame.AddrPC.Offset
                                                     , &moduleData))
                        {
                            stackEntry.moduleName = moduleData.ModuleName;
                        }
                        else
                        {
                            LOGGER_THIS_LOG_DEBUG() << "ERROR ejecutando " << STACK_WALK_FUNC_SYMGETMODULEINFO64 << ": " << GetLastError();
                        }
                    }

                    // Mostramos los datos de la entrada
                    showStackEntry(stackEntry);
                }

                // Verificamos si es la ultima llamada
                if (!stackFrame.AddrReturn.Offset) break;

                // Verificamos si estamos por superar la profundidad de busqueda maxima
                if (curStackDepth >= STACK_WALK_DEPTH - 1)
                {
                    LOGGER_THIS_LOG_WARNING() << "Recorrido por arbol de llamdas detenido por superar la profundidad maxima";
                    break;
                }
            }
        }

        bool StackWalkManager::prepareEnvironment(HANDLE processHandle)
        {
            // Verificamos si estamos listos
            if (this->readyToUse && processHandle == this->analyzedProcess) return true;
            
            // Realizamos limpieza preventiva
            cleanEnvironment();

            // Cargamos las funciones de interes
            if (!this->prepareDllFunctions())
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR cargando funciones";
                return false;
            }

            // Inicializa los simbolos
            std::lock_guard<std::mutex> lockSymInit(this->symInitWrapper->getMutex());
            if (!this->symInitFunc(processHandle, nullptr, TRUE))
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR llamando " << STACK_WALK_FUNC_SYMINITIALIZE << ": " << GetLastError();
                return false;
            }

            // Bloqueamos el uso de la DLL
            std::lock_guard<std::mutex> dllUsageLock(this->dllWrapper->getUsageMutex());

            // Modificamos las opciones de simbolo
            std::lock_guard<std::mutex> lockSymGetOptions(this->symGetOptWrapper->getMutex());
            DWORD symOptions = this->symGetOptFunc();
            symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
            symOptions |= SYMOPT_LOAD_LINES;
            // symOptions |= SYMOPT_NO_PROMPTS;
            // symOptions |= SYMOPT_UNDNAME;
            // symOptions |= SYMOPT_DEFERRED_LOADS;

            std::lock_guard<std::mutex> lockSymSetOptions(this->symSetOptWrapper->getMutex());
            this->symSetOptFunc(symOptions);

            // Guardamos informacion de interes
            this->analyzedProcess = processHandle;
            this->readyToUse      = true;
            return true;
        }

        bool StackWalkManager::prepareDllFunctions()
        {
            // Obtenemos la DLL
            if (!this->dllWrapper)
            {
                this->dllWrapper = Library::DllManager::getInstance(STACK_WALK_DLL_NAME, THIS_LOGGER());
                if (!this->dllWrapper || !this->dllWrapper->isValid())
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR cargando libreria " << STACK_WALK_DLL_NAME;
                    return false;
                }
            }
            
            // Cargamos la funcion SymInitialize
            if (!this->symInitWrapper)
            {
                this->symInitWrapper = nullptr;
                this->symInitFunc    = this->dllWrapper->getCastedFunction<SymInitialize>(STACK_WALK_FUNC_SYMINITIALIZE, this->symInitWrapper);
                if (!this->symInitFunc || !this->symInitWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMINITIALIZE;
                    return false;
                }
            }
            
            // Cargamos la funcion SymCleanup
            if (!this->symCleanupWrapper)
            {
                this->symCleanupWrapper = nullptr;
                this->symCleanupFunc    = this->dllWrapper->getCastedFunction<SymCleanup>(STACK_WALK_FUNC_SYMCLEANUP, this->symCleanupWrapper);
                if (!this->symCleanupFunc || !this->symCleanupWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMCLEANUP;
                    return false;
                }
            }

            // Cargamos la funcion SymGetOptions
            if (!this->symGetOptWrapper)
            {
                this->symGetOptWrapper = nullptr;
                this->symGetOptFunc    = this->dllWrapper->getCastedFunction<SymGetOptions>(STACK_WALK_FUNC_SYMGETOPTIONS, this->symGetOptWrapper);
                if (!this->symGetOptFunc || !this->symGetOptWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMGETOPTIONS;
                    return false;
                }
            }

            // Cargamos la funcion SymSetOptions
            if (!this->symSetOptWrapper)
            {
                this->symSetOptWrapper = nullptr;
                this->symSetOptFunc    = this->dllWrapper->getCastedFunction<SymSetOptions>(STACK_WALK_FUNC_SYMSETOPTIONS, this->symSetOptWrapper);
                if (!this->symSetOptFunc || !this->symSetOptWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMSETOPTIONS;
                    return false;
                }
            }

            // Cargamos la funcion StackWalk64
            if (!this->stackWalkWrapper)
            {
                this->stackWalkWrapper = nullptr;
                this->stackWalkFunc    = this->dllWrapper->getCastedFunction<StackWalk64>(STACK_WALK_FUNC_STACKWALK64, this->stackWalkWrapper);
                if (!this->stackWalkFunc || !this->stackWalkWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_STACKWALK64;
                    return false;
                }
            }

            // Cargamos la funcion SymFunctionTableAccess64
            if (!this->funcTableAccessWrapper)
            {
                this->funcTableAccessWrapper = nullptr;
                this->funcTableAccessFunc    = this->dllWrapper->getCastedFunction<SymFunctionTableAccess64>(STACK_WALK_FUNC_SYMFUNCTIONTABLEACCESS64, this->funcTableAccessWrapper);
                if (!this->funcTableAccessFunc || !this->funcTableAccessWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMFUNCTIONTABLEACCESS64;
                    return false;
                }
            }

            // Cargamos la funcion SymGetModuleBase64
            if (!this->getModuleBaseWrapper)
            {
                this->getModuleBaseWrapper = nullptr;
                this->getModuleBaseFunc    = this->dllWrapper->getCastedFunction<SymGetModuleBase64>(STACK_WALK_FUNC_SYMGETMODULEBASE64, this->getModuleBaseWrapper);
                if (!this->getModuleBaseFunc || !this->getModuleBaseWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMGETMODULEBASE64;
                    return false;
                }
            }

            // Cargamos la funcion SymFromAddr
            if (!this->symFromAddrWrapper)
            {
                this->symFromAddrWrapper = nullptr;
                this->symFromAddrFunc    = this->dllWrapper->getCastedFunction<SymFromAddr>(STACK_WALK_FUNC_SYMFROMADDR, this->symFromAddrWrapper);
                if (!this->symFromAddrFunc || !this->symFromAddrWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMFROMADDR;
                    return false;
                }
            }

            // Cargamos la funcion SymGetLineFromAddr64
            if (!this->symGetLineFromAddrWrapper)
            {
                this->symGetLineFromAddrWrapper = nullptr;
                this->symGetLineFromAddrFunc    = this->dllWrapper->getCastedFunction<SymGetLineFromAddr64>(STACK_WALK_FUNC_SYMGETLINEFROMADDR64, this->symGetLineFromAddrWrapper);
                if (!this->symGetLineFromAddrFunc || !this->symGetLineFromAddrWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMGETLINEFROMADDR64;
                    return false;
                }
            }

            // Cargamos la funcion SymGetModuleInfo64
            if (!this->symGetModuleInfoWrapper)
            {
                this->symGetModuleInfoWrapper = nullptr;
                this->symGetModuleInfoFunc    = this->dllWrapper->getCastedFunction<SymGetModuleInfo64>(STACK_WALK_FUNC_SYMGETMODULEINFO64, this->symGetModuleInfoWrapper);
                if (!this->symGetModuleInfoFunc || !this->symGetModuleInfoWrapper)
                {
                    LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo funcion " << STACK_WALK_FUNC_SYMGETMODULEINFO64;
                    return false;
                }
            }

            return true;
        }

        void StackWalkManager::cleanEnvironment()
        {
            if (this->readyToUse)
            {
                // Bloqueamos el uso de la DLL
                std::lock_guard<std::mutex> dllUsageLock(this->dllWrapper->getUsageMutex());
                
                // Limpiamos los simbolos
                std::lock_guard<std::mutex> lock(this->symCleanupWrapper->getMutex());
                if (!this->symCleanupFunc(this->analyzedProcess))
                    LOGGER_THIS_LOG_WARNING() << "ERROR llamando " << STACK_WALK_FUNC_SYMCLEANUP << ": " << GetLastError();

                this->analyzedProcess = nullptr;
                this->readyToUse      = false;
            }
        }

        bool StackWalkManager::prepareStackFrame(STACKFRAME64 &stackFrame, const CONTEXT &context)
        {
            // Inicializamos el stack frame
            ZeroMemory(&stackFrame, sizeof(stackFrame));

            // Verificamos la arquitectura
            if (MACHINE_TYPE == IMAGE_FILE_MACHINE_UNKNOWN) return false;

            // Actualizamos los datos del stack frame
#if defined(IMAGE_FILE_MACHINE_I386)
            stackFrame.AddrPC.Offset    = context.Eip;
            stackFrame.AddrPC.Mode      = AddrModeFlat;
            stackFrame.AddrStack.Offset = context.Esp;
            stackFrame.AddrStack.Mode   = AddrModeFlat;
            stackFrame.AddrFrame.Offset = context.Ebp;
            stackFrame.AddrFrame.Mode   = AddrModeFlat;
#elif defined(IMAGE_FILE_MACHINE_IA64)
            stackFrame.AddrPC.Offset     = context.StIIP;
            stackFrame.AddrPC.Mode       = AddrModeFlat;
            stackFrame.AddrStack.Offset  = context.IntSp;
            stackFrame.AddrStack.Mode    = AddrModeFlat;
            stackFrame.AddrFrame.Offset  = context.RsBSP;
            stackFrame.AddrFrame.Mode    = AddrModeFlat;
            stackFrame.AddrBStore.Offset = context.RsBSP;
            stackFrame.AddrBStore.Mode   = AddrModeFlat;
#elif defined(IMAGE_FILE_MACHINE_AMD64)
            stackFrame.AddrPC.Offset    = context.Rip;
            stackFrame.AddrPC.Mode      = AddrModeFlat;
            stackFrame.AddrStack.Offset = context.Rsp;
            stackFrame.AddrStack.Mode   = AddrModeFlat;
            stackFrame.AddrFrame.Offset = context.Rbp;
            stackFrame.AddrFrame.Mode   = AddrModeFlat;
#endif

            return true;
        }

        void StackWalkManager::showStackEntry(const StackFrameEntry &stackEntry)
        {
            std::stringstream ssStackEntry;

            // Verificamos el nombre del modulo
            if (stackEntry.moduleName.empty())
                ssStackEntry << STACK_WALK_MODULE_NOT_FOUND;
            else
                ssStackEntry << stackEntry.moduleName;
            ssStackEntry << ": ";

            // Verificamos el nombre del fichero
            if (stackEntry.fileName.empty())
                ssStackEntry << STACK_WALK_FUNC_NOT_FOUND;
            else
                ssStackEntry << stackEntry.fileName << ":" << stackEntry.line;
            ssStackEntry << ": ";

            // Verificamos el nombre de la funcion
            if (stackEntry.funcName.empty())
                ssStackEntry << STACK_WALK_FUNC_NOT_FOUND;
            else
                ssStackEntry << stackEntry.funcName;

            // Trazamos la informacion obtenida
            LOGGER_THIS_LOG_INFO() << ssStackEntry.str();
        }

        //--------------------//
        // Variables de clase //
        //--------------------//

        std::mutex StackWalkManager::stackWalkMux;
    };
};
