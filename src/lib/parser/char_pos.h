#pragma once

#include <variant>

namespace flua::parser
{
struct CharacterPos
{
    static constexpr unsigned kTabSize = 4;

    unsigned line = 1;
    unsigned column = 1;

    void considerChar(char chr);
};

bool operator<(const CharacterPos& alpha, const CharacterPos& beta);

template <class LexemeVariant>
CharacterPos posFromVariant(LexemeVariant&& variant)
{
    CharacterPos result;
    std::visit([&](const auto& specificLexeme) { result = specificLexeme.startingPos; }, variant);
    return result;
}
}
