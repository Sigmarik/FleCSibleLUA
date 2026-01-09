#include "lua_math.h"

namespace flua::lib
{
void abs(FluaState* state)
{
    if (!state->isNumber(0))
        throw Error("Attempt to call `abs` on a non-numeric value");

    state->pushValue(std::abs(state->getNumber(0)));
}
}

