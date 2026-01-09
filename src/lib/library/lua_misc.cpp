#include "lua_misc.h"
#include "visitors/interpreter/interpreter.h"

namespace flua::lib::misc
{
void pcall(FluaState& state)
{
    if (state.getArgumentCount() < 1)
        throw Error("Expected at least 1 argument");

    data::GenericValue* fnc = state.getRaw(0);
    if (!std::holds_alternative<data::Function>(*fnc))
        throw Error("Expected a function");

    data::Function function = std::move(std::get<data::Function>(*fnc));
    std::vector<data::GenericValue> arguments;
    for (unsigned argIdx = 1; argIdx < state.getArgumentCount(); ++argIdx)
    {
        arguments.emplace_back(std::move(*state.getRaw(argIdx)));
    }

    try
    {
        state.m_interpreter->runAnyFunction(function, arguments);
        state.m_interpreter->m_returnedValue.clear();
        state.pushValue(true);
        state.pushNil();
    }
    catch (vst::Interpreter::LuaRuntimeError& err)
    {
        state.pushValue(false);
        if (err.data)
            state.pushRaw(*err.data);
        else
            state.pushValue(err.what);
    }
}

void error(FluaState& state)
{
    vst::Interpreter::LuaRuntimeError error(*state.m_interpreter->m_functionCaller,
                                            "error(" + state.asString(0) + ")");
    error.data = std::move(*state.getRaw(0));
}

void type(FluaState& state)
{
    if (state.getArgumentCount() < 1)
        throw Error("Expected at least 1 argument");
    state.pushValue(data::get_type_name(*state.getRaw(0)));
}
}
