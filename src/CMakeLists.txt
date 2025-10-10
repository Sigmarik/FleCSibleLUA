add_library(FlecsibleLua
		flecsible_lua.cpp
		ast/ast_nodes.cpp
		ast/visitor.cpp
		ast/identification.cpp
		ast/data_types.cpp
		visitors/dbg.cpp
		grammar/lua_lexemes.cpp
		parser/char_pos.cpp
		visitors/interpreter/interpreter.cpp)

if (WINDOWS)
	target_compile_options(FlecsibleLua PRIVATE "/bigobj")
endif ()

target_include_directories(FlecsibleLua PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(FlecsibleLua PRIVATE ${CMAKE_INCLUDE_PATH})

target_include_directories(FlecsibleLua
	PRIVATE ${CMAKE_LIBRARY_PATH}/flecs/include
)

target_link_libraries(FlecsibleLua PRIVATE
	${FLECS_LIBRARIES}
)
