#pragma once

#include <windows.h>

namespace Error
{
    // Codigos de error
    enum class ExitCode: DWORD
    {
        EXIT_CODE_OK = 0,
        EXIT_CODE_KO,
        EXIT_CODE_NOT_IMPLEMENTED = 10,
        EXIT_CODE_EXCEPTION,
        EXIT_CODE_TERMINATE
    };
};