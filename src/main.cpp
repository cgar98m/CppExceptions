#include <windows.h>

#include <iomanip>
#include <memory>

#include "logger/ConsoleLogger.h"
#include "logger/ILogger.h"
#include "parser/NamedArgumentParser.h"
#include "Version.h"

int main(int argc, char **argv)
{
    // Obtenemos el logger
    std::shared_ptr<Logger::ILogger> l_logger;
    l_logger = Logger::ConsoleLogger::getInstance();

    // Mostramos la versión
    LOGGER_LOG(l_logger) << "CppExtensions v"
                         << std::setfill('0') << std::setw(2) << VERSION_MAJOR << "."
                         << std::setfill('0') << std::setw(2) << VERSION_MINOR;

    // Analizamos los argumentos
    Parser::NamedArgumentParser::ArgumentHeader l_argHeader = { Parser::INTEGER_ARGUMENT, "ErrorTest" };
    Parser::NamedArgumentParser l_parser({ l_argHeader });

    l_parser.feed(argc, argv);
    Parser::NamedArgumentParser::ArgumentValue l_argValue = l_parser.getValue(l_argHeader);
    if (!l_argValue)
    {
        LOGGER_LOG(l_logger) << "Argumento NO identificado";
        return 1;
    }
    
    // Convertimos el argumento a un valor valido
    if (l_argValue->type() != Parser::INTEGER_ARGUMENT)
    {
        LOGGER_LOG(l_logger) << "Argumento SIN tratar";
        return 1;
    }

    Parser::IntArgumentValue* l_intArg = dynamic_cast<Parser::IntArgumentValue*>(l_argValue.get());
    if (!l_intArg)
    {
        LOGGER_LOG(l_logger) << "Argumento de tipo entero incoherente";
        return 1;
    }

    // Tratamos la opcion elegida
    LOGGER_LOG(l_logger) << "Argumento identificado: " << l_intArg->value();

    return 0;
}
