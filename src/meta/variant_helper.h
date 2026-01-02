#pragma once

namespace flua::meta
{

// helper type for the visitor
template<class... Ts>
struct Overloads : Ts... { using Ts::operator()...; };

}
