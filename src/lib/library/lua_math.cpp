#include "lua_math.h"

#include "flecsible_lua_api.h"
#include <random>

namespace flua::lib::math
{
static void apply_generic_function(FluaState& state, const std::function<double(double)>& function)
{
    if (state.getArgumentCount() < 1)
        throw Error("Expected at least 1 numeric argument");

    for (unsigned idx = 0; idx < state.getArgumentCount(); idx++)
    {
        if (state.isNumber(idx))
        {
            double value = state.getNumber(idx);
            state.pushValue(function(value));
        }
        else if (state.isVec2(idx))
        {
            Vec2 value = state.getVec2(idx);
            state.pushValue(Vec2(function(value.x), function(value.y)));
        }
        else if (state.isVec3(idx))
        {
            Vec3 value = state.getVec3(idx);
            state.pushValue(Vec3(function(value.x), function(value.y), function(value.z)));
        }
        else if (state.isVec4(idx))
        {
            Vec4 value = state.getVec4(idx);
            state.pushValue(Vec4(function(value.x), function(value.y), function(value.z), function(value.w)));
        }
        else
        {
            throw Error("Expected argument " + std::to_string(idx) + " to be either a number or a vector");
        }
    }
}

void abs(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::abs(value);
    });
}

void acos(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::acos(value);
    });
}

void asin(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::asin(value);
    });
}

void atan(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::atan(value);
    });
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
    apply_generic_function(state, [](double value)
    {
        return std::ceil(value);
    });
}

void cos(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::cos(value);
    });
}

void cosh(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::cosh(value);
    });
}

void deg(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return value * 180.0 / kPi;
    });
}

void rad(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return value * kPi / 180.0;
    });
}

void exp(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::exp(value);
    });
}

void floor(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::floor(value);
    });
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
    apply_generic_function(state, [](double value)
    {
        if (value <= 0.0)
            throw Error("Logarithm of non-positive number");
        return std::log(value);
    });
}

void log10(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        if (value <= 0.0)
            throw Error("Logarithm of non-positive number");
        return std::log10(value);
    });
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
    apply_generic_function(state, [](double value)
    {
        return std::sin(value);
    });
}

void sinh(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::sinh(value);
    });
}

void sqrt(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        if (value < 0.0)
            throw Error("Square root of negative number");
        return std::sqrt(value);
    });
}

void tan(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::tan(value);
    });
}

void tanh(FluaState& state)
{
    apply_generic_function(state, [](double value)
    {
        return std::tanh(value);
    });
}

void lerp(FluaState& state)
{
    if (state.getArgumentCount() < 3 || !state.isNumber(0) || !state.isNumber(1) || !state.isNumber(2))
        throw Error("Expected 3 numeric arguments");
    double alpha = state.getNumber(0);
    double beta = state.getNumber(1);
    double value = state.getNumber(2);

    state.pushValue(alpha * (1.0 - value) + beta * value);
}

void lerpb(FluaState& state)
{
    if (state.getArgumentCount() < 3 || !state.isNumber(0) || !state.isNumber(1) || !state.isNumber(2))
        throw Error("Expected 3 numeric arguments");
    double alpha = state.getNumber(0);
    double beta = state.getNumber(1);
    double value = state.getNumber(2);

    value = std::clamp(value, 0.0, 1.0);
    state.pushValue(alpha * (1.0 - value) + beta * value);
}

void clamp(FluaState& state)
{
    if (state.getArgumentCount() < 3 || !state.isNumber(0) || !state.isNumber(1) || !state.isNumber(2))
        throw Error("Expected 3 numeric arguments");
    double value = state.getNumber(0);
    double min = state.getNumber(1);
    double max = state.getNumber(2);

    state.pushValue(std::clamp(value, min, max));
}

void vec(FluaState& state)
{
    std::vector<double> components;

    for (unsigned idx = 0; idx < state.getArgumentCount(); idx++)
    {
        if (state.isNumber(idx))
        {
            components.emplace_back(state.getNumber(idx));
        }
        else if (state.isVec2(idx))
        {
            Vec2 vec = state.getVec2(idx);
            components.emplace_back(vec.x);
            components.emplace_back(vec.y);
        }
        else if (state.isVec3(idx))
        {
            Vec3 vec = state.getVec3(idx);
            components.emplace_back(vec.x);
            components.emplace_back(vec.y);
            components.emplace_back(vec.z);
        }
        else if (state.isVec4(idx))
        {
            Vec4 vec = state.getVec4(idx);
            components.emplace_back(vec.x);
            components.emplace_back(vec.y);
            components.emplace_back(vec.z);
            components.emplace_back(vec.w);
        }
        else
        {
            throw Error("Argument " + std::to_string(idx) + " is neither a number or a vector");
        }
    }

    if (components.size() == 2)
        state.pushValue(Vec2{components[0], components[1]});
    else if (components.size() == 3)
        state.pushValue(Vec3{components[0], components[1], components[2]});
    else if (components.size() == 4)
        state.pushValue(Vec4{components[0], components[1], components[2], components[3]});
    else
        throw Error("Cannot contruct a vector from " + std::to_string(components.size()) + " components");
}
}
