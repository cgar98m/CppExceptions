#pragma once

#include <windows.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "logger/ILogger.h"

namespace Utils
{
    // Wrapper de la funcion de una DLL
    class DllFunctionWrapper: public Logger::ILoggerHolder
    {
        private:
            FARPROC           funcAddress = nullptr;
            const std::string funcName;
            std::mutex        funcMutex;

        public:
            DllFunctionWrapper() = delete;
            DllFunctionWrapper(const std::string& funcName, HMODULE module, const Logger::Logger& logger = Logger::BasicLogger::getInstance());
            DllFunctionWrapper(const DllFunctionWrapper&) = delete;
            DllFunctionWrapper operator=(const DllFunctionWrapper&) = delete;
            virtual ~DllFunctionWrapper() = default;

            bool isValid() const;

            FARPROC getAddress() const;
            std::mutex& getMutex();
    };

    // Wrapper de una DLL
    class DllWrapper: public Logger::ILoggerHolder
    {
        private:
            using FuncList = std::map<std::string, std::shared_ptr<DllFunctionWrapper>>;

            HMODULE           moduleHandle = nullptr;
            const std::string dllName;

            FuncList   funcList;
            std::mutex funcMutex;

        public:
            DllWrapper() = delete;
            DllWrapper(const std::string& dllName, const Logger::Logger& logger = Logger::BasicLogger::getInstance());
            DllWrapper(const DllWrapper&) = delete;
            DllWrapper operator=(const DllWrapper&) = delete;
            virtual ~DllWrapper();

            bool isValid() const;

            std::shared_ptr<DllFunctionWrapper> getFunction(const std::string& funcName);
            bool deleteFunction(const std::string& funcName);
    };

    // Manejador de librerias DLL dinamicas
    class DllManager: public Logger::ILoggerHolder
    {
        private:
            using DllList = std::map<std::string, std::shared_ptr<DllWrapper>>;

            static std::unique_ptr<DllManager> instance;
            static std::mutex                  instanceMutex;

            DllList    dllList;
            std::mutex dllMutex;

        public:
            static std::shared_ptr<DllWrapper> getInstance(const std::string& dllName, const Logger::Logger& logger = Logger::BasicLogger::getInstance());
            static bool deleteInstance(const std::string& dllName);
            
            DllManager(const DllManager&) = delete;
            DllManager operator=(const DllManager&) = delete;
            virtual ~DllManager() = default;

        private:
            DllManager(const Logger::Logger& logger = Logger::BasicLogger::getInstance());

            std::shared_ptr<DllWrapper> getModule(const std::string& dllName);
            bool deleteModule(const std::string& dllName);
    };
};
