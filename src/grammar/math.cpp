#include "math.h"

#include <vector>
#include <iostream>

namespace flua::grammar
{
int solve_something()
{
    auto lexemes = *ExpressionLexer::lexAllBut<Whitespace>("5 * 2 + (2 + 1) * 3");

    auto iterator = lexemes.begin();
    auto result = CompleteExpression::tryParse(iterator, lexemes.end());
    if (!result)
    {
        parser::ParsingError error = result.error();
        std::cerr << "Parsing error at line " << error.where.line <<
            ", column " << error.where.column << "\n\tWhat: " << error.what << std::endl;
        return 0;
    }
    return *result;
}
}
