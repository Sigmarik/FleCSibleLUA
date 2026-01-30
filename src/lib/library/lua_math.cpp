#include "lua_math.h"

#include "flecsible_lua_api.h"
#include <random>

namespace flua::lib::math
{
void abs(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::abs(value));
}

void acos(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::acos(value));
}

void asin(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::asin(value));
}

void atan(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::atan(value));
}

void atan2(FluaState& state)
{
    if (state.getArgumentCount() < 2 || !state.isNumber(0) || !state.isNumber(1))
        throw Error("Expected 2 numeric arguments");

    double valueY = state.getNumber(0);
    double valueX = state.getNumber(1);
    state.pushValue(std::atan2(valueY, valueX));
}

void ceil(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::ceil(value));
}

void cos(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::cos(value));
}

void cosh(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::cosh(value));
}

void deg(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double radians = state.getNumber(0);
    state.pushValue(radians * 180.0 / kPi);
}

void rad(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double degrees = state.getNumber(0);
    state.pushValue(degrees * kPi / 180.0);
}

void exp(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::exp(value));
}

void floor(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::floor(value));
}

void fmod(FluaState& state)
{
    if (state.getArgumentCount() < 2 || !state.isNumber(0) || !state.isNumber(1))
        throw Error("Expected 2 numeric arguments");

    double valueX = state.getNumber(0);
    double valueY = state.getNumber(1);

    state.pushValue(std::fmod(valueX, valueY));
}

void frexp(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    int exponent = 0;
    double mantissa = std::frexp(value, &exponent);

    state.pushValue(mantissa);
    state.pushValue(static_cast<double>(exponent));
}

void ldexp(FluaState& state)
{
    if (state.getArgumentCount() < 2 || !state.isNumber(0) || !state.isNumber(1))
        throw Error("Expected 2 numeric arguments");

    double mantissa = state.getNumber(0);
    double exponent = state.getNumber(1);

    state.pushValue(std::ldexp(mantissa, static_cast<int>(exponent)));
}

void log(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);

    if (value <= 0.0)
        throw Error("Logarithm of non-positive number");

    state.pushValue(std::log(value));
}

void log10(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);

    if (value <= 0.0)
        throw Error("Logarithm of non-positive number");

    state.pushValue(std::log10(value));
}

void max(FluaState& state)
{
    if (state.getArgumentCount() < 1)
        throw Error("Expected at least 1 argument");

    double maxValue = state.getNumber(0);

    for (unsigned argIdx = 1; argIdx < state.getArgumentCount(); ++argIdx)
    {
        if (!state.isNumber(argIdx))
            throw Error("All arguments must be numbers");

        double current = state.getNumber(argIdx);
        if (current > maxValue)
            maxValue = current;
    }

    state.pushValue(maxValue);
}

void min(FluaState& state)
{
    if (state.getArgumentCount() < 1)
        throw Error("Expected at least 1 argument");

    double minValue = state.getNumber(0);

    for (unsigned argIdx = 1; argIdx < state.getArgumentCount(); ++argIdx)
    {
        if (!state.isNumber(argIdx))
            throw Error("All arguments must be numbers");

        double current = state.getNumber(argIdx);
        if (current < minValue)
            minValue = current;
    }

    state.pushValue(minValue);
}

void modf(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    double intPart = 0.0;
    double fracPart = std::modf(value, &intPart);

    state.pushValue(fracPart);
    state.pushValue(intPart);
}

void pow(FluaState& state)
{
    if (state.getArgumentCount() < 2 || !state.isNumber(0) || !state.isNumber(1))
        throw Error("Expected 2 numeric arguments");

    double base = state.getNumber(0);
    double exponent = state.getNumber(1);

    state.pushValue(std::pow(base, exponent));
}

void random(FluaState& state)
{
    if (state.getArgumentCount() == 0)
    {
        std::uniform_real_distribution<double> distribution(0.0, 1.0);
        state.pushValue(distribution(state.getRandomEngine()));
    }
    else if (state.getArgumentCount() == 1)
    {
        if (!state.isNumber(0))
            throw Error("Expected numeric argument");

        double limit = state.getNumber(0);
        if (limit < 1.0)
            throw Error("Argument must be >= 1");

        std::uniform_int_distribution<int> distribution(1, static_cast<int>(limit));
        state.pushValue(static_cast<double>(distribution(state.getRandomEngine())));
    }
    else
    {
        if (!state.isNumber(0) || !state.isNumber(1))
            throw Error("Expected 2 numeric arguments");

        double lower = state.getNumber(0);
        double upper = state.getNumber(1);

        if (upper < lower)
            throw Error("Upper bound must be >= lower bound");

        std::uniform_int_distribution<int> distribution(
            static_cast<int>(lower), static_cast<int>(upper));
        state.pushValue(static_cast<double>(distribution(state.getRandomEngine())));
    }
}

void randomseed(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");
    state.getRandomEngine() = std::mt19937(static_cast<unsigned>(state.getNumber(0)));
}

void sin(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::sin(value));
}

void sinh(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::sinh(value));
}

void sqrt(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);

    if (value < 0.0)
        throw Error("Square root of negative number");

    state.pushValue(std::sqrt(value));
}

void tan(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::tan(value));
}

void tanh(FluaState& state)
{
    if (state.getArgumentCount() < 1 || !state.isNumber(0))
        throw Error("Expected 1 numeric argument");

    double value = state.getNumber(0);
    state.pushValue(std::tanh(value));
}
}

