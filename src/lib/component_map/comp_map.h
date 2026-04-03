#pragma once

#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <variant>
#include <flecs.h>

#include "mem_utils/string_container.h"

namespace flua::cmp_info
{
using GenericComponentPtr = std::variant<int*, unsigned*, float*, double*, std::string*, bool*, flecs::entity*>;

using EntityMemberAccessor = std::function<GenericComponentPtr(flecs::entity)>;

using EntityComponentChecker = std::function<bool(flecs::entity)>;

extern const std::unordered_map<std::string, EntityMemberAccessor> ENTITY_MEMBER_MAP;

extern const std::unordered_map<std::string, EntityComponentChecker> ENTITY_COMPONENT_CHECKERS;
extern const std::map<mem_utils::PointerMappedString, EntityComponentChecker> CACHED_ENTITY_COMPONENT_CHECKERS;

extern std::unordered_map<std::string, ecs_id_t> get_component_ids(const flecs::world&);
std::map<mem_utils::PointerMappedString, ecs_id_t> get_cached_component_ids(const flecs::world&);
}
