#pragma once

#include <string>
#include <string_view>

namespace flua::parser
{
using namespace flua;

struct Lexeme {};

struct LexemeDescriptor
{
    bool (*tryConstruct)(Lexeme& lexeme, std::string_view& view);
    std::string name;
};

}
