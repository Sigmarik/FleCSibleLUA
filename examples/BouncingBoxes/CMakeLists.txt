include(${CMK_INCLUDE_FOLDER}/flua.cmake)

add_executable(BouncingBoxes)

target_compile_definitions(BouncingBoxes
	PRIVATE
		UNICODE)

set_target_properties(BouncingBoxes PROPERTIES FOLDER "Examples")

SET(SFML_LIBRARIES)

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

	foreach(MODULE system window audio graphics network)
		unset(SFML_DEBUG CACHE)
		find_library(SFML_DEBUG NAMES sfml-${MODULE}-s-d
			PATHS ${CMAKE_LIBRARY_PATH}/sfml/lib/Windows/Debug
			NO_DEFAULT_PATH)
		unset(SFML_RELEASE CACHE)
		find_library(SFML_RELEASE NAMES sfml-${MODULE}-s
			PATHS ${CMAKE_LIBRARY_PATH}/sfml/lib/Windows/Release
			NO_DEFAULT_PATH)
		message("Using ${SFML_DEBUG} (module ${MODULE})")
		set(SFML_LIBRARIES ${SFML_LIBRARIES} ${SFML_DEBUG})
	endforeach()
endif()

target_sources(BouncingBoxes
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/systems.cpp
)

target_flua_components(BouncingBoxes FluaComponentParser components.h)

target_include_directories(BouncingBoxes
	PRIVATE ${CMAKE_LIBRARY_PATH}/sfml/include
	PRIVATE ${CMAKE_LIBRARY_PATH}/flecs/include
	PRIVATE ${CMAKE_INCLUDE_PATH}
)

target_compile_definitions(BouncingBoxes PRIVATE SFML_STATIC)

target_link_libraries(BouncingBoxes PRIVATE
	opengl32 winmm gdi32
	${SFML_LIBRARIES}
	${FLECS_LIBRARIES}

	FlecsibleLua
)
