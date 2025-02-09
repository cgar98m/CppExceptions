#include <windows.h>

#include <iomanip>
#include <memory>

#include "logger/ConsoleLogger.h"
#include "logger/ILogger.h"
#include "Version.h"

int main(int argc, char** argv)
{
    // Obtenemos el logger
    std::shared_ptr<Logger::ILogger> l_oConsoleLogger;
    l_oConsoleLogger = Logger::ConsoleLogger::getInstance();

    // Mostramos la versión
    LOGGER_LOG(l_oConsoleLogger) << "CppExtensions v"
                                 << std::setfill('0') << std::setw(2) << VERSION_MAJOR << "."
                                 << std::setfill('0') << std::setw(2) << VERSION_MINOR;

    return 0;
}
