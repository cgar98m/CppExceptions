#pragma once

#include <windows.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Utils
{
    // Wrapper de la funcion de una DLL
    class DllFunctionWrapper
    {
        private:
            FARPROC           funcAddress = nullptr;
            const std::string funcName;
            std::mutex        funcMutex;

        public:
            DllFunctionWrapper(const std::string& funcName, HMODULE module);
            DllFunctionWrapper(const DllFunctionWrapper&) = delete;
            DllFunctionWrapper operator=(const DllFunctionWrapper&) = delete;
            virtual ~DllFunctionWrapper() = default;

            bool isValid() const;

            FARPROC getAddress() const;
            std::mutex& getMutex();
    };

    // Wrapper de una DLL
    class DllWrapper
    {
        private:
            using FuncList = std::map<std::string, std::shared_ptr<DllFunctionWrapper>>;

            HMODULE           moduleHandle = nullptr;
            const std::string dllName;

            FuncList   funcList;
            std::mutex funcMutex;

        public:
            DllWrapper(const std::string& dllName);
            DllWrapper(const DllWrapper&) = delete;
            DllWrapper operator=(const DllWrapper&) = delete;
            virtual ~DllWrapper();

            bool isValid() const;

            std::shared_ptr<DllFunctionWrapper> getFunction(const std::string& funcName);
            bool deleteFunction(const std::string& funcName);
    };

    // Manejador de librerias DLL dinamicas
    class DllManager
    {
        private:
            using DllList = std::map<std::string, std::shared_ptr<DllWrapper>>;

            static std::unique_ptr<DllManager> instance;
            static std::mutex                  instanceMutex;

            DllList    dllList;
            std::mutex dllMutex;

        public:
            static std::shared_ptr<DllWrapper> getInstance(const std::string& dllName);
            static bool deleteInstance(const std::string& dllName);
            
            DllManager(const DllManager&) = delete;
            DllManager operator=(const DllManager&) = delete;
            virtual ~DllManager() = default;

        private:
            DllManager() = default;

            std::shared_ptr<DllWrapper> getModule(const std::string& dllName);
            bool deleteModule(const std::string& dllName);
    };
};
