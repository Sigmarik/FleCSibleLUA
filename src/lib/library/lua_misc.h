#include "flecsible_lua_api.h"

namespace flua::lib::misc
{
void pcall(FluaState& state); // function, [args...] -> status, error
void error(FluaState& state); // value ->

void type(FluaState& state); // value -> type_name
}
