#pragma once

#include <windows.h>
#include <vector>
#include "utils/exception/ipc/ExceptionRecord.h"

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////////////////////////////
        // EXCEPTION_POINTERS copiable (no serializable)  //
        ////////////////////////////////////////////////////
        
        struct ExceptionPointers
        {
            // Constructor/Destructor
            public:
                ExceptionPointers() = default;
                virtual ~ExceptionPointers();

                explicit ExceptionPointers(const ExceptionPointers &exceptionPointers);
                explicit ExceptionPointers(const EXCEPTION_POINTERS &exceptionPointers);

            // Operadores
            public:
                ExceptionPointers &operator=(const ExceptionPointers &exceptionPointers);
                ExceptionPointers &operator=(const EXCEPTION_POINTERS &exceptionPointers);

                PEXCEPTION_POINTERS operator()();

            // Funciones miembro
            public:
                void clearMsvcExceptionPointers();

            // Variables miembro
            public:
                CONTEXT contextRecord = {};
                std::vector<ExceptionRecord> exceptionRecords;
        
                PEXCEPTION_POINTERS exceptionPointers = nullptr;
        };
    };
};
