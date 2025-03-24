#pragma once

#include <windows.h>

namespace Utils
{
    namespace Logging
    {
        ////////////////
        // Constantes //
        ////////////////
        
        const size_t LOGGER_PRINT_BUFFER_SIZE = 1024;       // 1KB
        const size_t LOGGER_PRINT_LIMIT_SIZE  = 1024 * 16;  // 16KB
        const DWORD  LOGGER_FLUSH_TIMEOUT     = 2000;       // 2s
    };
};
