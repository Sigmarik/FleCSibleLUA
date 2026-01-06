#include "identification.h"

namespace flua::ids
{

void ResolvableName::resolveNew()
{
    thread_local IdT sGlobalId = 0;
    id = ++sGlobalId;
};

void ResolvableName::resolveAs(const ResolvableName& name)
{
    id = name.id;
}

void ResolvableName::resolveAs(IdT id)
{
    this->id = id;
}

}
