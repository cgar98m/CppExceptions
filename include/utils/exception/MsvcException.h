#pragma once

#include <windows.h>
#include <vector>

namespace Utils
{
    // EXCEPTION_RECORD copiable
    struct ExceptionRecord
    {
        DWORD     exceptionCode    = 0;
        DWORD     exceptionFlags   = 0;
        bool      isLast           = true;
        PVOID     exceptionAddress = nullptr;
        DWORD     numberParameters = 0;
        ULONG_PTR exceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS] = {};

        ExceptionRecord() = default;
        explicit ExceptionRecord(const ExceptionRecord& exceptionRecord);
        explicit ExceptionRecord(const EXCEPTION_RECORD& exceptionRecord);
        ExceptionRecord& operator=(const ExceptionRecord& exceptionRecord);
        ExceptionRecord& operator=(const EXCEPTION_RECORD& exceptionRecord);
        virtual ~ExceptionRecord() = default;
    };

    // EXCEPTION_POINTERS copiable (no serializable)
    struct ExceptionPointers
    {
        CONTEXT contextRecord = {};
        std::vector<ExceptionRecord> exceptionRecords;

        PEXCEPTION_POINTERS exceptionPointers = nullptr;

        ExceptionPointers() = default;
        explicit ExceptionPointers(const ExceptionPointers& exceptionPointers);
        explicit ExceptionPointers(const EXCEPTION_POINTERS& exceptionPointers);
        ExceptionPointers& operator=(const ExceptionPointers& exceptionPointers);
        ExceptionPointers& operator=(const EXCEPTION_POINTERS& exceptionPointers);
        virtual ~ExceptionPointers();

        PEXCEPTION_POINTERS operator()();
    };

    // EXCEPTION_POINTERS limitado para memoria compartida
    struct LimitedExceptionPointer
    {
        bool            isValid       = false;
        DWORD           processId     = GetCurrentProcessId();
        DWORD           threadId      = GetCurrentThreadId();
        CONTEXT         contextRecord = {};
        ExceptionRecord exceptionRecord;
    };
};
