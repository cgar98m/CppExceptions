#include "parser/NamedArgumentParser.h"

#include <windows.h>

#include <algorithm>
#include <stdexcept>

namespace Parser
{
    //////////////////////////
    // NamedArgumentParser  //
    //////////////////////////

    NamedArgumentParser::NamedArgumentParser(const ArgumentHeaders& valid_args)
        : validArguments(valid_args)
    {
    }
    
    void NamedArgumentParser::feed(int total_args, char **args)
    {
        // Limpiamos casos previos
        parsedArguments.clear();

        // Hay argumentos por analizar?
        if (!total_args || !args) return;

        // Analizamos los argumentos
        int l_idx = -1;
        while (++l_idx < total_args)
        {
            if (!args[l_idx]) continue;

            // Coincide con alguna cabecera?
            std::string l_sArg(args[l_idx]);
            auto l_it = std::find_if(validArguments.begin(), validArguments.end(),
                [l_sArg](const auto& validArg) -> bool
                { return l_sArg.compare(validArg.argName) == 0; });
            if (l_it == validArguments.end()) continue;

            // Es un argumento unico?
            if (l_it->argType == SOLO_ARGUMENT)
            {
                ArgumentValue l_argValue = std::make_shared<SoloArgumentValue>();
                parsedArguments[*l_it]   = l_argValue;
                continue;
            }

            // Existe el valor de argumento?
            if (++l_idx >= total_args) continue;
            if (!args[l_idx]) continue;

            // Analizamos el valor
            ArgumentValue l_argValue;
            std::string   l_sValue(args[l_idx]);
            switch (l_it->argType)
            {
                case INTEGER_ARGUMENT:
                    try
                    {
                        int l_iValue = std::stoi(l_sValue);
                        l_argValue   = std::make_shared<IntArgumentValue>(l_iValue);
                    }
                    catch (const std::invalid_argument &e)
                    {
                    }
                    catch (const std::out_of_range &e)
                    {
                    }
                    break;
                
                case STRING_ARGUMENT:
                    l_argValue = std::make_shared<StringArgumentValue>(l_sValue);
                    break;
                
                default:
                    break;
            }

            // Se trata de un argumento valido?
            if (!l_argValue)
            {
                --l_idx;
                continue;
            }

            // Agregamos el argumento
            parsedArguments[*l_it] = l_argValue;
        }
    }

    NamedArgumentParser::ArgumentValue NamedArgumentParser::getValue(const ArgumentHeader& arg_header)
    {
        ArgumentValue value;

        auto l_itArg = std::find_if(parsedArguments.begin(), parsedArguments.end(),
            [arg_header](const ArgumentPair& arg)
            { return arg.first == arg_header; } );
        if (l_itArg == parsedArguments.end()) return value;

        return l_itArg->second;
    }
    
    bool operator<(const NamedArgumentParser::ArgumentHeader& lhs, const NamedArgumentParser::ArgumentHeader& rhs)
    {
        return lhs.argType < rhs.argType ||
               lhs.argType == rhs.argType && lhs.argName.compare(rhs.argName) <= 0;
    }
    
    bool operator==(const NamedArgumentParser::ArgumentHeader& lhs, const NamedArgumentParser::ArgumentHeader& rhs)
    {
        return lhs.argType == rhs.argType && lhs.argName.compare(rhs.argName) == 0;
    }
};
