#include "flecsible_lua_api.h"
#include "visitors/interpreter/interpreter.h"
#include "meta/variant_helper.h"

namespace flua
{
unsigned FluaState::getArgumentCount() const
{
    return static_cast<unsigned>(m_interpreter->m_externalFunctionInputs.size());
}

const data::GenericValue* FluaState::getRaw(const ValueAccessor& accessor) const
{
    data::GenericValue* value = nullptr;
    const auto getter = meta::Overloads
    {
        [&](unsigned index)
        {
            if (index < getArgumentCount()) value = &m_interpreter->m_externalFunctionInputs[index];
            else value = nullptr;
        },
        [&](const std::string& name)
        {
            auto& frame = m_interpreter->m_stack.front();
            auto found = frame.varNameMap.find(name);
            if (found != frame.varNameMap.end()) value = found->second.get();
            else value = nullptr;
        }
    };
    std::visit(getter, accessor.m_core);
    return value;
}

bool FluaState::isNil(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value == nullptr || std::holds_alternative<data::Nil>(*value);
}

bool FluaState::isNumber(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value != nullptr && std::holds_alternative<double>(*value);
}

double FluaState::getNumber(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return std::get<double>(*value);
}

std::string FluaState::asString(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value != nullptr ? data::to_string(*value) : data::to_string(data::Nil());
}

void FluaState::pushNil() const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(data::Nil());
}

void FluaState::pushValue(double value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::pushValue(const std::string& value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::setGlobal(const std::string& name, double value) const
{
    m_interpreter->m_stack.front().varNameMap[name] = mem_utils::CopyMovePtr<data::GenericValue>(value);
}

void FluaState::setGlobal(const std::string& name, const std::string& value) const
{
    m_interpreter->m_stack.front().varNameMap[name] = mem_utils::CopyMovePtr<data::GenericValue>(value);
}
}
