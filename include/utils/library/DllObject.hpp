#pragma once

#include <windows.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "utils/library/DllFunction.h"
#include "utils/logging/BasicLogger.h"
#include "utils/logging/ILogger.h"
#include "utils/logging/LogEntry.h"
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
            //----------------------------//
            // Tipos, estructuras y enums //
            //----------------------------//

            private:
                using FuncList = std::map<std::string, SharedDllFunction>;

            //------------------------//
            // Constructor/Destructor //
            //------------------------//
            
            public:
                explicit DllObject(const std::string &dllName, const SharedLogger &logger = BASIC_LOGGER())
                    : Logging::LoggerHolder(logger)
                    , dllName(dllName)
                {
                    if (this->dllName.empty()) return;
            
                    // Cargamos la DLL en memoria
                    this->moduleHandle = LoadLibraryA(this->dllName.c_str());
                    if (!this->moduleHandle) LOGGER_THIS_LOG_ERROR() << "ERROR cargando DLL " << this->dllName << ": " << GetLastError();
                }

                virtual ~DllObject()
                {
                    // Descargamos la DLL de la memoria
                    std::lock_guard<std::mutex> lock(this->dllMutex);
                    if (this->moduleHandle)
                    {
                        if (!FreeLibrary(this->moduleHandle)) LOGGER_THIS_LOG_WARNING() << "ERROR liberando DLL " << this->dllName << ": " << GetLastError();
                        this->moduleHandle = nullptr;
                    }
                }

            //----------//
            // Deleted  //
            //----------//
            
            public:
                DllObject()                            = delete;
                explicit DllObject(const DllObject&)   = delete;
                DllObject &operator=(const DllObject&) = delete;

            //--------------------//
            // Funciones miembro  //
            //--------------------//

            public:
                bool isValid()
                {
                    std::lock_guard<std::mutex> lock(this->dllMutex);
                    return this->moduleHandle;
                }

                SharedDllFunction getFunction(const std::string &funcName)
                {
                    SharedDllFunction funcWrapper;
        
                    // Verificamos la validez de la DLL
                    std::lock_guard<std::mutex> lockDll(this->dllMutex);
                    if (!this->moduleHandle) return funcWrapper;
            
                    // Verificamos la validez de la funcion
                    std::lock_guard<std::mutex> lockFunc(this->funcMutex);
                    if (funcName.empty()) return funcWrapper;
            
                    // Comprobamos su existencia
                    if (this->funcList.find(funcName) != this->funcList.end()) return this->funcList[funcName];
            
                    // Cargamos la funcion en la lista
                    funcWrapper = std::make_shared<DllFunction>(funcName, this->moduleHandle, THIS_LOGGER());
                    if (!funcWrapper || !funcWrapper->isValid()) return SharedDllFunction();
                    this->funcList[funcName] = funcWrapper;
                    return funcWrapper;
                }

                template <typename T>
                T getCastedFunction(const std::string &funcName, SharedDllFunction &funcWrapper = SharedDllFunction())
                {
                    // Obtenemos la funcion de interes
                    SharedDllFunction dllFunc = this->getFunction(funcName);
                    if (!dllFunc || !dllFunc->isValid())
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR cargando funcion " << funcName;
                        return nullptr;
                    }
            
                    T funcAddress = reinterpret_cast<T>(dllFunc->getAddress());
                    if (!funcAddress)
                    {
                        LOGGER_THIS_LOG_ERROR() << "ERROR traduciendo funcion " << funcName;
                        return nullptr;
                    }

                    funcWrapper = dllFunc;
                    return funcAddress;
                }

                std::mutex &getUsageMutex()
                {
                    return this->usageMutex;
                }

            // Variables miembro
            public:
                HMODULE           moduleHandle = nullptr;
            private:
                const std::string dllName;
                std::mutex        dllMutex;
    
                FuncList   funcList;
                std::mutex funcMutex;

                std::mutex usageMutex;
        };
    };
};

////////////////////////////////
// Tipos, estructuras y enums //
////////////////////////////////

using SharedDllObject = std::shared_ptr<Utils::Library::DllObject>;
