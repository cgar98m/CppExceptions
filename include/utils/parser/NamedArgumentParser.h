#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include "utils/parser/ParsedArgument.h"

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

        private:
            const ArgumentHeaders validArgs;
            Arguments             parsedArgs;
            std::mutex            argMutex;

        public:
            NamedArgumentParser() = delete;
            explicit NamedArgumentParser(const ArgumentHeaders &validArgs);
            NamedArgumentParser &operator=(const NamedArgumentParser&) = delete;
            ~NamedArgumentParser() = default;
            
            void feed(int totalArgs, char **args);
            ArgumentValue getValue(const ArgumentHeader& argHeader);
    };

    bool operator<(const NamedArgumentParser::ArgumentHeader &lhs, const NamedArgumentParser::ArgumentHeader &rhs);
    bool operator==(const NamedArgumentParser::ArgumentHeader &lhs, const NamedArgumentParser::ArgumentHeader &rhs);
};
