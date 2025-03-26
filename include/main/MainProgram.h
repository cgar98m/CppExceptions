#pragma once

#include <string>
#include "main/WorkMode.h"
#include "utils/ExitCode.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/parser/argument/ArgumentType.h"
#include "utils/parser/argument/RequiredArgument.h"

namespace Main
{
    ////////////////////////////////////
    // Wrapper del programa principal //
    ////////////////////////////////////

    class MainProgram: public Utils::Logging::LoggerHolder
    {
        // Constantes
        private:
            static const char                        *WORK_MODE_ARG;
            static const Utils::Parser::ArgumentType WORK_MODE_TYPE;
            static const WorkMode::Mode              WORK_MODE_DEFAULT_VALUE;
            static const bool                        WORK_MODE_REQUIRED;

            static const char                        *IDENTIFIER_ARG;
            static const Utils::Parser::ArgumentType IDENTIFIER_MODE_TYPE;
            static const char                        *IDENTIFIER_DEFAULT_VALUE;
            static const bool                        IDENTIFIER_REQUIRED;

            static const RequiredArguments REQUIRED_ARGS;

        // Constructor/Destructor
        public:
            MainProgram(const SharedLogger &logger = BASIC_LOGGER());
            virtual ~MainProgram() = default;
        
        // Deleted
        public:
            MainProgram(const MainProgram&)            = delete;
            MainProgram &operator=(const MainProgram&) = delete;

        // Funciones miembro
        public:
            Utils::ExitCode run(int totalArgs, char **args);

        private:
            bool analyzeArguments(int totalArgs, char **args);
            Utils::ExitCode work(bool requisitesMet);

        // Variables miembro
        private:
            WorkMode::Mode workMode   = WorkMode::Mode::UNDEFINED;
            std::string    identifier = IDENTIFIER_DEFAULT_VALUE;
    };
};
