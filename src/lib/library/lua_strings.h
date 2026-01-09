#pragma once

#include "flecsible_lua_api.h"

namespace flua::lib::string
{
void tostring(FluaState* state);

void byte(FluaState* state);
void to_char(FluaState* state);
void len(FluaState* state);
void lower(FluaState* state);
void upper(FluaState* state);
void reverse(FluaState* state);
void sub(FluaState* state);
void rep(FluaState* state);
void format(FluaState* state);

void pack(FluaState* state);
void unpack(FluaState* state);
void packsize(FluaState* state);
}
