#include <windows.h>
#include "error/Exception.h"
#include "utils/Program.h"
#include "utils/logging/ConsoleLogger.h"

int main(int argc, char **argv)
{
    // Obtenemos el logger
    Utils::Logger logger = Utils::ConsoleLogger::getInstance();

    // Instanciamos el programa
    Utils::Main program(logger);

    // Ejecutamos el programa
    return static_cast<int>(program.run(argc, argv));
}
