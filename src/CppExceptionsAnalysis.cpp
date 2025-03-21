#include "Config.h"
#include "error/Exception.h"
#include "utils/ExitCode.h"
#include "utils/filesystem/FileTools.h"
#include "utils/logging/FileLogger.h"

int main(int argc, char **argv)
{
    Utils::Logger logger = Utils::FileLogger::getInstance("DumpAnalysis", Utils::FileTools::OUTPUT_PATH);

    if (!CONFIG_EXTERNALIZE_DUMPS)
    {
        LOGGER_LOG_INFO(logger) << "No se puede ejecutar el programa";
        return static_cast<int>(Utils::ExitCode::EXIT_CODE_NOT_IMPLEMENTED);
    }

    Error::ExceptionManager exceptionManager(true, false, logger);

    // Creamos el manager
    Error::ExternalExceptionManager externalExceptionManager(false, logger);
    if (!externalExceptionManager.isValid())
    {
        LOGGER_LOG_INFO(logger) << "ERROR creando manejador de excepciones externas";
        return static_cast<int>(Utils::ExitCode::EXIT_CODE_KO);
    }

    // Recibimos los datos de la excepcion y la manejamos
    if (!externalExceptionManager.receiveException())
    {
        LOGGER_LOG_INFO(logger) << "ERROR procesando excepcion";
        return static_cast<int>(Utils::ExitCode::EXIT_CODE_KO);
    }

    return static_cast<int>(Utils::ExitCode::EXIT_CODE_OK);
}
