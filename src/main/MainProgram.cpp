#include "main/MainProgram.h"

#include <windows.h>
#include <stdexcept>
#include "Config.h"
#include "Version.h"
#include "main/exception/CppExceptionThread.h"
#include "main/exception/SehExceptionThread.h"
#include "utils/exception/ExceptionManager.h"
#include "utils/logging/LogEntry.h"
#include "utils/parser/argument/ArgumentManager.h"
#include "utils/thread/ThreadHolder.h"

namespace Main
{
    ////////////////////////
    // Programa principal //
    ////////////////////////

    //------------//
    // Constantes //
    //------------//

    const char                        *MainProgram::WORK_MODE_ARG          = "WorkMode";
    const Utils::Parser::ArgumentType MainProgram::WORK_MODE_TYPE          = Utils::Parser::ArgumentType::INTEGER_ARGUMENT;
    const WorkMode::Mode              MainProgram::WORK_MODE_DEFAULT_VALUE = WorkMode::Mode::UNDEFINED;
    const bool                        MainProgram::WORK_MODE_REQUIRED      = false;
    
    const char                        *MainProgram::IDENTIFIER_ARG           = "Identifier";
    const Utils::Parser::ArgumentType MainProgram::IDENTIFIER_MODE_TYPE      = Utils::Parser::ArgumentType::STRING_ARGUMENT;
    const char                        *MainProgram::IDENTIFIER_DEFAULT_VALUE = "MainProgram";
    const bool                        MainProgram::IDENTIFIER_REQUIRED       = false;

    const RequiredArguments MainProgram::REQUIRED_ARGS =
    {
        { { MainProgram::WORK_MODE_TYPE,       MainProgram::WORK_MODE_ARG  }, MainProgram::WORK_MODE_REQUIRED  },
        { { MainProgram::IDENTIFIER_MODE_TYPE, MainProgram::IDENTIFIER_ARG }, MainProgram::IDENTIFIER_REQUIRED }
    };

    //------------------------//
    // Constructor/Destructor //
    //------------------------//

    MainProgram::MainProgram(const SharedLogger &logger)
        : Utils::Logging::LoggerHolder(logger)
    {
    }
    
    //--------------------//
    // Funciones miembro  //
    //--------------------//

    Utils::ExitCode MainProgram::run(int totalArgs, char **args)
    {
        // Mostramos la versión
        Core::Version::notifyVersion(THIS_LOGGER());

        // Procesamos los argumentos
        if (!this->analyzeArguments(totalArgs, args)) return Utils::ExitCode::EXIT_CODE_KO;

        // Ejecutamos el programa
        return this->work();
    }

    bool MainProgram::analyzeArguments(int totalArgs, char **args)
    {
        // Parseamos los argumentos
        Utils::Parser::ArgumentManager argManager(REQUIRED_ARGS);
        argManager.parseArguments(totalArgs, args);
        if (!argManager.minimumArgsAvailable())
        {
            LOGGER_THIS_LOG_ERROR() << "Parametros especificados incompletos";
            return false;
        }

        // Analizamos los argumentos parseados
        bool requiredArgsPresent = true;
        for (auto itArg = REQUIRED_ARGS.begin(); itArg != REQUIRED_ARGS.end(); ++itArg)
        {
            // Verificamos si tiene contenido
            std::string argName = itArg->argument.getName();
            if (argName.empty()) continue;

            // Gestionamos el argumento segun su tipo
            bool argError = false;
            switch (itArg->argument.getType())
            {
                case Utils::Parser::ArgumentType::SOLO_ARGUMENT:
                {
                    break;
                }
                
                case Utils::Parser::ArgumentType::INTEGER_ARGUMENT:
                {
                    int argValue = 0;

                    // Gestionamos el modo de trabajo
                    if (argName == WORK_MODE_ARG)
                    {
                        if (argManager.existsIntArgument(argName, argValue)) this->workMode = static_cast<WorkMode::Mode>(argValue);
                        else                                                 argError = true;
                    }
                    break;
                }
                
                case Utils::Parser::ArgumentType::STRING_ARGUMENT:
                {
                    std::string argValue;
                    
                    // Gestionamos el modo de trabajo
                    if (argName == IDENTIFIER_ARG)
                    {
                        if (argManager.existsStringArgument(argName, argValue)) this->identifier = argValue;
                        else                                                    argError = true;
                    }
                    break;
                }
                
                default:
                {
                    LOGGER_THIS_LOG_WARNING() << "Argumento " << argName << " de tipo desconocido";
                    break;
                }
            }

            // Gestionamos el error
            if (argError && itArg->required)
            {
                LOGGER_THIS_LOG_ERROR() << "ERROR procesando argumento " << argName;
                requiredArgsPresent = false;
            }
        }

        return requiredArgsPresent;
    }

    Utils::ExitCode MainProgram::work()
    {
        // Iniciamos la ejecucion
        LOGGER_THIS_LOG_INFO() << "Inicio de la ejecucion: Modo de trabajo: " << static_cast<DWORD>(this->workMode) << " Identificador: " << this->identifier;
        
        // Instanciamos el gestor de errores
        Utils::Exception::ExceptionManager exceptionManager(true, Utils::Exception::ExceptionManager::Params{CONFIG_EXTERNALIZE_DUMPS, THIS_LOGGER(), this->identifier});

        // Gestionamos el modo de trabajo
        switch (this->workMode)
        {
            case WorkMode::Mode::UNDEFINED:
            {
                break;
            }
            
            case WorkMode::Mode::THROW_CCP_EXCEPTION:
            {
                throw std::runtime_error("C++ Exception");
                break;
            }
            
            case WorkMode::Mode::THROW_SEH_EXCEPTION:
            {
                int *p = nullptr;
                *p = 20;
                break;
            }
            
            case WorkMode::Mode::THROW_THREADED_CPP_EXCEPTION:
            {
                Exception::CppExceptionThread cppThread;
                Utils::Thread::ThreadHolder   cppHolder(cppThread, Utils::Thread::ThreadHolder::Params(), THIS_LOGGER());
                cppHolder.run();
                Sleep(1000);
                cppHolder.stop();
                break;
            }
            
            case WorkMode::Mode::THROW_THREADED_SEH_EXCEPTION:
            {
                Exception::SehExceptionThread sehThread;
                Utils::Thread::ThreadHolder   sehHolder(sehThread, Utils::Thread::ThreadHolder::Params(), THIS_LOGGER());
                sehHolder.run();
                Sleep(1000);
                sehHolder.stop();
                break;
            }
        }
        
        // Finalizamos
        LOGGER_THIS_LOG_INFO() << "Fin de la ejecucion";
        return Utils::ExitCode::EXIT_CODE_OK;
    }
};
