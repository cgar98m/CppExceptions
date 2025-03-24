#include "main/MainProgram.h"

#include <windows.h>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include "Config.h"
#include "Version.h"
#include "main/exception/CppExceptionThread.h"
#include "main/exception/SehExceptionThread.h"
#include "utils/exception/ExceptionManager.h"
#include "utils/thread/ThreadHolder.h"
#include "utils/parser/argument/IntArgumentValue.h"
#include "utils/parser/argument/StringArgumentValue.h"

namespace Main
{
    ////////////////////////
    // Programa principal //
    ////////////////////////

    const char *MainProgram::ARG_ERROR_MODE     = "ErrorTest";
    const char *MainProgram::ARG_IDENTIFIER     = "Identifier";
    const char *MainProgram::DEFAULT_IDENTIFIER = "MainProgram";

    const MainProgram::ArgList MainProgram::ARG_LIST =
    {
        { Utils::Parser::ArgumentTag(Utils::Parser::ArgumentType::INTEGER_ARGUMENT, MainProgram::ARG_ERROR_MODE), false },
        { Utils::Parser::ArgumentTag(Utils::Parser::ArgumentType::STRING_ARGUMENT,  MainProgram::ARG_IDENTIFIER), false }
    };

    MainProgram::MainProgram(const SharedLogger& logger)
        : Utils::Logging::LoggerHolder(logger)
        , workMode(WorkMode::UNDEFINED)
    {
    }

    Utils::ExitCode MainProgram::run(int argc, char **argv)
    {
        // Limpiamos el objeto
        clear();

        // Mostramos la versión
        notifyVersion();

        // Procesamos los argumentos
        parseArguments(argc, argv);

        // Ejecutamos el programa
        return work();
    }

    // Previa a la ejecucion del programa
    void MainProgram::clear()
    {
        workMode = WorkMode::UNDEFINED;
    }

    void MainProgram::notifyVersion()
    {
        // Mostramos la versión
        LOGGER_THIS_LOG_INFO() << "VERSION: "
                          << std::setfill('0') << std::setw(2) << VERSION_MAJOR << "."
                          << std::setfill('0') << std::setw(2) << VERSION_MINOR;
    }

    void MainProgram::parseArguments(int argc, char **argv)
    {
        // Obtenemos los argumentos del programa
        ArgumentTags argHeaders;
        for (auto itArgHeader = ARG_LIST.begin(); itArgHeader != ARG_LIST.end(); ++itArgHeader)
        {
            argHeaders.push_back(itArgHeader->argument);
        }

        Utils::Parser::NamedArgumentParser argParser(argHeaders);
        argParser.feedArgs(argc, argv);

        // Analizamos los argumentos
        bool requiredError = false;
        for (auto itArg = ARG_LIST.begin(); itArg != ARG_LIST.end() && !requiredError; ++itArg)
        {
            // Identificamos el argumento
            ArgValue argValue = argParser.getArgValue(itArg->argument);
            if (!argValue)
            {
                LOGGER_THIS_LOG_INFO() << "Argumento " << itArg->argument.getName() << ": NO identificado";
                if (itArg->required) requiredError = true;
                continue;
            }
            
            // Verificamos el tipo del argumento
            if (argValue->getType() != itArg->argument.getType())
            {
                LOGGER_THIS_LOG_INFO() << "Argumento " << itArg->argument.getName() << ": Tipo INVALIDO";
                if (itArg->required) requiredError = true;
                continue;
            }

            switch (itArg->argument.getType())
            {
                case Utils::Parser::SOLO_ARGUMENT:
                    break;

                case Utils::Parser::INTEGER_ARGUMENT:
                    if (!dynamic_cast<Utils::Parser::IntArgumentValue*>(argValue.get()))
                    {
                        LOGGER_THIS_LOG_INFO() << "Argumento " << itArg->argument.getName() << ": Tipo entero NO COHERENTE";
                        if (itArg->required) requiredError = true;
                        continue;
                    }
                    break;

                case Utils::Parser::STRING_ARGUMENT:
                    if (!dynamic_cast<Utils::Parser::StringArgumentValue*>(argValue.get()))
                    {
                        LOGGER_THIS_LOG_INFO() << "Argumento " << itArg->argument.getName() << ": Tipo string NO COHERENTE";
                        if (itArg->required) requiredError = true;
                        continue;
                    }
                    break;

                default:
                    if (itArg->required) requiredError = true;
                    continue;
            }

            // Procesamos el argumento
            if (!analyzeArgument(itArg->argument.getName(), argValue) && itArg->required) requiredError = true;
        }

        // Notificamos el valor parseado
        if (requiredError) workMode = WorkMode::UNDEFINED;
        LOGGER_THIS_LOG_INFO() << "Modo de trabajo: " << static_cast<uint32_t>(workMode) << " - " << getWorkModeDescription(workMode);
    }

