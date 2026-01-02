#include "output.h"

#include <functional>

namespace flua::lib
{
    extern const std::unordered_map<std::string, std::function<void(FluaState*)>> STANDARD_LIBRARY;
}
