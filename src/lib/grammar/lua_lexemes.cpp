#include "lua_lexemes.h"

#include <cassert>

namespace flua::lualex
{
using namespace flua;


std::optional<Number> Number::tryConstruct(std::string_view& view)
{
    if (view.empty())
    {
        return std::nullopt;
    }

    char* end = nullptr;
    float value = std::strtof(&view.front(), &end);

    if (end == nullptr)
    {
        return std::nullopt;
    }

    if (errno == ERANGE)
    {
        errno = 0;
    }

    uintptr_t distance = reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(&view.front());
    if (distance == 0)
    {
        return std::nullopt;
    }

    view.remove_prefix(distance);

    return Number{.value = value};
}

static bool can_exist_in_name(char chr)
{
    return std::isalnum(chr) || chr == '_';
}

std::optional<Name> Name::tryConstruct(std::string_view& view)
{
    std::string name;

    while (!view.empty() && can_exist_in_name(view.front()))
    {
        name += view.front();
        view.remove_prefix(1);
    }
    if (name.empty())
    {
        return std::nullopt;
    }

    return Name{.name = name};
}

static char map_escape_char(char escapee)
{
    switch (escapee)
    {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'v': return '\v';
        default: return escapee;
    }
}

std::optional<String> String::tryConstruct(std::string_view& view)
{
    std::string str;
    if (view.empty())
    {
        return std::nullopt;
    }

    char bgnQuote = view.front();
    if (bgnQuote != '\'' && bgnQuote != '\"')
    {
        return std::nullopt;
    }

    view.remove_prefix(1);

    while (!view.empty() && view.front() != bgnQuote)
    {
        if (view.front() == '\\')
        {
            view.remove_prefix(1);
            if (view.empty())
            {
                return std::nullopt;
            }
            str += map_escape_char(view.front());
        }
        else
        {
            str += view.front();
        }
        view.remove_prefix(1);
    }

    if (!view.empty())
    {
        view.remove_prefix(1);
    }

    return String{.string = str};
}

std::optional<SingleLineComment> SingleLineComment::tryConstruct(std::string_view& view)
{
    if (view.size() < 2)
    {
        return std::nullopt;
    }

    if (view[0] != '-' || view[1] != '-')
    {
        return std::nullopt;
    }

    while (!view.empty() && view.front() != '\n')
    {
        view.remove_prefix(1);
    }

    return SingleLineComment{};
}

std::optional<MultiLineComment> MultiLineComment::tryConstruct(std::string_view& view)
{
    if (view.size() < 4)
    {
        return std::nullopt;
    }

    if (view[0] != '-' || view[1] != '-' || view[2] != '[' || view[3] != '[')
    {
        return std::nullopt;
    }

    while (view.size() >= 2 && (view[0] != ']' || view[1] != ']'))
    {
        view.remove_prefix(1);
    }

    view.remove_prefix(std::min(view.size(), 2ull));

    return MultiLineComment{};
}

std::optional<Whitespace> Whitespace::tryConstruct(std::string_view& view)
{
    bool hasWs = false;
    while (!view.empty() && (std::isspace(view.front()) || view.front() == ';'))
    {
        hasWs = true;
        view.remove_prefix(1);
    }
    if (hasWs)
    {
        return Whitespace();
    }
    return std::nullopt;
}
}
