if (WINDOWS)
    set(BISON_ROOT ${TOOLS_PATH}/win_flex_bison)
    set(BISON_EXECUTABLE ${TOOLS_PATH}/win_flex_bison/win_bison.exe)
    set(FLEX_ROOT ${TOOLS_PATH}/win_flex_bison)
    set(FLEX_EXECUTABLE ${TOOLS_PATH}/win_flex_bison/win_flex.exe)
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# flex_target(FlexSomething path/to/something.l
#     ${CMAKE_CURRENT_SOURCE_DIR}/path_to_something.c
# )

# bison_target(BisonSomething path/to/something.y
#     ${CMAKE_CURRENT_SOURCE_DIR}/path/to/something.c
# DEFINES_FILE
#     ${CMAKE_CURRENT_SOURCE_DIR}/path/to/something.h
# )

add_library(FlecsibleLua)

target_sources(FlecsibleLua PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/flecsible_lua.cpp
)

target_include_directories(FlecsibleLua PRIVATE ${CMAKE_INCLUDE_PATH})
