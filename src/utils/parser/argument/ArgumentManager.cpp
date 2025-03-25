#include "utils/parser/argument/ArgumentManager.h"

#include "utils/logging/LogEntry.h"
#include "utils/parser/argument/ArgumentType.h"
#include "utils/parser/argument/IntArgumentValue.h"
#include "utils/parser/argument/SoloArgumentValue.h"
#include "utils/parser/argument/StringArgumentValue.h"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////
        // Gestor de argumentos //
        //////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ArgumentManager::ArgumentManager(const RequiredArguments &requiredArgs, const SharedLogger &logger)
            : Logging::LoggerHolder(logger)
            , requiredArgs(requiredArgs)
        {
            // Obtenemos los argumentos del programa
            for (auto itArg = this->requiredArgs.begin(); itArg != this->requiredArgs.end(); ++itArg)
                this->requiredArgTags.push_back(itArg->argument);
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        void ArgumentManager::parseArguments(int totalArgs, char **args)
        {
            // Obtenemos los argumentos del programa
            this->argParser.reset(new NamedArgumentParser(this->requiredArgTags));
            if (!this->argParser)
            {
                if (!this->requiredArgs.empty()) this->minimumRequiredArgs = false;
                return;
            }
            this->argParser->feedArgs(totalArgs, args);

            // Analizamos los argumentos requeridos
            this->minimumRequiredArgs = true;
            for (auto itArg = this->requiredArgs.begin(); itArg != this->requiredArgs.end(); ++itArg)
            {
                // Verificamos la validez del argumento
                std::string argName = itArg->argument.getName();
                if (argName.empty()) continue;

                // Identificamos el argumento
                NamedArgumentParser::NamedArgumentValue argValue = this->argParser->getArgValue(itArg->argument);
                if (!argValue)
                {
                    if (itArg->required)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Argumento " << itArg->argument.getName() << " NO identificado";
                        this->minimumRequiredArgs = false;
                    }
                    continue;
                }
                
                // Verificamos el tipo del argumento
                if (argValue->getType() != itArg->argument.getType())
                {
                    if (itArg->required)
                    {
                        LOGGER_THIS_LOG_ERROR() << "Argumento " << itArg->argument.getName() << " con tipo NO valido";
                        this->minimumRequiredArgs = false;
                    }
                    continue;
                }
            }
        }

        bool ArgumentManager::minimumArgsAvailable()
        {
            return this->minimumRequiredArgs;
        }

        bool ArgumentManager::existsSoloArgument(const std::string &argName)
        {
            // Verificamos si tenemos parser operativo
            if (!this->argParser) return false;

            // Obtenemos el argumento sin valor
            NamedArgumentParser::NamedArgumentValue parsedArgValue = this->argParser->getArgValue(ArgumentTag(ArgumentType::SOLO_ARGUMENT, argName));
            if (!parsedArgValue || parsedArgValue->getType() != ArgumentType::SOLO_ARGUMENT) return false;

            // Lo casteamos al tipo apropiado
            SoloArgumentValue *castedValueArg = dynamic_cast<SoloArgumentValue*>(parsedArgValue.get());
            if (!castedValueArg) return false;

            return true;
        }

        bool ArgumentManager::existsIntArgument(const std::string &argName, int &argValue)
        {
            // Verificamos si tenemos parser operativo
            if (!this->argParser) return false;

            // Obtenemos el argumento sin valor
            NamedArgumentParser::NamedArgumentValue parsedArgValue = this->argParser->getArgValue(ArgumentTag(ArgumentType::INTEGER_ARGUMENT, argName));
            if (!parsedArgValue || parsedArgValue->getType() != ArgumentType::INTEGER_ARGUMENT) return false;

            // Lo casteamos al tipo apropiado
            IntArgumentValue *castedValueArg = dynamic_cast<IntArgumentValue*>(parsedArgValue.get());
            if (!castedValueArg) return false;

            // Obtenemos su valor
            argValue = castedValueArg->getValue();
            return true;
        }

        bool ArgumentManager::existsStringArgument(const std::string &argName, std::string &argValue)
        {
            // Verificamos si tenemos parser operativo
            if (!this->argParser) return false;

            // Obtenemos el argumento sin valor
            NamedArgumentParser::NamedArgumentValue parsedArgValue = this->argParser->getArgValue(ArgumentTag(ArgumentType::STRING_ARGUMENT, argName));
            if (!parsedArgValue || parsedArgValue->getType() != ArgumentType::STRING_ARGUMENT) return false;

            // Lo casteamos al tipo apropiado
            StringArgumentValue *castedValueArg = dynamic_cast<StringArgumentValue*>(parsedArgValue.get());
            if (!castedValueArg) return false;

            // Obtenemos su valor
            argValue = castedValueArg->getValue();
            return true;
        }
    };
};
