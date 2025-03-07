#include "error/Exception.h"
#include "utils/Program.h"

int main(int argc, char **argv)
{
    // Insranciamos el gestor de errores
    Error::ExceptionManager exceptionManager(true);

    // Instanciamos el programa
    Utils::Main program;

    // Ejecutamos el programa
    return program.run(argc, argv);
}
