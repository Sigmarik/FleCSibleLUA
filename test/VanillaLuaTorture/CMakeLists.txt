include(${CMK_INCLUDE_FOLDER}/flua.cmake)

add_executable(test_VanillaLuaTorture)

target_compile_definitions(test_VanillaLuaTorture
	PRIVATE
		UNICODE)

set_target_properties(test_VanillaLuaTorture PROPERTIES FOLDER "Test")

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

target_sources(test_VanillaLuaTorture
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories(test_VanillaLuaTorture
	PRIVATE ${CMAKE_LIBRARY_PATH}/flecs/include
	PRIVATE ${CMAKE_INCLUDE_PATH}
)

target_flua_components(test_VanillaLuaTorture)

target_compile_definitions(test_VanillaLuaTorture PRIVATE SFML_STATIC)

target_link_libraries(test_VanillaLuaTorture PRIVATE
	opengl32 winmm gdi32
	${FLECS_LIBRARIES}

	FlecsibleLua
)
