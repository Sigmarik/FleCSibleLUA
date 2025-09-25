#include "math.h"

#include <vector>

namespace flua::grammar
{
int solve_something()
{
    std::vector<LexemeVariant> lexemes;

    lexemes.emplace_back(Number{.value = 5});
    lexemes.emplace_back(Multiply{});
    lexemes.emplace_back(Number{.value = 2});
    lexemes.emplace_back(Add{});
    lexemes.emplace_back(BracketL{});
    lexemes.emplace_back(Number{.value = 1});
    lexemes.emplace_back(Add{});
    lexemes.emplace_back(Number{.value = 2});
    lexemes.emplace_back(BracketR{});
    lexemes.emplace_back(Multiply{});
    lexemes.emplace_back(Number{.value = 3});

    auto iterator = lexemes.begin();
    return *Expression::tryParse(iterator, lexemes.end());
}
}
