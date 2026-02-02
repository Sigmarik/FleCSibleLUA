#pragma once

#include <string>
#include <variant>
#include <vector>
#include <exception>
#include <random>

#include <flecs.h>
#include <functional>

namespace flua
{

struct Vec2
{
    double x = 0, y = 0;
};

struct Vec3
{
    double x = 0, y = 0, z = 0;
};

struct Vec4
{
    double x = 0, y = 0, z = 0, w = 0;
};

namespace data
{
    class GenericValue;
}

struct Error final : public std::exception
{
    explicit Error(const std::string& msg) : message(msg) {}

    std::string message{};
};

class FluaState;

class ValueAccessor
{
public:
    // NOTE: These should be implicit for convenience
    ValueAccessor(size_t argumentIndex) : m_core(argumentIndex) {}
    ValueAccessor(const std::string& variableName) : m_core(variableName) {}

    friend class FluaState;

private:
    std::variant<size_t, std::string> m_core{};
};

namespace vst
{
    class Interpreter;
}

namespace lib
{
    void print(FluaState&);
}

namespace lib::misc
{
    void pcall(FluaState&);
    void error(FluaState&);
}

class FluaState
{
public:
    friend class vst::Interpreter;

    friend void lib::print(FluaState&);
    friend void lib::misc::pcall(FluaState&);
    friend void lib::misc::error(FluaState&);

    flecs::world* getWorld() const
    {
        return m_world;
    }

    unsigned getArgumentCount() const;

    data::GenericValue* getRaw(const ValueAccessor& accessor) const;

    bool isNil(const ValueAccessor& key) const;

    bool isBool(const ValueAccessor& key) const;
    bool getBool(const ValueAccessor& key) const;
    bool asBool(const ValueAccessor& key) const;

    bool isNumber(const ValueAccessor& key) const;
    double getNumber(const ValueAccessor& key) const;

    bool isVec2(const ValueAccessor& key) const;
    Vec2 getVec2(const ValueAccessor& key) const;
    bool isVec3(const ValueAccessor& key) const;
    Vec3 getVec3(const ValueAccessor& key) const;
    bool isVec4(const ValueAccessor& key) const;
    Vec4 getVec4(const ValueAccessor& key) const;

    bool isEntity(const ValueAccessor& key) const;
    flecs::entity getEntity(const ValueAccessor& key) const;

    bool isString(const ValueAccessor& key) const;
    std::string getString(const ValueAccessor& key) const;
    std::string asString(const ValueAccessor& key) const;

    void pushRaw(data::GenericValue& value);
    void pushNil() const;

    void pushValue(bool value) const;
    void pushValue(double value) const;
    void pushValue(const Vec2& value) const;
    void pushValue(const Vec3& value) const;
    void pushValue(const Vec4& value) const;
    void pushValue(const std::string& value) const;
    void pushValue(flecs::entity value) const;
    void pushValue(const std::function<void(FluaState&)>& value) const;

    void setGlobal(const std::string& name, bool value) const;
    void setGlobal(const std::string& name, double value) const;
    void setGlobal(const std::string& name, const Vec2& value) const;
    void setGlobal(const std::string& name, const Vec3& value) const;
    void setGlobal(const std::string& name, const Vec4& value) const;
    void setGlobal(const std::string& name, const std::string& value) const;
    void setGlobal(const std::string& name, flecs::entity value) const;
    void setGlobal(const std::string& name, const std::function<void(FluaState&)>& value) const;

    std::mt19937& getRandomEngine() const;

private:
    FluaState(vst::Interpreter* interpreter, flecs::world* world) : m_interpreter(interpreter), m_world(world) {}
    FluaState(const FluaState&) = default;
    FluaState(FluaState&&) = default;
    FluaState& operator=(const FluaState&) = default;
    FluaState& operator=(FluaState&&) = default;

    vst::Interpreter* m_interpreter;
    flecs::world* m_world;
};

}
