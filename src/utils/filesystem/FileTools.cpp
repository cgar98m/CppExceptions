#include "utils/filesystem/FileTools.h"

#include <windows.h>
#include <vector>

namespace Utils
{
    namespace FileSystem
    {
        ////////////////////////////////////////
        // Utilidades del sistema de ficheros //
        ////////////////////////////////////////

        //------------//
        // Constantes //
        //------------//

        const char *FileTools::OUTPUT_PATH = "output";

        //--------------------//
        // Funciones de clase //
        //--------------------//

        std::string FileTools::getUndecoratedFileName(const std::string &fileName, bool trimExtension)
        {
            // Verificamos si tiene contenido
            std::string undecoratedName = fileName;
            if (undecoratedName.empty()) return undecoratedName;
            
            // Buscamos el ultimo separador para quedarnos con el nombre del fichero
            size_t pos = undecoratedName.find_last_of("\\/");
            if (pos != std::string::npos && pos < undecoratedName.size() - 1) undecoratedName = undecoratedName.substr(pos + 1);

            // Quitamos la extension del fichero
            if (trimExtension)
            {
                pos = undecoratedName.find_last_of(".");
                if (pos != std::string::npos && pos < undecoratedName.size() - 1) undecoratedName = undecoratedName.substr(pos + 1);
            }
            return undecoratedName;
        }

        std::string FileTools::getDirPath(const std::string &fileName)
        {
            // Verificamos si tiene contenido
            std::string dirPath = fileName;
            if (dirPath.empty()) return dirPath;
            
            // Buscamos el ultimo separador para quedarnos con el path
            size_t pos = dirPath.find_last_of("\\/");
            if (pos != std::string::npos && pos < dirPath.size() - 1) dirPath = dirPath.substr(0, pos + 1);
            return dirPath;
        }

        std::string FileTools::getDirAbsolutePath(const std::string &dirPath, bool createIfNotExists)
        {
            // Verificamos su existencia
            if (GetFileAttributes(dirPath.c_str()) == INVALID_FILE_ATTRIBUTES)
            {
                if (!createIfNotExists) return std::string();

                // Creamos el directorio de no existir
                if (!CreateDirectory(dirPath.c_str(), nullptr))
                {
                    if (GetLastError() != ERROR_ALREADY_EXISTS) return std::string();
                }
            }

            // Convertimos el path relativo a uno absoluto
            LPSTR             *fileName = nullptr;
            std::vector<char> fullPath(MAX_PATH, 0);
            DWORD pathLength = GetFullPathName(dirPath.c_str(), MAX_PATH, fullPath.data(), fileName);
            if (!pathLength) return std::string();

            // Redimensionamos el path si fuera necesario (una sola vez)
            if (pathLength >= MAX_PATH)
            {
                fullPath.resize(pathLength, 0);
                DWORD newPathLength = GetFullPathName(dirPath.c_str(), pathLength, fullPath.data(), fileName);
                if (!newPathLength || newPathLength >= pathLength) return std::string();

                pathLength = newPathLength;
            }
            fullPath.resize(pathLength);

            return std::string(fullPath.begin(), fullPath.end()) + "\\";    
        }

        std::string FileTools::getExecutableName(HMODULE module, bool trimExtension)
        {
            std::vector<char> exePath(MAX_PATH, 0);

            // Obtenemos el path del ejecutable
            DWORD pathLength = GetModuleFileName(module, exePath.data(), MAX_PATH);
            if (!pathLength) return std::string();
            exePath.resize(pathLength);

            // Recortamos el nombre del ejecutable
            return getUndecoratedFileName(std::string(exePath.begin(), exePath.end()), trimExtension);
        }

        std::string FileTools::getExecutablePath(HMODULE module)
        {
            std::vector<char> exePath(MAX_PATH, 0);

            // Obtenemos el path del ejecutable
            DWORD pathLength = GetModuleFileName(module, exePath.data(), MAX_PATH);
            if (!pathLength) return std::string();
            exePath.resize(pathLength);

            // Recortamos el nombre del ejecutable
            return getDirPath(std::string(exePath.begin(), exePath.end()));
        }
    };
};
