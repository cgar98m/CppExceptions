#include "utils/library/DllFunction.h"

#include "utils/logging/LogEntry.h"

namespace Utils
{
    namespace Library
    {
        //////////////////////////////////////
        // Wrapper de la funcion de una DLL //
        //////////////////////////////////////

        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        DllFunction::DllFunction(const std::string &funcName, HMODULE module, const SharedLogger &logger)
            : Logging::LoggerHolder(logger)
            , funcName(funcName)
        {
            if (this->funcName.empty() || !module) return;

            // Obtenemos la direccion de la funcion
            this->funcAddress = GetProcAddress(module, this->funcName.c_str());
            if (!this->funcAddress) LOGGER_THIS_LOG_ERROR() << "ERROR obteniendo direccion de " << this->funcName << ": " << GetLastError();
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        bool DllFunction::isValid() const
        {
            return this->funcAddress;
        }

        FARPROC DllFunction::getAddress() const
        {
            return this->funcAddress;
        }

        std::mutex &DllFunction::getMutex()
        {
            return this->funcMutex;
        }
    };
};
