add_executable(RotatingSquare)

target_compile_definitions(RotatingSquare
	PRIVATE
		UNICODE)

set_target_properties(RotatingSquare PROPERTIES FOLDER "Examples")

SET(SFML_LIBRARIES)

if (MSVC)
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

target_sources(RotatingSquare
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/systems.cpp
)

target_include_directories(RotatingSquare
	PRIVATE ${CMAKE_LIBRARY_PATH}/sfml/include
	PRIVATE ${CMAKE_LIBRARY_PATH}/flecs/include
)

target_compile_definitions(RotatingSquare PRIVATE SFML_STATIC)

target_link_libraries(RotatingSquare PRIVATE
	opengl32 winmm gdi32
	${SFML_LIBRARIES}
	${FLECS_LIBRARIES}
)
