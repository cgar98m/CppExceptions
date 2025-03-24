#include "utils/parser/argument/ArgumentTypeHolder.h"

namespace Utils
{
    namespace Parser
    {
        ////////////////////////////////////////////////
        // Clase que contiene el tipo de un argumento //
        ////////////////////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ArgumentTypeHolder::ArgumentTypeHolder(ArgumentType argType)
            : argType(argType)
        {
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        ArgumentType ArgumentTypeHolder::getType() const
        {
            return this->argType;
        }
    };
};
