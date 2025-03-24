#include "utils/parser/argument/SoloArgumentValue.h"

#include "utils/parser/argument/ArgumentType.h"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////////
        // Valor de un argumento SIN valor  //
        //////////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        SoloArgumentValue::SoloArgumentValue()
            : ArgumentValue<char>(ArgumentType::SOLO_ARGUMENT, 0)
        {
        }

        //----------------//
        // Final virtual  //
        //----------------//
        
        char SoloArgumentValue::getValue() const
        {
            return 0;
        }

        void SoloArgumentValue::setValue(const char &value)
        {
        }
    };
};
