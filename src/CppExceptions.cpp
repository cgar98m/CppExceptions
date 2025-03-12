#include <windows.h>
#include "CMakeDefine.h"
#include "error/Exception.h"
#include "logger/ConsoleLogger.h"
#include "utils/Program.h"

int main(int argc, char **argv)
{
    // Obtenemos el logger
    Logger::Logger logger = Logger::ConsoleLogger::getInstance();

    // Instanciamos el programa
    Utils::Main program(logger);

    // Ejecutamos el programa
    int result = static_cast<int>(program.run(argc, argv));

    // Esperamos para que las trazas se escriban
    Sleep(Logger::LOGGER_STANDARD_OUTPUT_FLUSH_TIMEOUT);
    return result;
}
