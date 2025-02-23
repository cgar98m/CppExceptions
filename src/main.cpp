#include "program/Program.h"

int main(int argc, char **argv)
{
    // Instanciamos el programa
    Program::Main l_main;

    // Ejecutamos el programa
    return l_main.run(argc, argv);
}
