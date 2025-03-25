#pragma once

#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 8

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
