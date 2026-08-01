#include "flecsible_lua_api.h"

namespace flua::lib
{
void pairs(FluaState& state); // table -> key_value_pair_iterator
void ipairs(FluaState& state); // table -> key_value_pair_iterator
}
