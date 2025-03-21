# Establecemos el estandar de CMake minimo
cmake_minimum_required(VERSION 3.31)

# Configuramos el nombre del proyecto y su version
project(CppExceptions VERSION 0.6)

# Configuramos los defines
if (EXTERNALIZE_DUMPS_DEFINE)
    set(EXTERNALIZE_DUMPS true)
else()
    set(EXTERNALIZE_DUMPS false)
endif()

# Actualizamos los ficheros generados con CMake
configure_file(${PROJECT_SOURCE_DIR}/cmakeInclude/Config.h.in ${PROJECT_SOURCE_DIR}/include/Config.h)
configure_file(${PROJECT_SOURCE_DIR}/cmakeInclude/Version.h.in ${PROJECT_SOURCE_DIR}/include/Version.h)

# Establecemos el estandar de C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Mostramos las opciones del compilador
message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

# Creamos el ejecutable principal
add_executable(CppExceptions
    "${PROJECT_SOURCE_DIR}/src/CppExceptions.cpp"
    "${PROJECT_SOURCE_DIR}/src/error/Exception.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Program.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Thread.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/MsvcException.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/filesystem/FileTools.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/library/DllManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/BasicLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/ConsoleLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/FileLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/ILogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/IThreadedLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/LogTypes.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/NamedArgumentParser.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/ParsedArgument.cpp")
target_include_directories(CppExceptions PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_link_libraries(CppExceptions PUBLIC dbghelp)

# Creamos el ejecutable secundario
add_executable(CppExceptionsAnalysis
    "${PROJECT_SOURCE_DIR}/src/CppExceptionsAnalysis.cpp"
    "${PROJECT_SOURCE_DIR}/src/error/Exception.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/Thread.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/MsvcException.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/filesystem/FileTools.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/library/DllManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/BasicLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/ConsoleLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/FileLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/ILogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/IThreadedLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/LogTypes.cpp")
    target_include_directories(CppExceptionsAnalysis PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_link_libraries(CppExceptionsAnalysis PUBLIC dbghelp)
