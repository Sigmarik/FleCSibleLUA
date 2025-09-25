#include "math.h"

#include <vector>
#include <iostream>

namespace flua::grammar
{
int solve_something()
{
    auto result = ExpressionParser::parse("5 * 2 + (2 + 1) * 3 *");
    if (!result)
    {
        parser::ParsingError error = result.error();
        std::cerr << "Parsing error at line " << error.where.line <<
            ", column " << error.where.column << ":\n\t" << error.what << std::endl;
        return 0;
    }
    return *result;
}
}
