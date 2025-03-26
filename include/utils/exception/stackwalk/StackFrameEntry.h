#pragma once

#include <windows.h>
#include <string>

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////////////////
        // Datos de interes de un stack frame //
        ////////////////////////////////////////

        struct StackFrameEntry
        {
            // Variables miembro
            public:
                std::string funcName;
                std::string fileName;
                DWORD       line = 0;
                std::string moduleName;
        };
    };
};
