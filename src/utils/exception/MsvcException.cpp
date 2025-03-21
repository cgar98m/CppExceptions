#include "utils/exception/MsvcException.h"

#include <cstring>
#include <stack>

namespace Utils
{
    ////////////////////////////////
    // EXCEPTION_RECORD copiable  //
    ////////////////////////////////
    
    ExceptionRecord::ExceptionRecord(const ExceptionRecord& exceptionRecord)
        : exceptionCode(exceptionRecord.exceptionCode)
        , exceptionFlags(exceptionRecord.exceptionFlags)
        , isLast(exceptionRecord.isLast)
        , exceptionAddress(exceptionRecord.exceptionAddress)
        , numberParameters(exceptionRecord.numberParameters)
    {
        ZeroMemory(exceptionInformation, sizeof(exceptionInformation));
        std::memcpy(exceptionInformation, exceptionRecord.exceptionInformation, sizeof(ULONG_PTR) * numberParameters);
    }

    ExceptionRecord::ExceptionRecord(const EXCEPTION_RECORD& exceptionRecord)
        : exceptionCode(exceptionRecord.ExceptionCode)
        , exceptionFlags(exceptionRecord.ExceptionFlags)
        , isLast(exceptionRecord.ExceptionRecord == nullptr)
        , exceptionAddress(exceptionRecord.ExceptionAddress)
        , numberParameters(exceptionRecord.NumberParameters)
    {
        ZeroMemory(exceptionInformation, sizeof(exceptionInformation));
        std::memcpy(exceptionInformation, exceptionRecord.ExceptionInformation, sizeof(ULONG_PTR) * numberParameters);
    }

    ExceptionRecord& ExceptionRecord::operator=(const ExceptionRecord& exceptionRecord)
    {
        exceptionCode    = exceptionRecord.exceptionCode;
        exceptionFlags   = exceptionRecord.exceptionFlags;
        isLast           = exceptionRecord.isLast;
        exceptionAddress = exceptionRecord.exceptionAddress;
        numberParameters = exceptionRecord.numberParameters;
        
        ZeroMemory(&exceptionInformation, sizeof(exceptionInformation));
        std::memcpy(exceptionInformation, exceptionRecord.exceptionInformation, sizeof(ULONG_PTR) * numberParameters);
        
        return *this;
    }

    ExceptionRecord& ExceptionRecord::operator=(const EXCEPTION_RECORD& exceptionRecord)
    {
        exceptionCode    = exceptionRecord.ExceptionCode;
        exceptionFlags   = exceptionRecord.ExceptionFlags;
        isLast           = exceptionRecord.ExceptionRecord == nullptr;
        exceptionAddress = exceptionRecord.ExceptionAddress;
        numberParameters = exceptionRecord.NumberParameters;
        
        ZeroMemory(&exceptionInformation, sizeof(exceptionInformation));
        std::memcpy(exceptionInformation, exceptionRecord.ExceptionInformation, sizeof(ULONG_PTR) * numberParameters);
        
        return *this;
    }

    ////////////////////////////////////////////////////
    // EXCEPTION_POINTERS copiable (no serializable)  //
    ////////////////////////////////////////////////////

    ExceptionPointers::ExceptionPointers(const ExceptionPointers& exceptionPointers)
        : contextRecord(exceptionPointers.contextRecord)
        , exceptionRecords(exceptionPointers.exceptionRecords)
    {
    }

    ExceptionPointers::ExceptionPointers(const EXCEPTION_POINTERS& exceptionPointers)
    {
        if (exceptionPointers.ContextRecord) contextRecord = *exceptionPointers.ContextRecord;

        PEXCEPTION_RECORD exceptionRecord = exceptionPointers.ExceptionRecord;
        while (exceptionRecord)
        {
            exceptionRecords.push_back(ExceptionRecord(*exceptionRecord));
            exceptionRecord = exceptionRecord->ExceptionRecord;
        }
    }

