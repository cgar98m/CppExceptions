#pragma once

#include "utils/parser/argument/ArgumentTypeHolder.h"

namespace Utils
{
    namespace Parser
    {
        ////////////////////////////
        // Valor de un argumento  //
        ////////////////////////////

        template <typename T>
        class ArgumentValue: public ArgumentTypeHolder
        {
            //------------------------//
            // Constructor/Destructor //
            //------------------------//

            public:
                virtual ~ArgumentValue() = default;

            protected:
                explicit ArgumentValue(ArgumentType argType, const T &argValue = T{})
                    : ArgumentTypeHolder(argType)
                    , argValue(argValue)
                {
                }
            
            //----------//
            // Deleted  //
            //----------//

            public:
                ArgumentValue() = delete;

            //----------//
            // Virtual  //
            //----------//

            public:
                virtual T getValue() const
                {
                    return this->argValue;
                }

                virtual void setValue(const T &argValue)
                {
                    this->argValue = argValue;
                }
            
            //--------------------//
            // Variables miembro  //
            //--------------------//

            private:
                T argValue = {};
        };
    };
};
