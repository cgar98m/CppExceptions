#include "CMakeDefine.h"
#include "error/Exception.h"
#include "error/Types.h"

int main(int argc, char **argv)
{
    Logger::Logger logger = Logger::ConsoleLogger::getInstance();

    if (!EXTERNALIZE_DUMPS)
    {
        LOGGER_LOG(logger) << "No se puede ejecutar el programa";
        return static_cast<int>(Error::ExitCode::EXIT_CODE_NOT_IMPLEMENTED);
    }

    Error::ExceptionManager exceptionManager(true, false, logger);

    // Creamos el manager
    Error::ExternalExceptionManager externalExceptionManager(false, logger);
    if (!externalExceptionManager.isValid())
    {
        LOGGER_LOG(logger) << "Error creando manejador de excepciones externas";
        return static_cast<int>(Error::ExitCode::EXIT_CODE_KO);
    }

    // Recibimos los datos de la excepcion y la manejamos
    if (!externalExceptionManager.receiveException())
    {
        LOGGER_LOG(logger) << "Error procesando excepcion";
        return static_cast<int>(Error::ExitCode::EXIT_CODE_KO);
    }

    return static_cast<int>(Error::ExitCode::EXIT_CODE_OK);
}
