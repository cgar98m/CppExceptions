#pragma once

#include <string>

namespace Utils
{
    // Utilidades del sistema de ficheros
    class FileTools
    {
        public:
            static const char *OUTPUT_PATH;

        public:
            static std::string getAbsolutePath(const std::string &relativePath);
    };
};
