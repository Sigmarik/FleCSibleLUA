#include "library.h"

namespace flua::lib
{
const std::unordered_map<std::string, std::function<void(FluaState*)> > STANDARD_LIBRARY
{
    {"print", print},
    {"ipairs", ipairs},
};
}
