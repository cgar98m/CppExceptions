#include "utils/parser/NamedArgumentParser.h"

#include <algorithm>
#include <stdexcept>

namespace Parser
{
    //////////////////////////
    // NamedArgumentParser  //
    //////////////////////////

    NamedArgumentParser::NamedArgumentParser(const ArgumentHeaders &validArgs)
        : validArgs(validArgs)
    {
    }
    
    void NamedArgumentParser::feed(int totalArgs, char **args)
    {
        std::lock_guard<std::mutex> lock(argMutex);

        // Limpiamos casos previos
        parsedArgs.clear();

        // Hay argumentos por analizar?
        if (!totalArgs || !args) return;

        // Analizamos los argumentos
        int idx = -1;
        while (++idx < totalArgs)
        {
            if (!args[idx]) continue;

            // Coincide con alguna cabecera?
            std::string argName(args[idx]);
            auto itArg = std::find_if(validArgs.begin(), validArgs.end(),
                [argName](const auto &validArg) -> bool
                { return argName.compare(validArg.argName) == 0; });
            if (itArg == validArgs.end()) continue;

            // Es un argumento unico?
            if (itArg->argType == SOLO_ARGUMENT)
            {
                parsedArgs[*itArg] = std::make_shared<SoloArgumentValue>();
                continue;
            }

            // Existe el valor de argumento?
            if (++idx >= totalArgs) continue;
            if (!args[idx]) continue;

            // Analizamos el valor
            ArgumentValue argValue;
            std::string   stringValue(args[idx]);
            switch (itArg->argType)
            {
                case INTEGER_ARGUMENT:
                    try
                    {
                        int intValue = std::stoi(stringValue);
                        argValue     = std::make_shared<IntArgumentValue>(intValue);
                    }
                    catch (const std::invalid_argument &e)
                    {
                    }
                    catch (const std::out_of_range &e)
                    {
                    }
                    break;
                
                case STRING_ARGUMENT:
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
            parsedArgs[*itArg] = argValue;
        }
    }

    NamedArgumentParser::ArgumentValue NamedArgumentParser::getValue(const ArgumentHeader &arg_header)
    {
        std::lock_guard<std::mutex> lock(argMutex);

        auto itArg = std::find_if(parsedArgs.begin(), parsedArgs.end(),
            [arg_header](const ArgumentPair &arg)
            { return arg.first == arg_header; } );
        if (itArg == parsedArgs.end()) return ArgumentValue();

        return itArg->second;
    }
    
    bool operator<(const NamedArgumentParser::ArgumentHeader &lhs, const NamedArgumentParser::ArgumentHeader &rhs)
    {
        return lhs.argType < rhs.argType ||
               lhs.argType == rhs.argType && lhs.argName.compare(rhs.argName) <= 0;
    }
    
    bool operator==(const NamedArgumentParser::ArgumentHeader &lhs, const NamedArgumentParser::ArgumentHeader &rhs)
    {
        return lhs.argType == rhs.argType && lhs.argName.compare(rhs.argName) == 0;
    }
};
