#pragma once

#include <string>

namespace Utils
{
    namespace Text
    {
        //////////////////////////////////////////////
        // Utilidades para la manipulacion de texto //
        //////////////////////////////////////////////
        
        class TextTools
        {
            // Funciones de clase
            public:
                static std::string toLowerCase(const std::string &text);
                
                static std::string getUndecoratedFunctionName(const std::string &funcName);
        };
    };
};
