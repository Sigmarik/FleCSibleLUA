#pragma once

#include <algorithm>

namespace flua::meta
{
template <size_t N>
struct StringLiteral final {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }

    char value[N];
};
}
