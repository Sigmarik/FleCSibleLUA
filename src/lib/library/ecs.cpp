#include "ecs.h"

#include <flecs.h>
#include <optional>
#include <set>

#include "ecs/guarded_iterator.h"

namespace flua::lib::ecs
{
void find_entity(FluaState& state)
{
    if (state.getArgumentCount() < 1)
        throw Error("Expected 1 argument");

    if (state.isNumber(0))
    {
        flecs::entity_t id = static_cast<flecs::entity_t>(state.getNumber(0));
        flecs::entity entity(*state.getWorld(), id);
        if (ecs_is_valid(state.getWorld()->c_ptr(), id))
        {
            state.pushValue(entity);
        }
    }
    else if (state.isString(0))
    {
        std::string name = state.getString(0);
        flecs::entity found = state.getWorld()->lookup(name.c_str());
        if (found) state.pushValue(found);
        else state.pushNil();
    }
    else
    {
        throw Error("Expected a path or an id");
    }
}

void entity_id(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Expected 1 argument of type `entity`");

    flecs::entity entity = state.getEntity(0);
    state.pushValue(static_cast<double>(entity.id()));
}

void entity_name(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Expected 1 argument of type `entity`");

    flecs::entity entity = state.getEntity(0);
    flecs::string name = entity.name();
    if (name.length() == 0) state.pushNil();
    else state.pushValue(std::string(name));
}

void entity_path(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Expected 1 argument of type `entity`");

    flecs::entity entity = state.getEntity(0);
    state.pushValue(std::string(entity.path()));
}

void entity_parent(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Expected 1 argument of type `entity`");

    flecs::entity entity = state.getEntity(0);
    flecs::entity parent = entity.parent();
    if (parent.id() != 0) state.pushValue(parent);
    else state.pushNil();
}

void set_entity_parent(FluaState& state)
{
    if (!state.isEntity(0) || !state.isEntity(1))
        throw Error("Expected 2 arguments of type `entity`");
    flecs::entity child = state.getEntity(0);
    flecs::entity parent = state.getEntity(1);
    flecs::entity superParent = parent;
    std::set<flecs::entity_t> visitedEntities{child.id()};
    while (superParent)
    {
        if (visitedEntities.contains(superParent.id())) throw Error("Parenthood cycle detected");
        superParent = superParent.parent();
        visitedEntities.insert(superParent.id());
    }
    child.child_of(parent);
}

struct ChildrenIterator
{
    flua::ecs::GuardedEcsIterator iter;
    long long int internalIdx = 0;

    explicit ChildrenIterator(const ecs_iter_t& iterator) : iter(iterator) {}

    void operator()(FluaState& state)
    {
        while (internalIdx >= iter->count)
        {
            if (!ecs_children_next(&*iter))
            {
                state.pushNil();
                return;
            }
            internalIdx = 0;
        }
        flecs::entity entity(*state.getWorld(), iter->entities[internalIdx]);
        state.pushValue(entity);
        ++internalIdx;
    }
};

void entity_children(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Expected 1 argument of type `entity`");

    flecs::entity entity = state.getEntity(0);
    ChildrenIterator iterator(ecs_children(state.getWorld()->c_ptr(), entity.id()));
    state.pushValue(iterator);
}

void destroy_entity(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Expected 1 argument of type `entity`");

    state.getEntity(0).destruct();
}

void create_empty_entity(FluaState& state)
{
    std::optional<std::string> name = std::nullopt;
    if (state.getArgumentCount() > 0)
        name = state.asString(0);

    if (name.has_value() && name->empty())
        throw Error("New entity name cannot be empty");

    if (name.has_value())
    {
        flecs::entity found = state.getWorld()->lookup(name->c_str());
        if (found && found.is_alive())
            throw Error("Cannot clone to an existing entity");
    }

    state.pushValue(name ? state.getWorld()->entity(name->c_str()) : state.getWorld()->entity());
}

void clone_entity(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Expected 1 argument of type `entity`");
    std::optional<std::string> name = std::nullopt;
    if (state.getArgumentCount() > 1)
        name = state.asString(1);

    if (name.has_value() && name->empty())
        throw Error("Clone target entity name cannot be empty");

    if (name.has_value())
    {
        flecs::entity found = state.getWorld()->lookup(name->c_str());
        if (found && found.is_alive())
            throw Error("Cannot clone to an existing entity");
    }

    flecs::entity newEntity = state.getWorld()->entity();
    ecs_clone(state.getWorld()->c_ptr(), newEntity.id(), state.getEntity(0).id(), true);
    if (name.has_value()) newEntity.set_name(name->c_str());
    state.pushValue(newEntity);
}

void delta_time(FluaState& state)
{
    state.pushValue(state.getWorld()->delta_time());
}
}
