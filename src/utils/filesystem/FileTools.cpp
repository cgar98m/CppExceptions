#include "utils/filesystem/FileTools.h"

#include <windows.h>
#include <vector>

namespace Utils
{
    ////////////////////////////////////////
    // Utilidades del sistema de ficheros //
    ////////////////////////////////////////
    
    const std::string FileTools::OUTPUT_PATH = "output";

    std::string FileTools::getAbsolutePath(const std::string& relativePath)
    {
        // Verificamos su existencia
        if (GetFileAttributes(relativePath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            // Creamos el directorio de no existir
            if (!CreateDirectory(relativePath.c_str(), nullptr))
            {
                if (GetLastError() != ERROR_ALREADY_EXISTS) return std::string();
            }
        }

        // Convertimos el path relativo a uno absoluto
        LPSTR             *fileName = nullptr;
        std::vector<char> fullPath(MAX_PATH, 0);
        DWORD pathLength = GetFullPathName(relativePath.c_str(), MAX_PATH, fullPath.data(), fileName);
        if (!pathLength) return std::string();

        // Redimensionamos el path si fuera necesario (una sola vez)
        if (pathLength >= MAX_PATH)
        {
            fullPath.resize(pathLength, 0);
            DWORD newPathLength = GetFullPathName(relativePath.c_str(), pathLength, fullPath.data(), fileName);
            if (!newPathLength || newPathLength >= pathLength) return std::string();

            pathLength = newPathLength;
        }
        fullPath.resize(pathLength);

        return std::string(fullPath.begin(), fullPath.end()) + "\\";    
    }

    bool FileTools::createFile(const std::string& filePath)
    {
        // Verificamos su existencia
        if (GetFileAttributes(filePath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            // Creamos el directorio de no existir
            HANDLE fileHandle = CreateFile(filePath.c_str()
                                         , GENERIC_READ | GENERIC_WRITE
                                         , 0
                                         , nullptr
                                         , CREATE_NEW
                                         , FILE_ATTRIBUTE_NORMAL
                                         , nullptr);
            if (!fileHandle && GetLastError() != ERROR_FILE_EXISTS) return false;
            CloseHandle(fileHandle);
        }

        return true;
    }
};
