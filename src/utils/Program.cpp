#include "utils/Program.h"

#include <windows.h>
#include <algorithm>
#include <iomanip>
#include <stdexcept>
#include "error/Exception.h"
#include "logger/ConsoleLogger.h"
#include "Version.h"

namespace Utils
{
    //////////
    // Main //
    //////////

    const std::string Main::ARG_ERROR_MODE = "ErrorTest";

    const Main::ArgList Main::ARG_LIST =
    {
        { { Parser::INTEGER_ARGUMENT, Main::ARG_ERROR_MODE }, false }
    };

    Main::Main()
        : logger(Logger::ConsoleLogger::getInstance())
        , workMode(WorkMode::UNDEFINED)
    {
    }

    int Main::run(int argc, char **argv)
    {
        // Instanciamos el gestor de errores
        Error::ExceptionManager exceptionManager(true);
        
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
    void Main::clear()
    {
        workMode = WorkMode::UNDEFINED;
    }

    void Main::notifyVersion()
    {
        // Mostramos la versión
        LOGGER_LOG(logger) << "VERSION: "
                           << std::setfill('0') << std::setw(2) << VERSION_MAJOR << "."
                           << std::setfill('0') << std::setw(2) << VERSION_MINOR;
    }

    void Main::parseArguments(int argc, char **argv)
    {
        // Obtenemos los argumentos del programa
        Parser::NamedArgumentParser::ArgumentHeaders argHeaders;
        for (auto itArgHeader = ARG_LIST.begin(); itArgHeader != ARG_LIST.end(); ++itArgHeader)
        {
            argHeaders.push_back(itArgHeader->argument);
        }

        Parser::NamedArgumentParser argParser(argHeaders);
        argParser.feed(argc, argv);

        // Analizamos los argumentos
        bool requiredError = false;
        for (auto itArg = ARG_LIST.begin(); itArg != ARG_LIST.end() && !requiredError; ++itArg)
        {
            // Identificamos el argumento
            ArgValue argValue = argParser.getValue(itArg->argument);
            if (!argValue)
            {
                LOGGER_LOG(logger) << "Argumento " << itArg->argument.argName << ": NO identificado";
                if (itArg->required) requiredError = true;
                continue;
            }
            
            // Verificamos el tipo del argumento
            if (argValue->type() != itArg->argument.argType)
            {
                LOGGER_LOG(logger) << "Argumento " << itArg->argument.argName << ": Tipo INVALIDO";
                if (itArg->required) requiredError = true;
                continue;
            }

            switch (itArg->argument.argType)
            {
                case Parser::SOLO_ARGUMENT:
                    break;

                case Parser::INTEGER_ARGUMENT:
                    if (!dynamic_cast<Parser::IntArgumentValue*>(argValue.get()))
                    {
                        LOGGER_LOG(logger) << "Argumento " << itArg->argument.argName << ": Tipo entero NO COHERENTE";
                        if (itArg->required) requiredError = true;
                        continue;
                    }
                    break;

                case Parser::STRING_ARGUMENT:
                    if (!dynamic_cast<Parser::StringArgumentValue*>(argValue.get()))
                    {
                        LOGGER_LOG(logger) << "Argumento " << itArg->argument.argName << ": Tipo string NO COHERENTE";
                        if (itArg->required) requiredError = true;
                        continue;
                    }
                    break;

                default:
                    if (itArg->required) requiredError = true;
                    continue;
            }

            // Procesamos el argumento
            if (!analyzeArgument(itArg->argument.argName, argValue) && itArg->required) requiredError = true;
        }

        // Notificamos el valor parseado
        if (requiredError) workMode = WorkMode::UNDEFINED;
        LOGGER_LOG(logger) << "Modo de trabajo: " << static_cast<uint32_t>(workMode) << " - " << getWorkModeDescription(workMode);
    }

    bool Main::analyzeArgument(std::string name, ArgValue argument)
    {
        if (!argument) return false;

        if (name == ARG_ERROR_MODE)
        {
            Parser::IntArgumentValue* intArg = dynamic_cast<Parser::IntArgumentValue*>(argument.get());
            if (intArg)
            {
                switch (intArg->value())
                {
                    case 0:
                        break;
    
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
                        LOGGER_LOG(logger) << "Argumento " << name << ": Valor NO esperado";
                        return false;
                }
            }
        }

        return false;
    }

    // Logica del programa
    int Main::work()
    {
        LOGGER_LOG(logger) << "Inicio de la ejecucion";

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
                Error::CppExceptionThread cppThread;
                Utils::ThreadHolder       cppHolder(cppThread, Utils::ThreadHolder::Params());
                cppHolder.run();
                Sleep(1000);
                cppHolder.stop();
                break;
            }
            
            case WorkMode::THROW_THREADED_SEH_EXCEPTION:
            {
                Error::SehExceptionThread sehThread;
                Utils::ThreadHolder       sehHolder(sehThread, Utils::ThreadHolder::Params());
                sehHolder.run();
                Sleep(1000);
                sehHolder.stop();
                break;
            }
        }
        
        LOGGER_LOG(logger) << "Fin de la ejecucion";
        return 0;
    }

    // Utils
    std::string Main::getWorkModeDescription(WorkMode workMode)
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