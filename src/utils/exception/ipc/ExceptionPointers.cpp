#include "utils/exception/ipc/ExceptionPointers.h"

#include <stack>

namespace Utils
{
    namespace Exception
    {
        ////////////////////////////////////////////////////
        // EXCEPTION_POINTERS copiable (no serializable)  //
        ////////////////////////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        ExceptionPointers::~ExceptionPointers()
        {
            this->clearMsvcExceptionPointers();
        }

        ExceptionPointers::ExceptionPointers(const ExceptionPointers &exceptionPointers)
            : contextRecord(exceptionPointers.contextRecord)
            , exceptionRecords(exceptionPointers.exceptionRecords)
        {
        }

        ExceptionPointers::ExceptionPointers(const EXCEPTION_POINTERS &exceptionPointers)
        {
            if (exceptionPointers.ContextRecord) this->contextRecord = *exceptionPointers.ContextRecord;

            // Iteramos por los registros hasta que no encontremos ninguno
            PEXCEPTION_RECORD exceptionRecord = exceptionPointers.ExceptionRecord;
            while (exceptionRecord)
            {
                this->exceptionRecords.push_back(ExceptionRecord(*exceptionRecord));
                exceptionRecord = exceptionRecord->ExceptionRecord;
            }
        }

        ExceptionPointers &ExceptionPointers::operator=(const ExceptionPointers &exceptionPointers)
        {
            this->contextRecord    = exceptionPointers.contextRecord;
            this->exceptionRecords = exceptionPointers.exceptionRecords;

            return *this;
        }

        ExceptionPointers &ExceptionPointers::operator=(const EXCEPTION_POINTERS &exceptionPointers)
        {
            if (exceptionPointers.ContextRecord) this->contextRecord = *exceptionPointers.ContextRecord;

            PEXCEPTION_RECORD exceptionRecord = exceptionPointers.ExceptionRecord;
            while (exceptionRecord)
            {
                this->exceptionRecords.push_back(ExceptionRecord(*exceptionRecord));
                exceptionRecord = exceptionRecord->ExceptionRecord;
            }

            return *this;
        }
        
        PEXCEPTION_POINTERS ExceptionPointers::operator()()
        {
            if (!this->exceptionPointers)
            {
                // Creamos el puntero principal
                this->exceptionPointers = new EXCEPTION_POINTERS();
                if (!this->exceptionPointers) return nullptr;

                // Copiamos el contexto
                this->exceptionPointers->ContextRecord = new CONTEXT();
                if (!this->exceptionPointers->ContextRecord)
                {
                    clearMsvcExceptionPointers();
                    return nullptr;
                }
                std::memcpy(this->exceptionPointers->ContextRecord, &this->contextRecord, sizeof(CONTEXT));

                // Copiamos los registros
                PEXCEPTION_RECORD previousRecord = nullptr;
                for (auto itExceptionRecord = this->exceptionRecords.begin(); itExceptionRecord != this->exceptionRecords.end(); ++itExceptionRecord)
                {
                    // Creamos el registro
                    PEXCEPTION_RECORD record = new EXCEPTION_RECORD();
                    if (!record)
                    {
                        clearMsvcExceptionPointers();
                        return nullptr;
                    }

                    // Copiamos los datos
                    record->ExceptionCode    = itExceptionRecord->exceptionCode;
                    record->ExceptionFlags   = itExceptionRecord->exceptionFlags;
                    record->ExceptionAddress = itExceptionRecord->exceptionAddress;
                    record->NumberParameters = itExceptionRecord->numberParameters;
                    record->ExceptionRecord  = nullptr;

                    ZeroMemory(record->ExceptionInformation, sizeof(record->ExceptionInformation));
                    std::memcpy(record->ExceptionInformation, itExceptionRecord->exceptionInformation, sizeof(ULONG_PTR) * record->NumberParameters);

                    // Asignamos la direccion de memoria al registro padre
                    if (previousRecord) previousRecord->ExceptionRecord          = record;
                    else                this->exceptionPointers->ExceptionRecord = record;
                    previousRecord = record;
                }
            }

            return this->exceptionPointers;
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        void ExceptionPointers::clearMsvcExceptionPointers()
        {
            if (this->exceptionPointers)
            {
                // Liberamos el contexto
                if (this->exceptionPointers->ContextRecord)
                {
                    delete this->exceptionPointers->ContextRecord;
                    this->exceptionPointers->ContextRecord = nullptr;
                }

                // Iteramos por los registros
                std::stack<PEXCEPTION_RECORD> records;
                PEXCEPTION_RECORD exceptionRecord = this->exceptionPointers->ExceptionRecord;
                while (exceptionRecord)
                {
                    records.push(exceptionRecord);
                    exceptionRecord = exceptionRecord->ExceptionRecord;
                }

                // Liberamos los registros de la memoria en orden inverso
                while (!records.empty())
                {
                    exceptionRecord = records.top();
                    if (exceptionRecord->ExceptionRecord)
                    {
                        delete exceptionRecord->ExceptionRecord;
                        exceptionRecord->ExceptionRecord = nullptr;
                    }
                    records.pop();
                }

                // Liberamos el puntero principal
                delete this->exceptionPointers;
                this->exceptionPointers = nullptr;
            }
        }
    };
};
