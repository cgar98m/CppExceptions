#include "utils/exception/ipc/ExceptionRecord.h"

#include <cstring>

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////////
        // EXCEPTION_RECORD copiable  //
        ////////////////////////////////
        
        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ExceptionRecord::ExceptionRecord(const ExceptionRecord &exceptionRecord)
            : exceptionCode(exceptionRecord.exceptionCode)
            , exceptionFlags(exceptionRecord.exceptionFlags)
            , isLast(exceptionRecord.isLast)
            , exceptionAddress(exceptionRecord.exceptionAddress)
            , numberParameters(exceptionRecord.numberParameters)
        {
            // Copiamos lo justo y necesario
            ZeroMemory(this->exceptionInformation, sizeof(this->exceptionInformation));
            std::memcpy(this->exceptionInformation, exceptionRecord.exceptionInformation, sizeof(ULONG_PTR) * this->numberParameters);
        }
    
        ExceptionRecord::ExceptionRecord(const EXCEPTION_RECORD &exceptionRecord)
            : exceptionCode(exceptionRecord.ExceptionCode)
            , exceptionFlags(exceptionRecord.ExceptionFlags)
            , isLast(exceptionRecord.ExceptionRecord == nullptr)
            , exceptionAddress(exceptionRecord.ExceptionAddress)
            , numberParameters(exceptionRecord.NumberParameters)
        {
            // Copiamos lo justo y necesario
            ZeroMemory(this->exceptionInformation, sizeof(this->exceptionInformation));
            std::memcpy(this->exceptionInformation, exceptionRecord.ExceptionInformation, sizeof(ULONG_PTR) * this->numberParameters);
        }
    
        //------------//
        // Operadores //
        //------------//

        ExceptionRecord &ExceptionRecord::operator=(const ExceptionRecord &exceptionRecord)
        {
            this->exceptionCode    = exceptionRecord.exceptionCode;
            this->exceptionFlags   = exceptionRecord.exceptionFlags;
            this->isLast           = exceptionRecord.isLast;
            this->exceptionAddress = exceptionRecord.exceptionAddress;
            this->numberParameters = exceptionRecord.numberParameters;
            
            // Copiamos lo justo y necesario
            ZeroMemory(this->exceptionInformation, sizeof(this->exceptionInformation));
            std::memcpy(this->exceptionInformation, exceptionRecord.exceptionInformation, sizeof(ULONG_PTR) * this->numberParameters);
            
            return *this;
        }
    
        ExceptionRecord &ExceptionRecord::operator=(const EXCEPTION_RECORD &exceptionRecord)
        {
            this->exceptionCode    = exceptionRecord.ExceptionCode;
            this->exceptionFlags   = exceptionRecord.ExceptionFlags;
            this->isLast           = exceptionRecord.ExceptionRecord == nullptr;
            this->exceptionAddress = exceptionRecord.ExceptionAddress;
            this->numberParameters = exceptionRecord.NumberParameters;
            
            // Copiamos lo justo y necesario
            ZeroMemory(this->exceptionInformation, sizeof(this->exceptionInformation));
            std::memcpy(this->exceptionInformation, exceptionRecord.ExceptionInformation, sizeof(ULONG_PTR) * this->numberParameters);
            
            return *this;
        }
    };
};
