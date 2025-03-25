#pragma once

#include <windows.h>

namespace Utils
{
    namespace Parser
    {
        //////////////////////////
        // Tipos de argumentos  //
        //////////////////////////
        
        enum ArgumentType: DWORD
        {
            UNDEFINED_ARGUMENT = 0,
            SOLO_ARGUMENT,
            INTEGER_ARGUMENT,
            STRING_ARGUMENT
        };
    };
};
