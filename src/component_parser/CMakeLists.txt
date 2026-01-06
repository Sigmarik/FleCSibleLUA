add_executable(FluaComponentParser)

target_sources(FluaComponentParser PRIVATE main.cpp)

# Your project include root (keep existing relative include)
target_include_directories(FluaComponentParser PRIVATE ../..)

# Set CLANG_ROOT to the folder that contains 'include' and 'lib' (adjust path as needed)
set(CLANG_ROOT "${CMAKE_LIBRARY_PATH}/clang")

# Add clang headers
target_include_directories(FluaComponentParser PRIVATE
        "${CLANG_ROOT}/include"
)

# Link using full path to the import library (preferred)
set(LIBCLANG_LIB "${CLANG_ROOT}/lib/Windows/Release/libclang.lib")
target_link_libraries(FluaComponentParser PRIVATE "${LIBCLANG_LIB}")

# Ensure the corresponding DLL is deployed next to the executable
set(LIBCLANG_DLL "${CLANG_ROOT}/lib/Windows/Release/libclang.dll")
add_custom_command(TARGET FluaComponentParser POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBCLANG_DLL}"
        $<TARGET_FILE_DIR:FluaComponentParser>
)