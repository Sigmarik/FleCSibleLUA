#pragma once

#include <string>
#include <variant>
#include <vector>

namespace flua
{

namespace data
{
    class GenericValue;
}

class FluaState;

class ValueAccessor
{
public:
    // NOTE: These should be implicit for convenience
    ValueAccessor(unsigned argumentIndex) : m_core(argumentIndex) {}
    ValueAccessor(const std::string& variableName) : m_core(variableName) {}

    void extend(const std::string& field);

    friend class FluaState;

private:
    std::variant<unsigned, std::string> m_core{};
    std::vector<std::string> m_internalPath{};
};

namespace vst
{
    class Interpreter;
}

namespace lib
{
    void print(FluaState*);
}

class FluaState
{
public:
    friend class vst::Interpreter;

    friend void lib::print(FluaState*);

    unsigned getArgumentCount() const;

    const data::GenericValue* getRaw(const ValueAccessor& accessor) const;

    bool isNil(const ValueAccessor& key) const;

    bool isNumber(const ValueAccessor& key) const;
    double getNumber(const ValueAccessor& key) const;

    std::string asString(const ValueAccessor& key) const;

    void pushNil() const;
    void pushValue(double value) const;
    void pushValue(const std::string& value) const;

    void setGlobal(const std::string& name, double value) const;
    void setGlobal(const std::string& name, const std::string& value) const;

private:
    FluaState(vst::Interpreter* interpreter) : m_interpreter(interpreter) {}
    FluaState(const FluaState&) = default;
    FluaState(FluaState&&) = default;
    FluaState& operator=(const FluaState&) = default;
    FluaState& operator=(FluaState&&) = default;

    vst::Interpreter* m_interpreter;
};

}
