#pragma once

#include <string>

namespace Parser
{
    // Tipos de argumentos
    enum ArgumentType
    {
        SOLO_ARGUMENT = 0,
        INTEGER_ARGUMENT,
        STRING_ARGUMENT
    };

    // Interfaz del valor de un argumento
    class IArgumentValue
    {
        private:
            ArgumentType argType;

        public:
            IArgumentValue() = delete;
            explicit IArgumentValue(ArgumentType argType);
            virtual ~IArgumentValue() = default;

            ArgumentType type() const;
    };

    // Valor de argumento sin valor
    class SoloArgumentValue: public IArgumentValue
    {
        public:
            SoloArgumentValue();
            virtual ~SoloArgumentValue() = default;
    };

    // Valor de argumento numerico
    class IntArgumentValue: public IArgumentValue
    {
        private:
            int intValue;

        public:
            IntArgumentValue();
            explicit IntArgumentValue(int intValue);
            virtual ~IntArgumentValue() = default;
        
            int value(void) const;
            void value(int newIntValue);
    };

    // Valor de argumento de texto
    class StringArgumentValue: public IArgumentValue
    {
        private:
            std::string stringValue;

        public:
            StringArgumentValue();
            explicit StringArgumentValue(const std::string& stringValue);
            virtual ~StringArgumentValue() = default;

            std::string value(void) const;
            void value(const std::string& newStringValue);
    };
};
