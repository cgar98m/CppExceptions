#pragma once

#include <windows.h>
#include <string>

namespace Main
{
    ////////////////////////////////////////////
    // Modo de trabajo del programa principal //
    ////////////////////////////////////////////

    class WorkMode
    {
        // Tipos, estructuras y enums
        public:
            enum class Mode: DWORD
            {
                UNDEFINED = 0,
                THROW_CCP_EXCEPTION,
                THROW_SEH_EXCEPTION,
                THROW_THREADED_CPP_EXCEPTION,
                THROW_THREADED_SEH_EXCEPTION
            };
        
        // Funciones de clase
        public:
            static std::string getWorkModeDescription(Mode mode);
    };
};
