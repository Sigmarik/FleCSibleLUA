#include "flecsible_lua_api.h"

#include <functional>

namespace flua::lib
{
    using LibraryElement = std::variant<std::function<void(FluaState&)>, double, std::string>;
    extern const std::unordered_map<std::string, LibraryElement> STANDARD_LIBRARY;
}
