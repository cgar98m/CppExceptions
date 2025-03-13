#pragma once

#include <string>

namespace Utils
{
    // Utilidades del sistema de ficheros
    class FileSystem
    {
        public:
            static const std::string OUTPUT_PATH;

        public:
            static std::string getAbsolutePath(const std::string& relativePath);
            static bool createFile(const std::string& filePath);
    };
};
