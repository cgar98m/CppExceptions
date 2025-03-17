#include "utils/library/DllManager.h"

namespace Utils
{
    //////////////////////////////////////
    // Wrapper de la funcion de una DLL //
    //////////////////////////////////////

    DllFunctionWrapper::DllFunctionWrapper(const std::string& funcName, HMODULE module, const Logger::Logger& logger)
        : ILoggerHolder(logger)
        , funcName(funcName)
    {
        if (funcName.empty() || !module) return;

        funcAddress = GetProcAddress(module, funcName.c_str());
        if (!funcAddress) LOGGER_THIS_LOG() << "Error obteniendo direccion de " << funcName << ": " << GetLastError();
    }

    bool DllFunctionWrapper::isValid() const
    {
        return funcAddress;
    }

    FARPROC DllFunctionWrapper::getAddress() const
    {
        return funcAddress;
    }

    std::mutex& DllFunctionWrapper::getMutex()
    {
        return funcMutex;
    }

    ////////////////////////
    // Wrapper de una DLL //
    ////////////////////////
    
    DllWrapper::DllWrapper(const std::string& dllName, const Logger::Logger& logger)
        : ILoggerHolder(logger)
        , dllName(dllName)
    {
        if (dllName.empty()) return;

        moduleHandle = LoadLibraryA(dllName.c_str());
        if (!moduleHandle) LOGGER_THIS_LOG() << "Error cargando DLL " << dllName << ": " << GetLastError();
    }

    DllWrapper::~DllWrapper()
    {
        if (!moduleHandle) return;
        if (!FreeLibrary(moduleHandle)) LOGGER_THIS_LOG() << "Error liberando DLL " << dllName << ": " << GetLastError();
    }

    bool DllWrapper::isValid() const
    {
        return moduleHandle;
    }

    std::shared_ptr<DllFunctionWrapper> DllWrapper::getFunction(const std::string& funcName)
    {
        if (!moduleHandle) return std::shared_ptr<DllFunctionWrapper>();

        std::lock_guard<std::mutex> lock(funcMutex);

        // Verificamos la validez de la funcion
        if (funcName.empty()) return std::shared_ptr<DllFunctionWrapper>();

        // Comprobamos su existencia
        if (funcList.find(funcName) != funcList.end()) return funcList[funcName];

        // Buscamos la funcion
        std::shared_ptr<DllFunctionWrapper> funcWrapper = std::make_shared<DllFunctionWrapper>(funcName, moduleHandle, getLogger());
        if (!funcWrapper || !funcWrapper->isValid()) return std::shared_ptr<DllFunctionWrapper>();
        funcList[funcName] = funcWrapper;
        return funcWrapper;
    }

    bool DllWrapper::deleteFunction(const std::string& funcName)
    {
        std::lock_guard<std::mutex> lock(funcMutex);

        // Verificamos la validez de la dll
        if (funcName.empty()) return false;

        // Comprobamos su existencia
        auto funcIt = funcList.find(funcName);
        if (funcIt == funcList.end()) return false;

        // Eliminamos el modulo
        funcList.erase(funcIt);
        return true;
    }

    //////////////////////////////////////////
    // Manejador de librerias DLL dinamicas //
    //////////////////////////////////////////
    
    std::unique_ptr<DllManager> DllManager::instance;
    std::mutex                  DllManager::instanceMutex;

    std::shared_ptr<DllWrapper> DllManager::getInstance(const std::string& dllName, const Logger::Logger& logger)
    {
        std::lock_guard<std::mutex> lock(instanceMutex);

        // Obtenemos la instancia gestora
        if (!instance) instance.reset(new DllManager(logger));
        if (!instance) return std::shared_ptr<DllWrapper>();

        // Obtenemos el modulo solicitado
        return instance->getModule(dllName);
    }

    bool DllManager::deleteInstance(const std::string& dllName)
    {
        std::lock_guard<std::mutex> lock(instanceMutex);

        // Obtenemos la instancia gestora
        if (!instance) return false;

        // Descargamos el modulo
        return instance->deleteModule(dllName);
    }

    DllManager::DllManager(const Logger::Logger& logger)
        : ILoggerHolder(logger)
    {
    }

    std::shared_ptr<DllWrapper> DllManager::getModule(const std::string& dllName)
    {
        std::lock_guard<std::mutex> lock(dllMutex);

        // Verificamos la validez de la dll
        if (dllName.empty()) return std::shared_ptr<DllWrapper>();

        // Comprobamos su existencia
        if (dllList.find(dllName) != dllList.end()) return dllList[dllName];

        // Creamos el modulo
        std::shared_ptr<DllWrapper> dllWrapper = std::make_shared<DllWrapper>(dllName, getLogger());
        if (!dllWrapper || !dllWrapper->isValid()) return std::shared_ptr<DllWrapper>();
        dllList[dllName] = dllWrapper;
        return dllWrapper;
    }

    bool DllManager::deleteModule(const std::string& dllName)
    {
        std::lock_guard<std::mutex> lock(dllMutex);

        // Verificamos la validez de la dll
        if (dllName.empty()) return false;

        // Comprobamos su existencia
        auto dllIt = dllList.find(dllName);
        if (dllIt == dllList.end()) return false;

        // Eliminamos el modulo
        dllList.erase(dllIt);
        return true;
    }
};
