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
    {"ipairs", ipairs},
    {"id2entity", id2entity},
    {"entity2id", entity2id},
    {"deltaTime", getDeltaTime},

    {"math.abs", abs},
};
}
