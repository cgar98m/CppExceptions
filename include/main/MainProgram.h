#pragma once

#include <windows.h>
#include <exception>
#include <functional>
#include "utils/ExitCode.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"
#include "utils/parser/argument/ArgumentTag.h"
#include "utils/parser/argument/NamedArgumentParser.h"

namespace Main
{
    // Wrapper del programa principal
    class MainProgram: public Utils::Logging::LoggerHolder
    {
        private:
            struct RequiredArgument
            {
                Utils::Parser::ArgumentTag argument;
                bool                       required;
            };

            enum class WorkMode : uint32_t
            {
                UNDEFINED = 0,
                THROW_CCP_EXCEPTION,
                THROW_SEH_EXCEPTION,
                THROW_THREADED_CPP_EXCEPTION,
                THROW_THREADED_SEH_EXCEPTION
            };

            using ArgList  = std::vector<RequiredArgument>;
            using ArgValue = Utils::Parser::NamedArgumentParser::NamedArgumentValue;
            
            static const char *ARG_ERROR_MODE;
            static const char *ARG_IDENTIFIER;
            static const char *DEFAULT_IDENTIFIER;

            static const ArgList ARG_LIST;

            WorkMode    workMode;
            std::string identifier = DEFAULT_IDENTIFIER;

        public:
            explicit MainProgram(const SharedLogger& logger = BASIC_LOGGER());
            MainProgram(const MainProgram&) = delete;
            MainProgram& operator=(const MainProgram&) = delete;
            virtual ~MainProgram() = default;

            Utils::ExitCode run(int argc, char **argv);
        
        private:
            // Previa a la ejecucion del programa
            void clear();
            void notifyVersion();
            void parseArguments(int argc, char **argv);
            bool analyzeArgument(std::string name, ArgValue argument);

            // Logica del programa
            Utils::ExitCode work();

            // Utils
            std::string getWorkModeDescription(WorkMode workMode);
    };
};
