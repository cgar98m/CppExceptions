#pragma once

#include <windows.h>

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////////
        // Informacion minima para generar un mini dump //
        //////////////////////////////////////////////////
        
        struct MiniDumpInfo
        {
            // Funciones miembro
            public:
                bool isValid() const;
            
            // Variables miembro
            public:
                HANDLE              process   = GetCurrentProcess();
                DWORD               processId = GetCurrentProcessId();
                DWORD               threadId  = GetCurrentThreadId();
                PEXCEPTION_POINTERS exception = nullptr;
        };
    };
};
