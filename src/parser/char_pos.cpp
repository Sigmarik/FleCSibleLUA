#include "char_pos.h"

namespace flua::parser
{

void CharacterPos::considerChar(char chr)
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

bool operator<(const CharacterPos& alpha, const CharacterPos& beta)
{
    return alpha.line < beta.line || (alpha.line == beta.line && alpha.column < beta.column);
}

}
