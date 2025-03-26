#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "utils/library/DllObject.hpp"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LoggerHolder.h"

namespace Utils
{
    namespace Library
    {
        //////////////////////////////////////////
        // Manejador de librerias DLL dinamicas //
        //////////////////////////////////////////
        
        class DllManager: public Logging::LoggerHolder
        {
            // Tipos, estructuras y enums
            private:
                using DllList          = std::map<std::string, SharedDllObject>;
                using UniqueDllManager = std::unique_ptr<DllManager>;
    
            // Constructor/Destructor
            public:
                virtual ~DllManager() = default;
    
            private:
                DllManager(const SharedLogger &logger = BASIC_LOGGER());
            
            // Deleted
            public:
                DllManager(const DllManager&)            = delete;
                DllManager &operator=(const DllManager&) = delete;

            // Funciones de clase
            public:
                static SharedDllObject getInstance(const std::string &dllName, const SharedLogger &logger = BASIC_LOGGER());
    
            // Funciones miembro
            private:
                SharedDllObject getModule(const std::string &dllName);

                std::string getUniqueDllName(const std::string &dllName);

            // Variables de clase
            private:
                static UniqueDllManager instance;
                static std::mutex       instanceMutex;
    
            // Variables miembro
            private:
                DllList    dllList;
                std::mutex dllMutex;
        };
    };
};
