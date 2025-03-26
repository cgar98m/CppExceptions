#pragma once

#include <windows.h>

namespace Utils
{
    namespace Exception
    {
        //////////////////////////////////////////////////
        // Informacion minima para generar un mini dump //
        //////////////////////////////////////////////////
        
        struct RequiredExceptionInfo
        {
            // Funciones miembro
            public:
                bool isValid() const;
                bool isFullyValid() const;
            
            // Variables miembro
            public:
                HANDLE              process   = GetCurrentProcess();
                DWORD               processId = GetCurrentProcessId();
                HANDLE              thread    = GetCurrentThread();
                DWORD               threadId  = GetCurrentThreadId();
                PEXCEPTION_POINTERS exception = nullptr;
        };
    };
};
