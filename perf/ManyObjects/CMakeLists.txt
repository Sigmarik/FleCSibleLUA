include(${CMK_INCLUDE_FOLDER}/flua.cmake)

add_executable(perf_ManyObjects)

target_compile_definitions(perf_ManyObjects
	PRIVATE
		UNICODE)

set_target_properties(perf_ManyObjects PROPERTIES FOLDER "Perf")

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

target_sources(perf_ManyObjects
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories(perf_ManyObjects
	PRIVATE ${CMAKE_LIBRARY_PATH}/flecs/include
	PRIVATE ${CMAKE_INCLUDE_PATH}
)

target_flua_components(perf_ManyObjects FluaComponentParser components.h)

target_compile_definitions(perf_ManyObjects PRIVATE SFML_STATIC)

target_link_libraries(perf_ManyObjects PRIVATE
	opengl32 winmm gdi32
	${FLECS_LIBRARIES}

	FlecsibleLua
)
