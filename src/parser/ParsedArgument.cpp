#include "parser/ParsedArgument.h"

namespace Parser
{
    ////////////////////
    // IArgumentValue //
    ////////////////////

    IArgumentValue::IArgumentValue(ArgumentType arg_type)
        : argType(arg_type)
    {
    }

    ArgumentType IArgumentValue::type() const
    {
        return argType;
    }

    ////////////////////////
    // SoloArgumentValue  //
    ////////////////////////

    SoloArgumentValue::SoloArgumentValue()
        : IArgumentValue(SOLO_ARGUMENT)
    {
    }

    //////////////////////
    // IntArgumentValue //
    //////////////////////

    IntArgumentValue::IntArgumentValue()
        : IArgumentValue(INTEGER_ARGUMENT)
        , intValue(0)
    {
    }

    IntArgumentValue::IntArgumentValue(int int_value)
        : IArgumentValue(INTEGER_ARGUMENT)
        , intValue(int_value)
    {
    }

    int IntArgumentValue::value(void) const
    {
        return intValue;
    }

    void IntArgumentValue::value(int int_value)
    {
        intValue = int_value;
    }

    //////////////////////////
    // StringArgumentValue  //
    //////////////////////////

    StringArgumentValue::StringArgumentValue()
        : IArgumentValue(STRING_ARGUMENT)
    {
    }

    StringArgumentValue::StringArgumentValue(const std::string& string_value)
        : IArgumentValue(STRING_ARGUMENT)
        , stringValue(string_value)
    {
    }

    std::string StringArgumentValue::value(void) const
    {
        return stringValue;
    }

    void StringArgumentValue::value(const std::string& string_value)
    {
        stringValue = string_value;
    }
};
