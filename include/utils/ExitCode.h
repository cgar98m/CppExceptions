#pragma once

#include <windows.h>

namespace Utils
{
    // Codigos de error
    enum class ExitCode: DWORD
    {
        EXIT_CODE_OK = 0,
        EXIT_CODE_KO,
        EXIT_CODE_TIMEOUT,
        EXIT_CODE_NOT_AVAILABLE = 10,
        EXIT_CODE_EXCEPTION,
        EXIT_CODE_TERMINATE
    };
};