#include "ecs.h"

#include <flecs.h>

namespace flua::lib
{
void id2entity(FluaState& state)
{
    if (!state.isNumber(0))
        throw Error("Attempt to get an entity by non-numeric ID");

    flecs::entity_t id = static_cast<flecs::entity_t>(state.getNumber(0));
    flecs::entity entity(*state.getWorld(), id);
    if (ecs_is_valid(state.getWorld()->c_ptr(), id))
    {
        state.pushValue(entity);
    }
}

void entity2id(FluaState& state)
{
    if (!state.isEntity(0))
        throw Error("Attempt to get ID of a non-entity object");

    state.pushValue(static_cast<double>(state.getEntity(0).id()));
}

void getDeltaTime(FluaState& state)
{
    state.pushValue(state.getWorld()->delta_time());
}
}
