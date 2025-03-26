#pragma once

#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

namespace Core
{
    ////////////////////////////////////
    // Gestion de version de software //
    ////////////////////////////////////

    class Version
    {
        // Funciones de clase
        public:
            static void notifyVersion(const SharedLogger &logger = BASIC_LOGGER());
    };
};
