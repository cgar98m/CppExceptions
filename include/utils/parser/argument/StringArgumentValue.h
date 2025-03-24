#pragma once

#include <string>
#include "utils/parser/argument/ArgumentValue.hpp"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////
        // Valor de un argumento entero //
        //////////////////////////////////
        
        class StringArgumentValue: public ArgumentValue<std::string>
        {
            // Constructor/Destructor
            public:
                StringArgumentValue(const std::string &argValue);
                virtual ~StringArgumentValue() = default;
        };
    };
};
