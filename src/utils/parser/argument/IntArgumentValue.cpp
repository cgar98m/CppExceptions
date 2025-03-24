#include "utils/parser/argument/IntArgumentValue.h"

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

        IntArgumentValue::IntArgumentValue(int argValue)
            : ArgumentValue<int>(ArgumentType::INTEGER_ARGUMENT, argValue)
        {
        }
    };
};
