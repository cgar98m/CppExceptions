#include "analysis/AnalysisProgram.h"

#include "Config.h"
#include "Name.h"
#include "Version.h"
#include "utils/exception/ExceptionManager.h"
#include "utils/exception/ipc/IpcExceptionManager.h"
#include "utils/logging/LogEntry.h"
#include "utils/parser/argument/ArgumentManager.h"

namespace Analysis
{
    ////////////////////////
    // Programa principal //
    ////////////////////////

    //------------//
    // Constantes //
    //------------//

    const char *AnalysisProgram::PROCESS_IDENTIFIER = NAME_ANALYSIS_APP;

    const char                        *AnalysisProgram::EXTERNAL_IDENTIFIER_ARG           = Utils::Exception::IpcExceptionManager::EXTERNAL_IDENTIFIER_PARAM;
    const Utils::Parser::ArgumentType AnalysisProgram::EXTERNAL_IDENTIFIER_MODE_TYPE      = Utils::Parser::ArgumentType::STRING_ARGUMENT;
    const char                        *AnalysisProgram::EXTERNAL_IDENTIFIER_DEFAULT_VALUE = "";
    const bool                        AnalysisProgram::EXTERNAL_IDENTIFIER_REQUIRED       = true;

    const RequiredArguments AnalysisProgram::REQUIRED_ARGS =
    {
        { { AnalysisProgram::EXTERNAL_IDENTIFIER_MODE_TYPE, AnalysisProgram::EXTERNAL_IDENTIFIER_ARG }, AnalysisProgram::EXTERNAL_IDENTIFIER_REQUIRED }
    };

    //------------------------//
    // Constructor/Destructor //
    //------------------------//

    AnalysisProgram::AnalysisProgram(const SharedLogger &logger)
        : Utils::Logging::LoggerHolder(logger)
    {
    }
    
    //--------------------//
    // Funciones miembro  //
    //--------------------//

    Utils::ExitCode AnalysisProgram::run(int totalArgs, char **args)
    {
        // Mostramos la versión
        Core::Version::notifyVersion(THIS_LOGGER());

        // Procesamos los argumentos y ejecutamos el programa
        return this->work(this->analyzeArguments(totalArgs, args));
    }

    bool AnalysisProgram::analyzeArguments(int totalArgs, char **args)
    {
        // Parseamos los argumentos
        Utils::Parser::ArgumentManager argManager(REQUIRED_ARGS, THIS_LOGGER());
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
                    break;
                }
                
                case Utils::Parser::ArgumentType::STRING_ARGUMENT:
                {
                    std::string argValue;
                    
                    // Gestionamos el identificador externo
                    if (argName == EXTERNAL_IDENTIFIER_ARG)
                    {
                        if (argManager.existsStringArgument(argName, argValue)) this->externalIdentifier = argValue;
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

    Utils::ExitCode AnalysisProgram::work(bool requisitesMet)
    {
        // Iniciamos la ejecucion
        LOGGER_THIS_LOG_INFO() << "Inicio de la ejecucion: Identificador externo: " << this->externalIdentifier;
        
        // Sobreescribimos el comportamiento de las excepciones
        std::string identifier = std::string(PROCESS_IDENTIFIER) + std::string("/") + this->externalIdentifier;
        Utils::Exception::ExceptionManager exceptionManager(true, Utils::Exception::ExceptionManager::Params{false, THIS_LOGGER(), identifier});
        
        // Verificamos si podemos ejecutar el programa y salida del programa
        if (!requisitesMet || !CONFIG_EXTERNALIZE_DUMPS)
        {
            LOGGER_THIS_LOG_ERROR() << "No se puede ejecutar el programa";
            return Utils::ExitCode::EXIT_CODE_NOT_AVAILABLE;
        }

        // Creamos el gestor de excepciones externas en modo escucha
        Utils::Exception::IpcExceptionManager externalExceptionManager(this->externalIdentifier, false, THIS_LOGGER());
        if (!externalExceptionManager.isValid())
        {
            LOGGER_THIS_LOG_ERROR() << "ERROR creando manejador de excepciones externas";
            return Utils::ExitCode::EXIT_CODE_KO;
        }

        // Recibimos los datos de la excepcion y la manejamos
        if (!externalExceptionManager.receiveException())
        {
            LOGGER_THIS_LOG_ERROR() << "ERROR procesando excepcion";
            return Utils::ExitCode::EXIT_CODE_KO;
        }
        
        // Finalizamos
        LOGGER_THIS_LOG_INFO() << "Fin de la ejecucion";
        return Utils::ExitCode::EXIT_CODE_OK;
    }
};
