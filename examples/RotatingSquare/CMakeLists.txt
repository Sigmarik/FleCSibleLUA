add_executable(RotatingSquare)

target_compile_definitions(RotatingSquare
	PRIVATE
		UNICODE)

set_target_properties(RotatingSquare PROPERTIES FOLDER "Examples")

SET(SFML_LIBRARIES)

if (MSVC)
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
)

target_include_directories(RotatingSquare
	PRIVATE ${CMAKE_LIBRARY_PATH}/sfml/include
)

target_compile_definitions(RotatingSquare PRIVATE SFML_STATIC)

link_directories(${CMAKE_LIBRARY_PATH}/sfml/lib/Windows/Debug)

target_link_libraries(RotatingSquare PRIVATE
	opengl32 winmm gdi32
	${SFML_LIBRARIES}
)
