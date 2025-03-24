#include "utils/parser/argument/StringArgumentValue.h"

#include "utils/parser/argument/ArgumentType.h"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////
        // Valor de un argumento entero //
        //////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        StringArgumentValue::StringArgumentValue(const std::string &argValue)
            : ArgumentValue<std::string>(ArgumentType::STRING_ARGUMENT, argValue)
        {
        }
    };
};
