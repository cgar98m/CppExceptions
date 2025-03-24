#include "utils/library/DllObject.h"

#include "utils/logging/LogEntry.h"

namespace Utils
{
    namespace Library
    {
        ////////////////////////
        // Wrapper de una DLL //
        ////////////////////////
        
        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        DllObject::DllObject(const std::string &dllName, const SharedLogger &logger)
            : Logging::LoggerHolder(logger)
            , dllName(dllName)
        {
            if (this->dllName.empty()) return;
    
            // Cargamos la DLL en memoria
            this->moduleHandle = LoadLibraryA(this->dllName.c_str());
            if (!this->moduleHandle) LOGGER_THIS_LOG_ERROR() << "ERROR cargando DLL " << this->dllName << ": " << GetLastError();
        }
    
        DllObject::~DllObject()
        {
            // Descargamos la DLL de la memoria
            std::lock_guard<std::mutex> lock(this->dllMutex);
            if (this->moduleHandle)
            {
                if (!FreeLibrary(this->moduleHandle)) LOGGER_THIS_LOG_WARNING() << "ERROR liberando DLL " << this->dllName << ": " << GetLastError();
                this->moduleHandle = nullptr;
            }
        }
    
        //--------------------//
        // Funciones miembro  //
        //--------------------//

        bool DllObject::isValid()
        {
            std::lock_guard<std::mutex> lock(this->dllMutex);
            return this->moduleHandle;
        }
    
        SharedDllFunction DllObject::getFunction(const std::string &funcName)
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
    
        bool DllObject::deleteFunction(const std::string &funcName)
        {
            std::lock_guard<std::mutex> lock(this->funcMutex);
    
            // Verificamos la validez de la dll
            if (funcName.empty()) return false;
    
            // Comprobamos su existencia
            auto funcIt = this->funcList.find(funcName);
            if (funcIt == this->funcList.end()) return false;
    
            // Eliminamos la funcion de la lista
            this->funcList.erase(funcIt);
            return true;
        }
    };
};
