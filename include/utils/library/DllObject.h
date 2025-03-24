#pragma once

#include <windows.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "utils/library/DllFunction.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Library
    {
        ////////////////////////
        // Wrapper de una DLL //
        ////////////////////////

        class DllObject: public Logging::LoggerHolder
        {
            // Tipos, estructuras y enums
            private:
                using FuncList = std::map<std::string, SharedDllFunction>;

            // Constructor/Destructor
            public:
                explicit DllObject(const std::string &dllName, const SharedLogger &logger = BASIC_LOGGER());
                virtual ~DllObject();

            // Deleted
            public:
                DllObject()                            = delete;
                explicit DllObject(const DllObject&)   = delete;
                DllObject &operator=(const DllObject&) = delete;

            // Funciones miembro
            public:
                bool isValid();
                SharedDllFunction getFunction(const std::string &funcName);
                bool deleteFunction(const std::string &funcName);

            // Variables miembro
            private:
                HMODULE           moduleHandle = nullptr;
                const std::string dllName;
                std::mutex        dllMutex;
    
                FuncList   funcList;
                std::mutex funcMutex;
        };
    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using SharedDllObject = std::shared_ptr<Utils::Library::DllObject>;
