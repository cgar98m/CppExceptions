# Establecemos el estandar de CMake minimo
cmake_minimum_required(VERSION 3.31)

# Configuramos el nombre de los ejecutables
set(MAIN_APP_NAME     "CppExceptions")
set(ANALYSIS_APP_NAME "CppExceptionsAnalysis")

# Configuramos el nombre del proyecto y su version
project(${MAIN_APP_NAME} VERSION 1.0)

# Configuramos los defines
if (EXTERNALIZE_DUMPS_DEFINE)
    set(EXTERNALIZE_DUMPS true)
else()
    set(EXTERNALIZE_DUMPS false)
endif()

# Actualizamos los ficheros generados con CMake
configure_file(${PROJECT_SOURCE_DIR}/cmakeInclude/Name.h.in ${PROJECT_SOURCE_DIR}/include/Name.h)
configure_file(${PROJECT_SOURCE_DIR}/cmakeInclude/Config.h.in ${PROJECT_SOURCE_DIR}/include/Config.h)
configure_file(${PROJECT_SOURCE_DIR}/cmakeInclude/Version.h.in ${PROJECT_SOURCE_DIR}/include/Version.h)

# Establecemos el estandar de C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Mostramos las opciones del compilador
message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

# Creamos una lista con los componentes comunes
set(COMMON_SRC
    "${PROJECT_SOURCE_DIR}/src/Version.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/ExceptionManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/ipc/ExceptionPointers.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/ipc/ExceptionRecord.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/ipc/IpcExceptionManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/minidump/MiniDumpInfo.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/exception/minidump/MiniDumpTools.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/filesystem/FileTools.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/library/DllFunction.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/library/DllManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/library/DllObject.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/BasicLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/ConsoleLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/FileLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/ILogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/IThreadedLogger.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/LogEntry.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/LoggerHolder.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/logging/LogLevel.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/argument/ArgumentManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/argument/ArgumentTag.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/argument/ArgumentTypeHolder.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/argument/IntArgumentValue.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/argument/NamedArgumentParser.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/argument/SoloArgumentValue.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/parser/argument/StringArgumentValue.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/text/TextTools.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/thread/ISafeThread.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/thread/IThread.cpp"
    "${PROJECT_SOURCE_DIR}/src/utils/thread/ThreadHolder.cpp")

# Creamos el ejecutable principal
add_executable(${MAIN_APP_NAME}
    "${PROJECT_SOURCE_DIR}/src/CppExceptions.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/MainProgram.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/WorkMode.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/exception/CppExceptionThread.cpp"
    "${PROJECT_SOURCE_DIR}/src/main/exception/SehExceptionThread.cpp"
    ${COMMON_SRC})
target_include_directories(${MAIN_APP_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_link_libraries(${MAIN_APP_NAME} PUBLIC dbghelp)
    
# Creamos el ejecutable secundario
add_executable(${ANALYSIS_APP_NAME}
    "${PROJECT_SOURCE_DIR}/src/CppExceptionsAnalysis.cpp"
    "${PROJECT_SOURCE_DIR}/src/analysis/AnalysisProgram.cpp"
    ${COMMON_SRC})
target_include_directories(${ANALYSIS_APP_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include/")
target_link_libraries(${ANALYSIS_APP_NAME} PUBLIC dbghelp)
