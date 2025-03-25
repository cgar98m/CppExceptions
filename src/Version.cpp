#include "Version.h"

#include <iomanip>
#include "utils/logging/LogEntry.h"

namespace Core
{
    ////////////////////////////////////
    // Gestion de version de software //
    ////////////////////////////////////

    ////////////////////////
    // Funciones de clase //
    ////////////////////////

    void Version::notifyVersion(const SharedLogger &logger)
    {
        // Mostramos la versión
        LOGGER_LOG_INFO(logger) << "VERSION: "
                                << std::setfill('0') << std::setw(2) << VERSION_MAJOR << "."
                                << std::setfill('0') << std::setw(2) << VERSION_MINOR;
    }
};
