#pragma once

#include <string>
#include <inttypes.h>

namespace flua::ids
{

struct ResolvableName
{
    ResolvableName(const std::string& string) : string(string) {}

    using IdT = uint32_t;

    void resolveNew();
    void resolveAs(const ResolvableName& name);
    void resolveAs(IdT id);

    std::string string;
    IdT id = 0;
};

}
