#include "library.h"

#include "output.h"
#include "tables.h"
#include "lua_math.h"
#include "lua_strings.h"
#include "lua_misc.h"
#include "ecs.h"

namespace flua::lib
{
const std::unordered_map<std::string, LibraryElement> STANDARD_LIBRARY
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

    {"math.abs", math::abs},
    {"math.acos", math::acos},
    {"math.asin", math::asin},
    {"math.atan", math::atan},
    {"math.atan2", math::atan2},
    {"math.ceil", math::ceil},
    {"math.cos", math::cos},
    {"math.cosh", math::cosh},
    {"math.deg", math::deg},
    {"math.rad", math::rad},
    {"math.exp", math::exp},
    {"math.floor", math::floor},
    {"math.fmod", math::fmod},
    {"math.frexp", math::frexp},
    {"math.ldexp", math::ldexp},
    {"math.log", math::log},
    {"math.log10", math::log10},
    {"math.max", math::max},
    {"math.min", math::min},
    {"math.modf", math::modf},
    {"math.pow", math::pow},
    {"math.random", math::random},
    {"math.randomseed", math::randomseed},
    {"math.sin", math::sin},
    {"math.sinh", math::sinh},
    {"math.sqrt", math::sqrt},
    {"math.tan", math::tan},
    {"math.tanh", math::tanh},

    {"math.lerp", math::lerp},
    {"math.lerpb", math::lerpb},
    {"math.clamp", math::clamp},

    {"math.dot", math::dot},
    {"math.cross", math::cross},
    {"math.length", math::length},
    {"math.length2", math::length2},
    {"math.length_man", math::length_man},
    {"math.dist", math::dist},
    {"math.dist2", math::dist2},
    {"math.dist_man", math::dist_man},
    {"math.normalize", math::normalize},

    {"vec", math::vec},

    {"math.huge", math::kHuge},
    {"math.pi", math::kPi},
    {"math.e", math::kE},

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
