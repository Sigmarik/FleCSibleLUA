#pragma once

namespace flua::parser
{
struct CharacterPos
{
    static constexpr unsigned kTabSize = 4;

    unsigned line = 0;
    unsigned column = 0;

    void considerChar(char chr)
    {
        if (chr == '\n')
        {
            ++line;
            column = 0;
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
