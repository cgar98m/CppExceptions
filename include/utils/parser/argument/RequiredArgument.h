#pragma once

#include <vector>
#include "utils/parser/argument/ArgumentTag.h"
#include "utils/parser/argument/ArgumentType.h"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////////////////////////////
        // Argumento con informacion sobre si se requiere o no  //
        //////////////////////////////////////////////////////////
        
        struct RequiredArgument
        {
            ArgumentTag argument;
            bool        required = false;
        };
    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using RequiredArguments = std::vector<Utils::Parser::RequiredArgument>;
