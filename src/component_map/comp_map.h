#pragma once

#include <unordered_map>
#include <string>
#include <functional>
#include <variant>
#include <flecs.h>

namespace flua::cmp_info
{
using GenericComponentPtr = std::variant<int*, unsigned*, float*, double*, std::string*, bool*, flecs::entity*>;

using EntityMemberAccessor = std::function<GenericComponentPtr(flecs::entity)>;

using EntityComponentChecker = std::function<bool(flecs::entity)>;

extern const std::unordered_map<std::string, EntityMemberAccessor> ENTITY_MEMBER_MAP;

extern const std::unordered_map<std::string, EntityComponentChecker> ENTITY_COMPONENT_CHECKERS;
}
