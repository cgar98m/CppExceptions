#pragma once

#include "logger/ILogger.h"

#include <windows.h>

#include <exception>
#include <functional>

#include "parser/NamedArgumentParser.h"

namespace Program
{
    // Wrapper del programa principal
    class Main
    {
        public:
            Main();
            ~Main();

            int run(int argc, char **argv);
        
        private:
            struct RequiredArgument
            {
                Parser::NamedArgumentParser::ArgumentHeader argument;
                bool required;
            };

            enum class WorkMode : uint32_t
            {
                UNDEFINED = 0,
                THROW_CCP_EXCEPTION,
                THROW_SEH_EXCEPTION
            };

            using ArgList  = std::vector<RequiredArgument>;
            using ArgValue = Parser::NamedArgumentParser::ArgumentValue;

            // Previa a la ejecucion del programa
            void clear();
            void notifyVersion();
            void parseArguments(int argc, char **argv);
            bool analyzeArgument(std::string name, ArgValue argument);

            // Logica del programa
            int work();

            // Utils
            std::string getWorkModeDescription(WorkMode workMode);
            
        private:
            static const ArgList m_argList;

            std::shared_ptr<Logger::ILogger> m_logger;
            WorkMode m_workMode;

            LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;
            std::terminate_handler       m_previousTerminate;
    };
};
