#pragma once

#include <map>
#include <mutex>
#include <utility>
#include "utils/parser/argument/ArgumentTag.h"
#include "utils/parser/argument/ArgumentTypeHolder.h"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////////////////
        // Parser de argumentos con nombre  //
        //////////////////////////////////////

        class NamedArgumentParser
        {
            // Tipos, estructuras y enums
            public:
                using NamedArgumentValue = SharedArgumentTypeHolder;

            private:
                using Arguments     = std::map<ArgumentTag, NamedArgumentValue>;
                using NamedArgument = std::pair<ArgumentTag, NamedArgumentValue>;

            // Constructor/Destructor
            public:
                explicit NamedArgumentParser(const ArgumentTags &validArgs);
                virtual ~NamedArgumentParser() = default;

            // Deleted
            public:
                NamedArgumentParser()                                      = delete;
                NamedArgumentParser &operator=(const NamedArgumentParser&) = delete;
    
            // Funciones miembro
            public:
                void feedArgs(int totalArgs, char **args);
                NamedArgumentValue getArgValue(const ArgumentTag &argHeader);

            // Variables miembro
            private:
                ArgumentTags validArgs;

                Arguments    parsedArgs;
                std::mutex   parsedMutex;
        };
    };
};
