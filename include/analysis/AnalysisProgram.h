#pragma once

#include <string>
#include "utils/ExitCode.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/parser/argument/ArgumentType.h"
#include "utils/parser/argument/RequiredArgument.h"

namespace Analysis
{
    ////////////////////////////////////
    // Wrapper del programa principal //
    ////////////////////////////////////

    class AnalysisProgram: public Utils::Logging::LoggerHolder
    {
        // Constantes
        private:
            static const char *PROCESS_IDENTIFIER;

            static const char                        *EXTERNAL_IDENTIFIER_ARG;
            static const Utils::Parser::ArgumentType EXTERNAL_IDENTIFIER_MODE_TYPE;
            static const char                        *EXTERNAL_IDENTIFIER_DEFAULT_VALUE;
            static const bool                        EXTERNAL_IDENTIFIER_REQUIRED;

            static const RequiredArguments REQUIRED_ARGS;

        // Constructor/Destructor
        public:
            AnalysisProgram(const SharedLogger &logger = BASIC_LOGGER());
            virtual ~AnalysisProgram() = default;
        
        // Deleted
        public:
            AnalysisProgram(const AnalysisProgram&)            = delete;
            AnalysisProgram &operator=(const AnalysisProgram&) = delete;

        // Funciones miembro
        public:
            Utils::ExitCode run(int totalArgs, char **args);

        private:
            bool analyzeArguments(int totalArgs, char **args);
            Utils::ExitCode work(bool requisitesMet);

        // Variables miembro
        private:
            std::string externalIdentifier = EXTERNAL_IDENTIFIER_DEFAULT_VALUE;
    };
};
