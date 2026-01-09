#include "lua_strings.h"

#include <string>
#include <cctype>
#include <algorithm>
#include <regex>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <bitset>
#include <complex>

#include "ast/data_types.h"

namespace flua::lib::string
{
void tostring(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("Expected at least 1 argument");
    state->pushValue(state->asString(0));
}


void byte(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.byte requires at least 1 argument");

    std::string str = state->asString(0);
    int len = static_cast<int>(str.length());


    int i = 1;
    int j = i;

    if (state->getArgumentCount() >= 2 && !state->isNil(1))
    {
        i = static_cast<int>(state->getNumber(1));
        if (i < 0) i = len + i + 1;
        if (i < 1) i = 1;
        j = i;
    }

    if (state->getArgumentCount() >= 3 && !state->isNil(2))
    {
        j = static_cast<int>(state->getNumber(2));
        if (j < 0) j = len + j + 1;
        if (j > len) j = len;
    }

    if (i > len || i > j)
    {
        state->pushNil();
        return;
    }


    for (int idx = i; idx <= j; ++idx)
    {
        state->pushValue(static_cast<double>(static_cast<unsigned char>(str[idx - 1])));
    }
}


void to_char(FluaState* state)
{
    std::string result;
    unsigned argCount = state->getArgumentCount();

    for (unsigned i = 0; i < argCount; ++i)
    {
        if (!state->isNumber(i))
            throw Error("string.char arguments must be numbers");

        int code = static_cast<int>(state->getNumber(i));
        if (code < 0 || code > 255)
            throw Error("invalid character code");

        result += static_cast<char>(code);
    }

    state->pushValue(result);
}


void len(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.len requires 1 argument");

    std::string str = state->asString(0);
    state->pushValue(static_cast<double>(str.length()));
}


void lower(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.lower requires 1 argument");

    std::string str = state->asString(0);
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    state->pushValue(str);
}


void upper(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.upper requires 1 argument");

    std::string str = state->asString(0);
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    state->pushValue(str);
}


void reverse(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.reverse requires 1 argument");

    std::string str = state->asString(0);
    std::reverse(str.begin(), str.end());
    state->pushValue(str);
}


void sub(FluaState* state)
{
    if (state->getArgumentCount() < 2)
        throw Error("string.sub requires at least 2 arguments");

    std::string str = state->asString(0);
    int len = static_cast<int>(str.length());

    int i = static_cast<int>(state->getNumber(1));
    int j = len;


    if (i < 0) i = len + i + 1;
    if (i < 1) i = 1;

    if (state->getArgumentCount() >= 3 && !state->isNil(2))
    {
        j = static_cast<int>(state->getNumber(2));
        if (j < 0) j = len + j + 1;
        if (j > len) j = len;
    }

    if (i > j || i > len || j < 1)
    {
        state->pushValue(std::string(""));
        return;
    }


    int start = std::max(1, i) - 1;
    int end = std::min(j, len) - 1;

    if (start <= end)
        state->pushValue(str.substr(start, end - start + 1));
    else
        state->pushValue(std::string(""));
}


void rep(FluaState* state)
{
    if (state->getArgumentCount() < 2)
        throw Error("string.rep requires at least 2 arguments");

    std::string str = state->asString(0);
    int n = static_cast<int>(state->getNumber(1));

    if (n <= 0)
    {
        state->pushValue(std::string(""));
        return;
    }

    std::string sep;
    if (state->getArgumentCount() >= 3 && !state->isNil(2))
    {
        sep = state->asString(2);
    }

    std::string result;
    for (int i = 0; i < n; ++i)
    {
        if (i > 0) result += sep;
        result += str;
    }

    state->pushValue(result);
}

void format(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.format requires at least 1 argument");

    std::stringstream output;
    std::string fmt = state->asString(0);
    size_t argIdx = 1;

    const char* inputCh = fmt.c_str();
    while (inputCh && *inputCh)
    {
        if (*inputCh != '%')
        {
            output.put(*inputCh);
            ++inputCh;
        }
        else if (*++inputCh == '%')
        {
            output.put('%');
            ++inputCh;
        }
        else
        {
            size_t formatSpan = std::strspn(inputCh, "-0123456789.");
            std::string fmtBuf(formatSpan + 2, '\0');
            fmtBuf[0] = '%';
            std::strncpy(&fmtBuf.front() + 1, inputCh, formatSpan);
            inputCh += formatSpan;
            fmtBuf.back() = *inputCh;
            std::string fmtOut(120, '\0');
            auto processFmtFunc = [&](const auto& arg)
            {
                std::snprintf(&fmtOut.front(), fmtOut.size(), fmtBuf.c_str(), arg);
                output << fmtOut.c_str();
            };
            switch (*inputCh)
            {
                case 'c':
                case 'd':
                case 'i':
                case 'u':
                case 'o':
                case 'x':
                case 'X':
                {
                    if (!state->isNumber(argIdx))
                        throw Error(
                            "string.format cannot pass non-numeric argument " + state->asString(argIdx) + " as %" +
                            std::string(1, *inputCh));
                    processFmtFunc(static_cast<long long int>(state->getNumber(argIdx)));
                    break;
                }
                case 'a':
                case 'A':
                case 'f':
                case 'e':
                case 'E':
                case 'g':
                case 'G':
                {
                    processFmtFunc(state->getNumber(argIdx));
                    break;
                }
                case 'p':
                {
                    processFmtFunc(state->getRaw(argIdx));
                    break;
                }
                case 'q':
                case 's':
                {
                    std::string string = state->asString(argIdx);
                    processFmtFunc(string.c_str());
                    break;
                }
                default:
                {
                    throw Error("Incorrect/unsupported format option " + std::string(1, *inputCh));
                }
            }
            ++argIdx;
            ++inputCh;
        }
    }

    state->pushValue(output.str());
}

void pack(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.pack requires at least 1 argument");

    std::string fmt = state->asString(0);
    std::string result;

    size_t argIndex = 1;
    for (char f : fmt)
    {
        if (argIndex >= state->getArgumentCount())
            throw Error("not enough arguments for string.pack");

        switch (f)
        {
            case 'c':
            {
                char c = static_cast<char>(state->getNumber(argIndex++));
                result += c;
                break;
            }
            case 'i':
            {
                int32_t val = static_cast<int32_t>(state->getNumber(argIndex++));
                for (int i = 0; i < 4; ++i)
                {
                    result += static_cast<char>(val & 0xFF);
                    val >>= 8;
                }
                break;
            }
            case 'f':
            {
                float val = static_cast<float>(state->getNumber(argIndex++));
                char* bytes = reinterpret_cast<char*>(&val);
                for (int i = 0; i < 4; ++i)
                    result += bytes[i];
                break;
            }
            case 's':
            {
                std::string str = state->asString(argIndex++);

                uint32_t len = static_cast<uint32_t>(str.length());
                for (int i = 0; i < 4; ++i)
                {
                    result += static_cast<char>(len & 0xFF);
                    len >>= 8;
                }
                result += str;
                break;
            }
            default:
                throw Error("invalid format specifier in string.pack");
        }
    }

    state->pushValue(result);
}


void unpack(FluaState* state)
{
    if (state->getArgumentCount() < 2)
        throw Error("string.unpack requires at least 2 arguments");

    std::string fmt = state->asString(0);
    std::string data = state->asString(1);

    int pos = 1;
    if (state->getArgumentCount() >= 3 && !state->isNil(2))
    {
        pos = static_cast<int>(state->getNumber(2));
    }

    if (pos < 1 || pos > static_cast<int>(data.length()))
        throw Error("invalid position in string.unpack");

    size_t index = pos - 1;

    for (char f : fmt)
    {
        switch (f)
        {
            case 'c':
                if (index >= data.length())
                    throw Error("data string too short");
                state->pushValue(static_cast<double>(static_cast<unsigned char>(data[index++])));
                break;

            case 'i':
                if (index + 3 >= data.length())
                    throw Error("data string too short");
                {
                    int32_t val = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        val |= static_cast<unsigned char>(data[index + i]) << (8 * i);
                    }
                    state->pushValue(static_cast<double>(val));
                    index += 4;
                }
                break;

            case 'f':
                if (index + 3 >= data.length())
                    throw Error("data string too short");
                {
                    float val;
                    char* bytes = reinterpret_cast<char*>(&val);
                    for (int i = 0; i < 4; ++i)
                        bytes[i] = data[index + i];
                    state->pushValue(static_cast<double>(val));
                    index += 4;
                }
                break;

            case 's':
                if (index + 3 >= data.length())
                    throw Error("data string too short");
                {
                    uint32_t len = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        len |= static_cast<unsigned char>(data[index + i]) << (8 * i);
                    }
                    index += 4;

                    if (index + len > data.length())
                        throw Error("data string too short");

                    std::string str = data.substr(index, len);
                    state->pushValue(str);
                    index += len;
                }
                break;

            default:
                throw Error("invalid format specifier in string.unpack");
        }
    }


    state->pushValue(static_cast<double>(index + 1));
}

void packsize(FluaState* state)
{
    if (state->getArgumentCount() < 1)
        throw Error("string.packsize requires 1 argument");

    std::string fmt = state->asString(0);
    size_t size = 0;

    for (char symbol : fmt)
    {
        switch (symbol)
        {
            case 'c':
                size += 1;
                break;
            case 'i':
            case 'f':
                size += 4;
                break;
            case 's':
                size += 4;
                break;
            default:
                throw Error("invalid format specifier in string.packsize");
        }
    }

    state->pushValue(static_cast<double>(size));
}
}
