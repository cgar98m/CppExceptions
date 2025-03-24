#pragma once

#include "utils/parser/argument/ArgumentValue.hpp"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////////
        // Valor de un argumento SIN valor  //
        //////////////////////////////////////
        
        class SoloArgumentValue: public ArgumentValue<char>
        {
            // Constructor/Destructor
            public:
                SoloArgumentValue();
                virtual ~SoloArgumentValue() = default;

            // Final virtual
            public:
                virtual char getValue() const            final;
                virtual void setValue(const char &value) final;
        };
    };
};
