#include "utils/text/TextTools.h"

#include <algorithm>
#include <cctype>

namespace Utils
{
    namespace Text
    {
        //////////////////////////////////////////////
        // Utilidades para la manipulacion de texto //
        //////////////////////////////////////////////

        //--------------------//
        // Funciones de clase //
        //--------------------//

        std::string TextTools::toLowerCase(const std::string &text)
        {
            if (text.empty()) return text;

            // Pasamos el texto a minusculas
            std::string lowerText(text.size(), '\0');
            std::transform(text.begin(), text.end(), lowerText.begin(), std::tolower);
            return lowerText;
        }
        
        std::string TextTools::getUndecoratedFunctionName(const std::string &funcName)
        {
            // Verificamos si tiene contenido
            std::string undecoratedName = funcName;
            if (undecoratedName.empty()) return undecoratedName;
            
            // Buscamos el ultimo separador para quedarnos con el nombre de la funcion
            size_t pos = undecoratedName.find_last_of("::");
            if (pos != std::string::npos && pos < undecoratedName.size() - 1) undecoratedName = undecoratedName.substr(pos + 1);
            return undecoratedName;
        }
    };
};
