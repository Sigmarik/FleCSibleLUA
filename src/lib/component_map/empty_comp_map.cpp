#include "comp_map.h"

namespace flua::cmp_info
{
extern const std::unordered_map<std::string, EntityMemberAccessor> ENTITY_MEMBER_MAP{};

extern const std::unordered_map<std::string, EntityComponentChecker> ENTITY_COMPONENT_CHECKERS{};

extern std::unordered_map<std::string, ecs_id_t> get_component_ids(const flecs::world&)
{
    return {};
}
}
