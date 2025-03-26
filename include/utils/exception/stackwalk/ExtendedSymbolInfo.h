#pragma once

#include <windows.h>
#include <dbghelp.h>

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////
        // Extension de SYMBOL_INFO para el nombre  //
        //////////////////////////////////////////////

        struct ExtendedSymbolInfo: public SYMBOL_INFO
        {
            // Variables miembro
            CHAR extendedName[MAX_PATH];
        };
    };
};
