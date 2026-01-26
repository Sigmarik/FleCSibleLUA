#include "library.h"

#include "output.h"
#include "tables.h"
#include "lua_math.h"
#include "lua_strings.h"
#include "lua_misc.h"
#include "ecs.h"

namespace flua::lib
{
const std::unordered_map<std::string, std::function<void(FluaState&)> > STANDARD_LIBRARY
{
    {"print", print},
    {"assert", lua_assert},
    {"ipairs", ipairs},

    {"ecs.find_entity", ecs::find_entity},
    {"ecs.entity_id", ecs::entity_id},
    {"ecs.entity_name", ecs::entity_name},
    {"ecs.entity_path", ecs::entity_path},
    {"ecs.entity_parent", ecs::entity_parent},
    {"ecs.set_entity_parent", ecs::set_entity_parent},
    {"ecs.entity_children", ecs::entity_children},
    {"ecs.destroy_entity", ecs::destroy_entity},
    {"ecs.create_empty_entity", ecs::create_empty_entity},
    {"ecs.clone_entity", ecs::clone_entity},
    {"ecs.delta_time", ecs::delta_time},

    {"pcall", misc::pcall},
    {"error", misc::error},
    {"type", misc::type},

    {"math.abs", abs},

    {"tostring", string::tostring},
    {"string.byte", string::byte},
    {"string.to_char", string::to_char},
    {"string.len", string::len},
    {"string.lower", string::lower},
    {"string.upper", string::upper},
    {"string.reverse", string::reverse},
    {"string.sub", string::sub},
    {"string.rep", string::rep},
    {"string.format", string::format},
    {"string.pack", string::pack},
    {"string.unpack", string::unpack},
    {"string.packsize", string::packsize},
};
}
