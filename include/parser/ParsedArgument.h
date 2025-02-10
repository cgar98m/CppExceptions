#pragma once

#include <string>

namespace Parser
{
    enum ArgumentType
    {
        SOLO_ARGUMENT = 0,
        INTEGER_ARGUMENT,
        STRING_ARGUMENT
    };

    // Interfaz del valor de un argumento
    class IArgumentValue
    {
        public:
            explicit IArgumentValue(ArgumentType arg_type);
            virtual ~IArgumentValue() = default;

            ArgumentType type() const;
        
        private:
            ArgumentType argType;
    };

    // Valor de argumento sin valor
    class SoloArgumentValue: public IArgumentValue
    {
        public:
            SoloArgumentValue();
            ~SoloArgumentValue() = default;
    };

    // Valor de argumento numerico
    class IntArgumentValue: public IArgumentValue
    {
        public:
            IntArgumentValue();
            explicit IntArgumentValue(int int_value);
            ~IntArgumentValue() = default;
        
            int value(void) const;
            void value(int int_value);

        private:
            int intValue;
    };

    // Valor de argumento de texto
    class StringArgumentValue: public IArgumentValue
    {
        public:
            StringArgumentValue();
            explicit StringArgumentValue(const std::string& string_value);
            ~StringArgumentValue() = default;

            std::string value(void) const;
            void value(const std::string& string_value);
        
        private:
            std::string stringValue;
    };
};
