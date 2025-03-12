#pragma once

#include <windows.h>
#include <exception>
#include <functional>
#include "error/Types.h"
#include "logger/ILogger.h"
#include "parser/NamedArgumentParser.h"

namespace Utils
{
    // Wrapper del programa principal
    class Main: public Logger::ILoggerHolder
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
            
            WorkMode workMode;
            
            static const std::string ARG_ERROR_MODE;

            static const ArgList ARG_LIST;

        public:
            explicit Main(const Logger::Logger& logger = Logger::BasicLogger::getInstance());
            Main(const Main&) = delete;
            Main& operator=(const Main&) = delete;
            virtual ~Main() = default;

            Error::ExitCode run(int argc, char **argv);
        
        private:
            // Previa a la ejecucion del programa
            void clear();
            void notifyVersion();
            void parseArguments(int argc, char **argv);
            bool analyzeArgument(std::string name, ArgValue argument);

            // Logica del programa
            Error::ExitCode work();

            // Utils
            std::string getWorkModeDescription(WorkMode workMode);
    };
};
