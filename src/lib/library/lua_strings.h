#pragma once

#include "flecsible_lua_api.h"

namespace flua::lib::string
{
void tostring(FluaState& state); // value -> string

void byte(FluaState& state);    // string, [i], [j] -> byte(s)
void to_char(FluaState& state); // bytes... -> string
void len(FluaState& state);     // string -> length
void lower(FluaState& state);   // string -> lowercased
void upper(FluaState& state);   // string -> uppercased
void reverse(FluaState& state); // string -> reversed
void sub(FluaState& state);     // string, i, [j] -> substring
void rep(FluaState& state);     // string, n -> repeated
void format(FluaState& state);  // format, args... -> formatted

void pack(FluaState& state);     // values... -> binary
void unpack(FluaState& state);   // binary -> values...
void packsize(FluaState& state); // format -> size
}
