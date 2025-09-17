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
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/ast_nodes.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/identification.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/data_types.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/visitors/dbg.cpp
)

target_include_directories(FlecsibleLua PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(FlecsibleLua PRIVATE ${CMAKE_INCLUDE_PATH})

if (WINDOWS)
	unset(FLECS_DEBUG CACHE)
	find_library(FLECS_DEBUG NAMES flecs_static
		PATHS ${CMAKE_LIBRARY_PATH}/flecs/lib/Windows/Debug
		NO_DEFAULT_PATH)
	unset(FLECS_RELEASE CACHE)
	find_library(FLECS_RELEASE NAMES flecs_static
		PATHS ${CMAKE_LIBRARY_PATH}/flecs/lib/Windows/Release
		NO_DEFAULT_PATH)
	set(FLECS_LIBRARIES ${FLECS_DEBUG})
endif()

target_include_directories(FlecsibleLua
	PRIVATE ${CMAKE_LIBRARY_PATH}/flecs/include
)

target_link_libraries(FlecsibleLua PRIVATE
	${FLECS_LIBRARIES}
)
