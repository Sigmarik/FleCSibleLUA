#include "output.h"

#include <iostream>

#include "ast/data_types.h"
#include "visitors/interpreter/interpreter.h"

namespace flua::lib
{
using namespace flua;

void print(FluaState* lua)
{
    unsigned argCount = lua->getArgumentCount();
    for (unsigned id = 0; id < argCount; ++id)
    {
        lua->m_interpreter->m_outStream << lua->asString(id) << "\t";
    }
    lua->m_interpreter->m_outStream << std::endl;
}
}
