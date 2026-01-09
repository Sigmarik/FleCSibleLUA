#include "library.h"

#include "output.h"
#include "tables.h"
#include "lua_math.h"
#include "ecs.h"

namespace flua::lib
{
const std::unordered_map<std::string, std::function<void(FluaState*)> > STANDARD_LIBRARY
{
    {"print", print},
    {"assert", lua_assert},
    {"pcall", misc::pcall},
    {"error", misc::error},
    {"ipairs", ipairs},
    {"ecs.id2entity", id2entity},
    {"ecs.entity2id", entity2id},
    {"ecs.deltaTime", getDeltaTime},

    {"math.abs", abs},
};
}
