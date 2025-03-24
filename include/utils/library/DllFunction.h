#pragma once

#include <windows.h>
#include <memory>
#include <mutex>
#include <string>
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Library
    {
        //////////////////////////////////////
        // Wrapper de la funcion de una DLL //
        //////////////////////////////////////

        class DllFunction: public Logging::LoggerHolder
        {
            // Constructor/Destructor
            public:
                explicit DllFunction(const std::string &funcName, HMODULE module, const SharedLogger &logger = BASIC_LOGGER());
                virtual ~DllFunction() = default;
            
            // Deleted
            public:
                DllFunction()                              = delete;
                explicit DllFunction(const DllFunction&)   = delete;
                DllFunction &operator=(const DllFunction&) = delete;

            // Funciones miembro
            public:
                bool isValid() const;
                FARPROC getAddress() const;
                std::mutex &getMutex();

            // Variables miembro
            private:
                FARPROC           funcAddress = nullptr;
                const std::string funcName;
                std::mutex        funcMutex;
        };
    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using SharedDllFunction = std::shared_ptr<Utils::Library::DllFunction>;
