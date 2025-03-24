#include <windows.h>
#include "main/MainProgram.h"
#include "utils/exception/ExceptionManager.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/ConsoleLogger.h"

int main(int argc, char **argv)
{
    // Obtenemos el logger
    SharedLogger logger = CONSOLE_LOGGER();

    // Instanciamos el programa
    Main::MainProgram program(logger);

    // Ejecutamos el programa
    return static_cast<int>(program.run(argc, argv));
}
