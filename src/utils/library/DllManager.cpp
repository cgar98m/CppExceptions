#include "utils/library/DllManager.h"

#include "utils/text/TextTools.h"

namespace Utils
{
    namespace Library
    {
        //////////////////////////////////////////
        // Manejador de librerias DLL dinamicas //
        //////////////////////////////////////////
        
        //------------------------//
        // Constructor/Destructor //
        //------------------------//

        DllManager::DllManager(const SharedLogger &logger)
            : Logging::LoggerHolder(logger)
        {
        }

        //--------------------//
        // Funciones de clase //
        //--------------------//
    
        SharedDllObject DllManager::getInstance(const std::string &dllName, const SharedLogger &logger)
        {
            std::lock_guard<std::mutex> lock(instanceMutex);
    
            // Obtenemos la instancia gestora
            if (!instance) instance.reset(new DllManager(logger));
            
            // Obtenemos el modulo solicitado
            if (!instance) return SharedDllObject();
            return instance->getModule(dllName);
        }
    
        bool DllManager::deleteInstance(const std::string &dllName)
        {
            std::lock_guard<std::mutex> lock(instanceMutex);
    
            // Obtenemos la instancia gestora
            if (!instance) return true;
    
            // Descargamos el modulo
            return instance->deleteModule(dllName);
        }

        //--------------------//
        // Funciones miembro  //
        //--------------------//

        SharedDllObject DllManager::getModule(const std::string &dllName)
        {
            SharedDllObject             dllWrapper;
            std::lock_guard<std::mutex> lock(this->dllMutex);
    
            // Verificamos la validez de la dll
            std::string uniqueDllName = getUniqueDllName(dllName);
            if (uniqueDllName.empty()) return dllWrapper;
    
            // Comprobamos su existencia
            if (this->dllList.find(uniqueDllName) != this->dllList.end()) return this->dllList[uniqueDllName];
    
            // Creamos el modulo
            dllWrapper = std::make_shared<DllObject>(uniqueDllName, THIS_LOGGER());
            if (!dllWrapper || !dllWrapper->isValid()) return SharedDllObject();
            this->dllList[uniqueDllName] = dllWrapper;
            return dllWrapper;
        }
    
        bool DllManager::deleteModule(const std::string &dllName)
        {
            std::lock_guard<std::mutex> lock(this->dllMutex);
    
            // Verificamos la validez de la dll
            std::string uniqueDllName = getUniqueDllName(dllName);
            if (uniqueDllName.empty()) return true;
    
            // Comprobamos su existencia
            auto dllIt = this->dllList.find(uniqueDllName);
            if (dllIt == this->dllList.end()) return true;
    
            // Eliminamos el modulo
            this->dllList.erase(dllIt);
            return true;
        }

        std::string DllManager::getUniqueDllName(const std::string &dllName)
        {
            // Pasamos el nombre a minusculas
            return Utils::Text::TextTools::toLowerCase(dllName);
        }

        //--------------------//
        // Variables de clase //
        //--------------------//

        DllManager::UniqueDllManager DllManager::instance;
        std::mutex                   DllManager::instanceMutex;
    };
};
