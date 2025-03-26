#include "Config.h"
#include "utils/ExitCode.h"
#include "utils/exception/ExceptionManager.h"
#include "utils/exception/ipc/IpcExceptionManager.h"
#include "utils/logging/FileLogger.h"
#include "utils/logging/ILogger.h"

#include "analysis/AnalysisProgram.h"

#define LOG_FILE_NAME "DumpAnalysis"

int main(int argc, char **argv)
{
    // Obtenemos el logger
    SharedLogger logger = FILE_LOGGER(LOG_FILE_NAME);

    // Instanciamos el programa
    Analysis::AnalysisProgram program(logger);

    // Ejecutamos el programa
    return static_cast<int>(program.run(argc, argv));
}
