#include <unordered_map>
#include <string>
#include <functional>
#include <variant>
#include <flecs.h>

#include "components.h"

namespace flua::cmp_info
{
using GenericComponentPtr = std::variant<int*, unsigned*, float*, double*, std::string*, bool*, flecs::entity*>;

using EntityMemberAccessor = std::function<GenericComponentPtr(flecs::entity)>;

using EntityComponentChecker = std::function<bool(flecs::entity)>;

extern const std::unordered_map<std::string, EntityMemberAccessor> ENTITY_MEMBER_MAP
{
    {"Rotation angle", [](flecs::entity entity)->GenericComponentPtr{ return &entity.get_mut<Rotation>().angle; }},
    {"Position x", [](flecs::entity entity)->GenericComponentPtr{ return &entity.get_mut<Position>().x; }},
    {"Position y", [](flecs::entity entity)->GenericComponentPtr{ return &entity.get_mut<Position>().y; }},
};

extern const std::unordered_map<std::string, EntityComponentChecker> ENTITY_COMPONENT_CHECKERS
{
    {"Rotation", [](flecs::entity entity){ return entity.has<Rotation>(); }},
    {"Position", [](flecs::entity entity){ return entity.has<Position>(); }},
};
}
