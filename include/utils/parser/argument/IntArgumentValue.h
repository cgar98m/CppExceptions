#pragma once

#include "utils/parser/argument/ArgumentValue.hpp"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////
        // Valor de un argumento entero //
        //////////////////////////////////
        
        class IntArgumentValue: public ArgumentValue<int>
        {
            // Constructor/Destructor
            public:
                IntArgumentValue(int argValue = 0);
                virtual ~IntArgumentValue() = default;
        };
    };
};
