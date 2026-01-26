#include "flecsible_lua_api.h"

namespace flua::lib::ecs
{
void find_entity(FluaState& state);  // id -> entity

void entity_id(FluaState& state);  // entity -> id
void entity_name(FluaState& state);  // entity -> name
void entity_path(FluaState& state);  // entity -> name
void entity_parent(FluaState& state);  // entity -> entity

void set_entity_parent(FluaState& state);  // child, parent

void entity_children(FluaState& state);  // entity -> children_it

void destroy_entity(FluaState& state);  // entity ->

void create_empty_entity(FluaState& state);  // [opt]name -> entity
void clone_entity(FluaState& state);  // entity, [opt]name -> clonedEntity

void delta_time(FluaState& state);  // -> dt
}
