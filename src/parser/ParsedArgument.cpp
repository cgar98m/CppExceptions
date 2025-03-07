#include "parser/ParsedArgument.h"

namespace Parser
{
    ////////////////////
    // IArgumentValue //
    ////////////////////

    IArgumentValue::IArgumentValue(ArgumentType argType)
        : argType(argType)
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

    IntArgumentValue::IntArgumentValue(int intValue)
        : IArgumentValue(INTEGER_ARGUMENT)
        , intValue(intValue)
    {
    }

    int IntArgumentValue::value(void) const
    {
        return intValue;
    }

    void IntArgumentValue::value(int newIntValue)
    {
        intValue = newIntValue;
    }

    //////////////////////////
    // StringArgumentValue  //
    //////////////////////////

    StringArgumentValue::StringArgumentValue()
        : IArgumentValue(STRING_ARGUMENT)
    {
    }

    StringArgumentValue::StringArgumentValue(const std::string& stringValue)
        : IArgumentValue(STRING_ARGUMENT)
        , stringValue(stringValue)
    {
    }

    std::string StringArgumentValue::value(void) const
    {
        return stringValue;
    }

    void StringArgumentValue::value(const std::string& newStringValue)
    {
        stringValue = newStringValue;
    }
};
