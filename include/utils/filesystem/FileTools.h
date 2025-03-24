#pragma once

#include <windows.h>
#include <string>

namespace Utils
{
    namespace FileSystem
    {
        ////////////////////////////////////////
        // Utilidades del sistema de ficheros //
        ////////////////////////////////////////
        
        class FileTools
        {
            // Constantes
            public:
                static const char *OUTPUT_PATH;
    
            // Funciones de clase
            public:
                static std::string getUndecoratedFileName(const std::string &fileName, bool trimExtension = false);
                static std::string getDirPath(const std::string &fileName);

                static std::string getDirAbsolutePath(const std::string &dirPath, bool createIfNotExists = false);
                
                static std::string getExecutableName(HMODULE module = nullptr, bool trimExtension = false);
                static std::string getExecutablePath(HMODULE module = nullptr);
        };
    };
};
