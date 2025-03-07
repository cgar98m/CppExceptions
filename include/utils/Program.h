#pragma once

#include "logger/ILogger.h"

#include <windows.h>
#include <exception>
#include <functional>
#include "parser/NamedArgumentParser.h"

namespace Utils
{
    // Wrapper del programa principal
    class Main
    {
        private:
            struct RequiredArgument
            {
                Parser::NamedArgumentParser::ArgumentHeader argument;
                bool                                        required;
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
            using ArgValue = Parser::NamedArgumentParser::ArgumentValue;
            
            std::shared_ptr<Logger::ILogger> logger;
            WorkMode workMode;
            
            static const std::string ARG_ERROR_MODE;

            static const ArgList ARG_LIST;

        public:
            Main();
            virtual ~Main() = default;

            int run(int argc, char **argv);
        
        private:
            // Previa a la ejecucion del programa
            void clear();
            void notifyVersion();
            void parseArguments(int argc, char **argv);
            bool analyzeArgument(std::string name, ArgValue argument);

            // Logica del programa
            int work();

            // Utils
            std::string getWorkModeDescription(WorkMode workMode);
    };
};
