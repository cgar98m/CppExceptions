#include "utils/parser/argument/NamedArgumentParser.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include "utils/parser/argument/ArgumentType.h"
#include "utils/parser/argument/IntArgumentValue.h"
#include "utils/parser/argument/SoloArgumentValue.h"
#include "utils/parser/argument/StringArgumentValue.h"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////////
        // Parser de argumentos con nombre  //
        //////////////////////////////////////
    
        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        NamedArgumentParser::NamedArgumentParser(const ArgumentTags &validArgs)
            : validArgs(validArgs)
        {
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        void NamedArgumentParser::feedArgs(int totalArgs, char **args)
        {
            std::lock_guard<std::mutex> lock(this->parsedMutex);
    
            // Limpiamos casos previos
            this->parsedArgs.clear();
    
            // Hay argumentos por analizar?
            if (!totalArgs || !args) return;
    
            // Analizamos los argumentos
            int idx = -1;
            while (++idx < totalArgs)
            {
                if (!args[idx]) continue;
    
                // Coincide con alguna cabecera?
                std::string argName(args[idx]);
                auto itArg = std::find_if(this->validArgs.begin(), this->validArgs.end(),
                    [argName](const auto &validArg) -> bool
                    { return argName.compare(validArg.getName()) == 0; });
                if (itArg == this->validArgs.end()) continue;
    
                // Es un argumento unico?
                if (itArg->getType() == ArgumentType::SOLO_ARGUMENT)
                {
                    this->parsedArgs[*itArg] = std::make_shared<SoloArgumentValue>();
                    continue;
                }
    
                // Existe el valor de argumento?
                if (++idx >= totalArgs || !args[idx]) continue;
    
                // Analizamos el valor
                NamedArgumentValue argValue;
                std::string        stringValue(args[idx]);
                switch (itArg->getType())
                {
                    case ArgumentType::INTEGER_ARGUMENT:
                        try
                        {
                            int intValue = std::stoi(stringValue);
                            argValue     = std::make_shared<IntArgumentValue>(intValue);
                        }
                        catch (const std::invalid_argument &exception)
                        {
                        }
                        catch (const std::out_of_range &exception)
                        {
                        }
                        break;
                    
                    case ArgumentType::STRING_ARGUMENT:
                        argValue = std::make_shared<StringArgumentValue>(stringValue);
                        break;
                    
                    default:
                        break;
                }
    
                // Se trata de un argumento valido?
                if (!argValue)
                {
                    --idx;
                    continue;
                }
    
                // Agregamos el argumento
                this->parsedArgs[*itArg] = argValue;
            }
        }
    
        NamedArgumentParser::NamedArgumentValue NamedArgumentParser::getArgValue(const ArgumentTag &arg_header)
        {
            std::lock_guard<std::mutex> lock(this->parsedMutex);
    
            auto itArg = std::find_if(this->parsedArgs.begin(), this->parsedArgs.end(),
                [arg_header](const NamedArgument &arg)
                { return arg.first == arg_header; } );
            if (itArg == this->parsedArgs.end()) return NamedArgumentValue();
    
            return itArg->second;
        }
    };
};
