#pragma once

#include <windows.h>
#include "utils/exception/ipc/ExceptionRecord.h"

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////////////////
        // EXCEPTION_POINTERS limitado para memoria compartida  //
        //////////////////////////////////////////////////////////
        
        struct LimitedExceptionPointer
        {
            // Variable miembro
            public:
                bool            isValid       = false;
                DWORD           processId     = GetCurrentProcessId();
                DWORD           threadId      = GetCurrentThreadId();
                CONTEXT         contextRecord = {};
                ExceptionRecord exceptionRecord;
        };
    };
};
