#pragma once

#include <memory>
#include "utils/parser/argument/ArgumentType.h"

namespace Utils
{
    namespace Parser
    {
        ////////////////////////////////////////////////
        // Clase que contiene el tipo de un argumento //
        ////////////////////////////////////////////////

        class ArgumentTypeHolder
        {
            // Constructor/Destructor
            public:
                explicit ArgumentTypeHolder(ArgumentType argType);
                virtual ~ArgumentTypeHolder() = default;
                
            // Deleted
            public:
                ArgumentTypeHolder() = delete;

            // Funciones miembro
            public:
                ArgumentType getType() const;

            // Variables miembro
            private:
                ArgumentType argType;
        };
    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using SharedArgumentTypeHolder = std::shared_ptr<Utils::Parser::ArgumentTypeHolder>;
