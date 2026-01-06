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
    {"Position x", [](flecs::entity entity)->GenericComponentPtr{ return &entity.get_mut<ecs::Position>().x; }},
    {"Position y", [](flecs::entity entity)->GenericComponentPtr{ return &entity.get_mut<ecs::Position>().y; }},
    {"Rotation angle", [](flecs::entity entity)->GenericComponentPtr{ return &entity.get_mut<ecs::Rotation>().angle; }}
};

extern const std::unordered_map<std::string, EntityComponentChecker> ENTITY_COMPONENT_CHECKERS
{
    {"Position", [](flecs::entity entity){ return entity.has<ecs::Position>(); }},
    {"Rotation", [](flecs::entity entity){ return entity.has<ecs::Rotation>(); }}
};

extern std::unordered_map<std::string, ecs_id_t> get_component_ids(const flecs::world& world)
{
    std::unordered_map<std::string, ecs_id_t> componentIds;
    componentIds["Position"] = flecs::component<ecs::Position>(world).id();
    componentIds["Rotation"] = flecs::component<ecs::Rotation>(world).id();
    return componentIds;
}
}