    ExceptionPointers& ExceptionPointers::operator=(const ExceptionPointers& exceptionPointers)
    {
        contextRecord    = exceptionPointers.contextRecord;
        exceptionRecords = exceptionPointers.exceptionRecords;

        return *this;
    }

    ExceptionPointers& ExceptionPointers::operator=(const EXCEPTION_POINTERS& exceptionPointers)
    {
        if (exceptionPointers.ContextRecord) contextRecord = *exceptionPointers.ContextRecord;

        PEXCEPTION_RECORD exceptionRecord = exceptionPointers.ExceptionRecord;
        while (exceptionRecord)
        {
            exceptionRecords.push_back(ExceptionRecord(*exceptionRecord));
            exceptionRecord = exceptionRecord->ExceptionRecord;
        }

        return *this;
    }
    
    ExceptionPointers::~ExceptionPointers()
    {
        if (exceptionPointers)
        {
            if (exceptionPointers->ContextRecord) delete exceptionPointers->ContextRecord;

            std::stack<PEXCEPTION_RECORD> records;
            PEXCEPTION_RECORD exceptionRecord = exceptionPointers->ExceptionRecord;
            while (exceptionRecord)
            {
                records.push(exceptionRecord);
                exceptionRecord = exceptionRecord->ExceptionRecord;
            }

            while (!records.empty())
            {
                exceptionRecord = records.top();
                if (exceptionRecord->ExceptionRecord) delete exceptionRecord->ExceptionRecord;
                exceptionRecord->ExceptionRecord = nullptr;
                records.pop();
            }

            delete exceptionPointers;
            exceptionPointers = nullptr;
        }
    }
    
    PEXCEPTION_POINTERS ExceptionPointers::operator()()
    {
        if (!exceptionPointers)
        {
            exceptionPointers = new EXCEPTION_POINTERS();
            if (!exceptionPointers) return nullptr;

            exceptionPointers->ContextRecord = new CONTEXT();
            if (!exceptionPointers->ContextRecord)
            {
                delete exceptionPointers->ContextRecord;
                delete exceptionPointers;
                exceptionPointers = nullptr;
                return nullptr;
            }
            std::memcpy(exceptionPointers->ContextRecord, &contextRecord, sizeof(CONTEXT));

            std::stack<PEXCEPTION_RECORD> records;
            PEXCEPTION_RECORD previousRecord = nullptr;
            for (auto itExceptionRecord = exceptionRecords.begin(); itExceptionRecord != exceptionRecords.end(); ++itExceptionRecord)
            {
                PEXCEPTION_RECORD record = new EXCEPTION_RECORD();
                if (!record) break;

                record->ExceptionCode    = itExceptionRecord->exceptionCode;
                record->ExceptionFlags   = itExceptionRecord->exceptionFlags;
                record->ExceptionAddress = itExceptionRecord->exceptionAddress;
                record->NumberParameters = itExceptionRecord->numberParameters;
                record->ExceptionRecord  = nullptr;

                ZeroMemory(record->ExceptionInformation, sizeof(record->ExceptionInformation));
                std::memcpy(record->ExceptionInformation, itExceptionRecord->exceptionInformation, sizeof(ULONG_PTR) * record->NumberParameters);

                if (previousRecord) previousRecord->ExceptionRecord = record;
                else                exceptionPointers->ExceptionRecord = record;
                records.push(record);
                previousRecord = record;
            }

            if (records.size() != exceptionRecords.size())
            {
                while (!records.empty())
                {
                    PEXCEPTION_RECORD record = records.top();
                    if (record->ExceptionRecord) delete record->ExceptionRecord;
                    record->ExceptionRecord = nullptr;
                    records.pop();
                }

                delete exceptionPointers->ContextRecord;
                delete exceptionPointers;
                exceptionPointers = nullptr;
                return nullptr;
            }
        }

        return exceptionPointers;
    }
};
