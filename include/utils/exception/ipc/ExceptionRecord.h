#pragma once

#include <windows.h>

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////////
        // EXCEPTION_RECORD copiable  //
        ////////////////////////////////
        
        struct ExceptionRecord
        {
            // Constructor/Destructor
            public:
                ExceptionRecord()          = default;
                virtual ~ExceptionRecord() = default;

                explicit ExceptionRecord(const ExceptionRecord &exceptionRecord);
                explicit ExceptionRecord(const EXCEPTION_RECORD &exceptionRecord);

            // Operadores
            public:
                ExceptionRecord &operator=(const ExceptionRecord &exceptionRecord);
                ExceptionRecord &operator=(const EXCEPTION_RECORD &exceptionRecord);

            // Variables miembro
            public:
                DWORD     exceptionCode    = 0;
                DWORD     exceptionFlags   = 0;
                bool      isLast           = true;
                PVOID     exceptionAddress = nullptr;
                DWORD     numberParameters = 0;
                ULONG_PTR exceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS] = {};
        };
    };
};
