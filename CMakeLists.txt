cmake_minimum_required(VERSION 3.10)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    option(WINDOWS "Defines whether the script is running on the most ventilated OS on the planet" ON)
endif()

cmake_policy(SET CMP0074 NEW)

project(FleCSibleLUA VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(BUILD ${CMAKE_SOURCE_DIR}/build)

set(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(TOOLS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD}/runtime)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD}/library)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD}/archive)

add_subdirectory(src)

add_subdirectory(examples)
