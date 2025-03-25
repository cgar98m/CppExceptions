#pragma once

#include <string>
#include <vector>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/parser/argument/ArgumentTag.h"
#include "utils/parser/argument/NamedArgumentParser.h"
#include "utils/parser/argument/RequiredArgument.h"

namespace Utils
{
    namespace Parser
    {
        //////////////////////////
        // Gestor de argumentos //
        //////////////////////////

        class ArgumentManager: public Logging::LoggerHolder
        {
            // Tipos, estructuras y enums
            public:
                using ParsedArgument = NamedArgumentParser::NamedArgumentValue;

            // Constructor/Destructor
            public:
                explicit ArgumentManager(const RequiredArguments &requiredArgs = RequiredArguments(), const SharedLogger &logger = BASIC_LOGGER());
                virtual ~ArgumentManager() = default;

            // Deleted
            public:
                ArgumentManager(const ArgumentManager&)            = delete;
                ArgumentManager &operator=(const ArgumentManager&) = delete;

            // Funciones miembro
            public:
                void parseArguments(int totalArgs, char **args);
                bool minimumArgsAvailable();

                bool existsSoloArgument(const std::string &argName);
                bool existsIntArgument(const std::string &argName, int &argValue);
                bool existsStringArgument(const std::string &argName, std::string &argValue);
            
            // Variables miembro
            private:
                RequiredArguments requiredArgs;
                ArgumentTags      requiredArgTags;
                bool              minimumRequiredArgs = false;

                UniqueNamedArgumentParser argParser;
        };
    };
};
