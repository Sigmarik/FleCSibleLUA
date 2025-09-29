#pragma once

#include <variant>

namespace flua::parser
{
struct CharacterPos
{
    static constexpr unsigned kTabSize = 4;

    unsigned line = 1;
    unsigned column = 1;

    void considerChar(char chr)
    {
        if (chr == '\n')
        {
            ++line;
            column = 1;
        }
        else if (chr == '\t')
        {
            column += kTabSize;
        }
        else
        {
            ++column;
        }
    }
};

template <class LexemeVariant>
CharacterPos posFromVariant(LexemeVariant&& variant)
{
    CharacterPos result;
    std::visit([&](const auto& specificLexeme) { result = specificLexeme.startingPos; }, variant);
    return result;
}
}
