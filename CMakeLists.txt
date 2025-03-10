# Establecemos el estandar de CMake minimo
cmake_minimum_required(VERSION 3.31)

# Configuramos el nombre del proyecto y su version
project(CppExceptions VERSION 0.6)

# Configuramos defines
if (EXTERNALIZE_DUMPS_DEFINE)
    set(EXTERNALIZE_DUMPS true)
else()
    set(EXTERNALIZE_DUMPS false)
endif()

# Actualizamos fichero de defines
configure_file(${PROJECT_SOURCE_DIR}/cmake/CMakeDefine.h.in ${PROJECT_SOURCE_DIR}/include/CMakeDefine.h)

# Establecemos el estandar de C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Mostramos las opciones del compilador
message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

# Creamos el ejecutable principal
add_executable(CppExceptions
    "${PROJECT_SOURCE_DIR}/src/CppExceptions.cpp"
    "${PROJECT_SOURCE_DIR}/src/error/Exception.cpp"
    "${PROJECT_SOURCE_DIR}/src/error/MsvcException.cpp"
    "${PROJECT_SOURCE_DIR}/src/logger/ConsoleLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/logger/ILogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/parser/NamedArgumentParser.cpp"
    "${PROJECT_SOURCE_DIR}/src/parser/ParsedArgument.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/DllManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Program.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Thread.cpp")
target_include_directories(CppExceptions PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_link_libraries(CppExceptions PUBLIC dbghelp)

# Creamos el ejecutable secundario
add_executable(CppExceptionsAnalysis
    "${PROJECT_SOURCE_DIR}/src/CppExceptionsAnalysis.cpp"
    "${PROJECT_SOURCE_DIR}/src/error/Exception.cpp"
    "${PROJECT_SOURCE_DIR}/src/error/MsvcException.cpp"
    "${PROJECT_SOURCE_DIR}/src/logger/ConsoleLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/logger/ILogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/DllManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Thread.cpp")
target_include_directories(CppExceptionsAnalysis PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_link_libraries(CppExceptionsAnalysis PUBLIC dbghelp)
