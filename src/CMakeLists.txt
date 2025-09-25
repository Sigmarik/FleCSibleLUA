add_library(FlecsibleLua)

target_sources(FlecsibleLua PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/flecsible_lua.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/ast_nodes.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/visitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/identification.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/syntax/data_types.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/visitors/dbg.cpp

		${CMAKE_CURRENT_SOURCE_DIR}/grammar/math.cpp
)

target_include_directories(FlecsibleLua PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(FlecsibleLua PRIVATE ${CMAKE_INCLUDE_PATH})

target_include_directories(FlecsibleLua
	PRIVATE ${CMAKE_LIBRARY_PATH}/flecs/include
)

target_link_libraries(FlecsibleLua PRIVATE
	${FLECS_LIBRARIES}
)
