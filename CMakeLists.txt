# Establecemos el estándar de CMake mínimo
cmake_minimum_required(VERSION 3.31)

# Configuramos el nombre del proyecto y su versión
project(CppExceptions VERSION 0.4)
configure_file(${PROJECT_SOURCE_DIR}/cmake/Version.h.in ${PROJECT_SOURCE_DIR}/include/Version.h)

# Creamos el ejecutable
add_executable(CppExceptions
    "${PROJECT_SOURCE_DIR}/src/main.cpp"
    "${PROJECT_SOURCE_DIR}/src/error/Exception.cpp"
    "${PROJECT_SOURCE_DIR}/src/logger/ConsoleLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/logger/ILogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/parser/NamedArgumentParser.cpp"
    "${PROJECT_SOURCE_DIR}/src/parser/ParsedArgument.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/DllManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Program.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Thread.cpp")

# Establecemos el estándar de C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Modificamos las opciones del compilador y las mostramos
# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

# Indicamos los ficheros a incluir
target_include_directories(CppExceptions PUBLIC "${PROJECT_SOURCE_DIR}/include/")

# Incluimos las librerias de dependencias
target_link_libraries(CppExceptions PUBLIC dbghelp)
