#pragma once

#include <string>
#include <vector>
#include "utils/parser/argument/ArgumentType.h"
#include "utils/parser/argument/ArgumentTypeHolder.h"

namespace Utils
{
    namespace Parser
    {
        ////////////////////////////
        // Nombre de un argumento //
        ////////////////////////////

        class ArgumentTag: public ArgumentTypeHolder 
        {
            // Constructor/Destructor
            public:
                explicit ArgumentTag(ArgumentType argType, const std::string &argName = std::string());
                virtual ~ArgumentTag() = default;

            // Funciones miembro
            public:
                std::string getName() const;
                void setName(const std::string &argName);

            // Variables miembro
            private:
                std::string argName;
        };

        ////////////////
        // Operadores //
        ////////////////

        bool operator<(const ArgumentTag &lhs, const ArgumentTag &rhs);
        bool operator<=(const ArgumentTag &lhs, const ArgumentTag &rhs);
        bool operator>(const ArgumentTag &lhs, const ArgumentTag &rhs);
        bool operator>=(const ArgumentTag &lhs, const ArgumentTag &rhs);
        bool operator==(const ArgumentTag &lhs, const ArgumentTag &rhs);
        bool operator!=(const ArgumentTag &lhs, const ArgumentTag &rhs);
    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using ArgumentTags = std::vector<Utils::Parser::ArgumentTag>;
