#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "parser/ParsedArgument.h"

namespace Parser
{
    class NamedArgumentParser
    {
        public:
            struct ArgumentHeader
            {
                ArgumentType argType;
                std::string  argName;
            };

            using ArgumentHeaders = std::vector<ArgumentHeader>;
            using ArgumentValue   = std::shared_ptr<IArgumentValue>;
            using ArgumentPair    = std::pair<ArgumentHeader, ArgumentValue>;
            using Arguments       = std::map<ArgumentHeader, ArgumentValue>;

            explicit NamedArgumentParser(const ArgumentHeaders& valid_args);
            ~NamedArgumentParser() = default;
            
            void feed(int total_args, char **args);
            ArgumentValue getValue(const ArgumentHeader& arg_header);

        private:
            NamedArgumentParser& operator=(const NamedArgumentParser&) = delete;

            const ArgumentHeaders validArguments;
            Arguments parsedArguments;
    };

    bool operator<(const NamedArgumentParser::ArgumentHeader& lhs, const NamedArgumentParser::ArgumentHeader& rhs);
    bool operator==(const NamedArgumentParser::ArgumentHeader& lhs, const NamedArgumentParser::ArgumentHeader& rhs);
};
