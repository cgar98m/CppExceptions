#include "Config.h"
#include "utils/ExitCode.h"
#include "utils/exception/ExceptionManager.h"
#include "utils/exception/ipc/IpcExceptionManager.h"
#include "utils/logging/FileLogger.h"
#include "utils/logging/ILogger.h"

#define LOG_FILE_NAME "DumpAnalysis"

int main(int argc, char **argv)
{
    // Verificamos si podemos ejecutarnos
    SharedLogger logger = FILE_LOGGER(LOG_FILE_NAME);
    if (!CONFIG_EXTERNALIZE_DUMPS)
    {
        LOGGER_LOG_ERROR(logger) << "No se puede ejecutar el programa";
        return static_cast<int>(Utils::ExitCode::EXIT_CODE_NOT_IMPLEMENTED);
    }

    // Sobreescribimos el comportamiento de las excepciones
    Utils::Exception::ExceptionManager exceptionManager(true, Utils::Exception::ExceptionManager::Params{false, logger, LOG_FILE_NAME});

    // Creamos el gestor de excepciones externas en modo escucha
    Utils::Exception::IpcExceptionManager externalExceptionManager("TESTING", false, logger);
    if (!externalExceptionManager.isValid())
    {
        LOGGER_LOG_ERROR(logger) << "ERROR creando manejador de excepciones externas";
        return static_cast<int>(Utils::ExitCode::EXIT_CODE_KO);
    }

    // Recibimos los datos de la excepcion y la manejamos
    if (!externalExceptionManager.receiveException())
    {
        LOGGER_LOG_ERROR(logger) << "ERROR procesando excepcion";
        return static_cast<int>(Utils::ExitCode::EXIT_CODE_KO);
    }

    return static_cast<int>(Utils::ExitCode::EXIT_CODE_OK);
}
