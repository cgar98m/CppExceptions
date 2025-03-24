#pragma once

#include "utils/parser/argument/ArgumentTag.h"

namespace Utils
{
    namespace Parser
    {
        ////////////////////////////
        // Nombre de un argumento //
        ////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ArgumentTag::ArgumentTag(ArgumentType argType, const std::string& argName)
            : ArgumentTypeHolder(argType)
            , argName(argName)
        {
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        std::string ArgumentTag::getName() const
        {
            return this->argName;
        }

        void ArgumentTag::setName(const std::string &argName)
        {
            this->argName = argName;
        }

        ////////////////
        // Operadores //
        ////////////////
        
        bool operator<(const ArgumentTag &lhs, const ArgumentTag &rhs)
        {
            return lhs.getType() < rhs.getType() ||
                   lhs.getType() == rhs.getType() && lhs.getName().compare(rhs.getName()) < 0;
        }
        
        bool operator<=(const ArgumentTag &lhs, const ArgumentTag &rhs)
        {
            return lhs.getType() <= rhs.getType() ||
                   lhs.getType() == rhs.getType() && lhs.getName().compare(rhs.getName()) <= 0;
        }
        
        bool operator>(const ArgumentTag &lhs, const ArgumentTag &rhs)
        {
            return lhs.getType() > rhs.getType() ||
                   lhs.getType() == rhs.getType() && lhs.getName().compare(rhs.getName()) > 0;
        }
        
        bool operator>=(const ArgumentTag &lhs, const ArgumentTag &rhs)
        {
            return lhs.getType() >= rhs.getType() ||
                   lhs.getType() == rhs.getType() && lhs.getName().compare(rhs.getName()) >= 0;
        }
        
        bool operator==(const ArgumentTag &lhs, const ArgumentTag &rhs)
        {
            return lhs.getType() == rhs.getType() && lhs.getName().compare(rhs.getName()) == 0;
        }
        
        bool operator!=(const ArgumentTag &lhs, const ArgumentTag &rhs)
        {
            return lhs.getType() != rhs.getType() || lhs.getName().compare(rhs.getName()) != 0;
        }
    };
};