    bool MainProgram::analyzeArgument(std::string name, ArgValue argument)
    {
        if (!argument) return false;

        if (name == ARG_ERROR_MODE)
        {
            Utils::Parser::IntArgumentValue* intArg = dynamic_cast<Utils::Parser::IntArgumentValue*>(argument.get());
            if (intArg)
            {
                switch (intArg->getValue())
                {
                    case 0:
                        return true;
    
                    case 1:
                        workMode = WorkMode::THROW_CCP_EXCEPTION;
                        return true;
    
                    case 2:
                        workMode = WorkMode::THROW_SEH_EXCEPTION;
                        return true;
    
                    case 3:
                        workMode = WorkMode::THROW_THREADED_CPP_EXCEPTION;
                        return true;
    
                    case 4:
                        workMode = WorkMode::THROW_THREADED_SEH_EXCEPTION;
                        return true;
    
                    default:
                        LOGGER_THIS_LOG_INFO() << "Argumento " << name << ": Valor NO esperado";
                        return false;
                }
            }
        }
        else if (name == ARG_IDENTIFIER)
        {
            Utils::Parser::StringArgumentValue* stringArg = dynamic_cast<Utils::Parser::StringArgumentValue*>(argument.get());
            if (stringArg) identifier = stringArg->getValue();
            return true;
        }

        return false;
    }

    // Logica del programa
    Utils::ExitCode MainProgram::work()
    {
        // Iniciamos la ejecucion
        LOGGER_THIS_LOG_INFO() << "Inicio de la ejecucion";
        
        // Instanciamos el gestor de errores
        Utils::Exception::ExceptionManager exceptionManager(true, Utils::Exception::ExceptionManager::Params{CONFIG_EXTERNALIZE_DUMPS, THIS_LOGGER(), identifier});

        // Gestionamos el modo de trabajo
        switch (workMode)
        {
            case WorkMode::UNDEFINED:
            {
                break;
            }
            
            case WorkMode::THROW_CCP_EXCEPTION:
            {
                throw std::runtime_error("C++ Exception");
                break;
            }
            
            case WorkMode::THROW_SEH_EXCEPTION:
            {
                int *p = nullptr;
                *p = 20;
                break;
            }
            
            case WorkMode::THROW_THREADED_CPP_EXCEPTION:
            {
                Main::Exception::CppExceptionThread cppThread;
                Utils::Thread::ThreadHolder cppHolder(cppThread, Utils::Thread::ThreadHolder::Params(), THIS_LOGGER());
                cppHolder.run();
                Sleep(1000);
                cppHolder.stop();
                break;
            }
            
            case WorkMode::THROW_THREADED_SEH_EXCEPTION:
            {
                Main::Exception::SehExceptionThread sehThread;
                Utils::Thread::ThreadHolder sehHolder(sehThread, Utils::Thread::ThreadHolder::Params(), THIS_LOGGER());
                sehHolder.run();
                Sleep(1000);
                sehHolder.stop();
                break;
            }
        }
        
        // FInalizamos
        LOGGER_THIS_LOG_INFO() << "Fin de la ejecucion";

        // Dmaos tiempo a terminar de escribir los logs
        return Utils::ExitCode::EXIT_CODE_OK;
    }

    // Utils
    std::string MainProgram::getWorkModeDescription(WorkMode workMode)
    {
        switch (workMode)
        {
            case WorkMode::UNDEFINED:                    return "Sin definir";
            case WorkMode::THROW_CCP_EXCEPTION:          return "Lanzamiento de excepcion C++";
            case WorkMode::THROW_SEH_EXCEPTION:          return "Lanzamiento de excepcion SEH";
            case WorkMode::THROW_THREADED_CPP_EXCEPTION: return "Lanzamiento de excepcion C++ en un thread";
            case WorkMode::THROW_THREADED_SEH_EXCEPTION: return "Lanzamiento de excepcion SEH en un thread";
            default:                                     return "N/A";
        }
    }
};