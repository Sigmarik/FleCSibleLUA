cmake_minimum_required(VERSION 3.10)

project(FleCSibleLUA VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(BUILD ${CMAKE_SOURCE_DIR}/build)

set(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD}/runtime)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD}/library)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD}/archive)

add_executable(Interpreter ${SRC}/main.cpp)

if(MSVC)
    target_compile_options(Interpreter PRIVATE /W4)
endif()

add_subdirectory(examples)
