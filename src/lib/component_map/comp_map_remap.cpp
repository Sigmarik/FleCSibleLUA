#include <map>

#include "comp_map.h"
#include "meta/remap.h"

namespace flua::cmp_info
{

extern const std::map<mem_utils::PointerMappedString, EntityComponentChecker> CACHED_ENTITY_COMPONENT_CHECKERS =
    meta::remapHashToCache<mem_utils::PointerMappedString>(ENTITY_COMPONENT_CHECKERS);

std::map<mem_utils::PointerMappedString, ecs_id_t> get_cached_component_ids(const flecs::world& world)
{
    return meta::remapHashToCache<mem_utils::PointerMappedString>(get_component_ids(world));
}

}
