#include "program/Program.h"

#include <windows.h>

#include <algorithm>
#include <iomanip>
#include <stdexcept>

#include "error/Exception.h"
#include "logger/ConsoleLogger.h"
#include "Version.h"

namespace Program
{
    //////////
    // Main //
    //////////

    const Main::ArgList Main::m_argList =
    {
        { Parser::INTEGER_ARGUMENT, "ErrorTest" }
    };

    Main::Main()
        : m_logger(Logger::ConsoleLogger::getInstance())
        , m_workMode(WorkMode::UNDEFINED)
        , m_previousFilter(SetUnhandledExceptionFilter(Error::ExceptionManager::manageMsvcException))
        , m_previousTerminate(std::set_terminate(Error::ExceptionManager::manageTerminate))
    {
    }

    Main::~Main()
    {
        SetUnhandledExceptionFilter(m_previousFilter);
        std::set_terminate(m_previousTerminate);
    }

    int Main::run(int argc, char **argv)
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
    void Main::clear()
    {
        m_workMode = WorkMode::UNDEFINED;
    }

    void Main::notifyVersion()
    {
        // Mostramos la versión
        LOGGER_LOG(m_logger) << "VERSION: "
                             << std::setfill('0') << std::setw(2) << VERSION_MAJOR << "."
                             << std::setfill('0') << std::setw(2) << VERSION_MINOR;
    }

    void Main::parseArguments(int argc, char **argv)
    {
        // Obtenemos los argumentos del programa
        Parser::NamedArgumentParser::ArgumentHeaders l_argHeaders;
        for (auto l_argHeader = m_argList.begin(); l_argHeader != m_argList.end(); ++l_argHeader)
        {
            l_argHeaders.push_back(l_argHeader->argument);
        }

        Parser::NamedArgumentParser l_parser(l_argHeaders);
        l_parser.feed(argc, argv);

        // Analizamos los argumentos
        bool l_bRequiredError = false;
        for (auto l_arg = m_argList.begin(); l_arg != m_argList.end() && !l_bRequiredError; ++l_arg)
        {
            // Identificamos el argumento
            ArgValue l_argValue = l_parser.getValue(l_arg->argument);
            if (!l_argValue)
            {
                LOGGER_LOG(m_logger) << "Argumento " << l_arg->argument.argName << ": NO identificado";
                if (l_arg->required) l_bRequiredError = true;
                continue;
            }
            
            // Verificamos el tipo del argumento
            if (l_argValue->type() != l_arg->argument.argType)
            {
                LOGGER_LOG(m_logger) << "Argumento " << l_arg->argument.argName << ": Tipo INVALIDO";
                if (l_arg->required) l_bRequiredError = true;
                continue;
            }

            switch (l_arg->argument.argType)
            {
                case Parser::SOLO_ARGUMENT:
                    break;

                case Parser::INTEGER_ARGUMENT:
                    if (!dynamic_cast<Parser::IntArgumentValue*>(l_argValue.get()))
                    {
                        LOGGER_LOG(m_logger) << "Argumento " << l_arg->argument.argName << ": Tipo entero NO COHERENTE";
                        if (l_arg->required) l_bRequiredError = true;
                        continue;
                    }
                    break;

                case Parser::STRING_ARGUMENT:
                    if (!dynamic_cast<Parser::StringArgumentValue*>(l_argValue.get()))
                    {
                        LOGGER_LOG(m_logger) << "Argumento " << l_arg->argument.argName << ": Tipo string NO COHERENTE";
                        if (l_arg->required) l_bRequiredError = true;
                        continue;
                    }
                    break;

                default:
                    if (l_arg->required) l_bRequiredError = true;
                    continue;
            }

            // Procesamos el argumento
            bool l_bErrorProcesado = analyzeArgument(l_arg->argument.argName, l_argValue);
            if (l_arg->required) l_bRequiredError = true;
        }

        // Notificamos el valor parseado
        if (l_bRequiredError) m_workMode = WorkMode::UNDEFINED;
        LOGGER_LOG(m_logger) << "Modo de trabajo: " << static_cast<uint32_t>(m_workMode) << " - " << getWorkModeDescription(m_workMode);
    }

    bool Main::analyzeArgument(std::string name, ArgValue argument)
    {
        if (!argument) return false;

        Parser::IntArgumentValue* l_intArg = dynamic_cast<Parser::IntArgumentValue*>(argument.get());
        if (l_intArg)
        {
            switch (l_intArg->value())
            {
                case 1:
                    m_workMode = WorkMode::THROW_CCP_EXCEPTION;
                    return true;

                case 2:
                    m_workMode = WorkMode::THROW_SEH_EXCEPTION;
                    return true;

                default:
                    LOGGER_LOG(m_logger) << "Argumento " << name << ": Valor NO esperado";
                    return false;
            }
        }

        return false;
    }

    // Logica del programa
    int Main::work()
    {
        // Modo valido?
        if (m_workMode == WorkMode::UNDEFINED) return 1;

        LOGGER_LOG(m_logger) << "Inicio de la ejecucion";

        // Forzamos el lanzamiento de una excepcion
        if (m_workMode == WorkMode::THROW_CCP_EXCEPTION)
        {
            throw std::runtime_error("C++ Exception");
        }
        else
        {
            int *p = nullptr;
            *p = 20;
        }
        
        LOGGER_LOG(m_logger) << "Fin de la ejecucion";
        return 0;
    }

    // Utils
    std::string Main::getWorkModeDescription(WorkMode workMode)
    {
        switch (workMode)
        {
            case WorkMode::UNDEFINED:           return "Sin definir";
            case WorkMode::THROW_CCP_EXCEPTION: return "Lanzamiento de excepcion C++";
            case WorkMode::THROW_SEH_EXCEPTION: return "Lanzamiento de excepcion SEH";
            default:                            return "N/A";
        }
    }
};