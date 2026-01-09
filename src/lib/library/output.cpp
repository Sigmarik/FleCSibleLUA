#include "output.h"

#include <iostream>

#include "visitors/interpreter/interpreter.h"

namespace flua::lib
{
using namespace flua;

void print(FluaState& state)
{
    unsigned argCount = state.getArgumentCount();
    for (unsigned id = 0; id < argCount; ++id)
    {
        state.m_interpreter->m_outStream << state.asString(id) << "\t";
    }
    state.m_interpreter->m_outStream << std::endl;
}

void lua_assert(FluaState& state)
{
    if (state.getArgumentCount() == 0)
        throw Error("Assertion failed, no arguments received");
    if (!data::to_bool(*state.getRaw(0)))
        throw Error("Assertion failed, expression evaluated to FALSE");
}
}
