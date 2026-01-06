#pragma once

#include <string>

#include "char_pos.h"

namespace flua::parser
{
struct ParsingError
{
    std::string what{};
    CharacterPos where{};
};
}
