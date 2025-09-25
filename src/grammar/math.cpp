#include "math.h"

#include <vector>

namespace flua::grammar
{
int solve_something()
{
    auto lexemes = ExpressionParser::lexAllBut<Whitespace>("5 * 2 + (2 + 1) * 3");

    auto iterator = lexemes.begin();
    return *Expression::tryParse(iterator, lexemes.end());
}
}
