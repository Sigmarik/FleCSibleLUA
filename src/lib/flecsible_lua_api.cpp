#include "flecsible_lua_api.h"
#include "visitors/interpreter/interpreter.h"
#include "meta/variant_helper.h"

namespace flua
{
unsigned FluaState::getArgumentCount() const
{
    return static_cast<unsigned>(m_interpreter->m_externalFunctionInputs.size());
}

data::GenericValue* FluaState::getRaw(const ValueAccessor& accessor) const
{
    data::GenericValue* value = nullptr;
    const auto getter = meta::Overloads
    {
        [&](size_t index)
        {
            if (index < getArgumentCount()) value = &m_interpreter->m_externalFunctionInputs[index];
            else value = nullptr;
        },
        [&](const std::string& name)
        {
            auto& frame = m_interpreter->m_globalVariables;
            auto found = frame.find(mem_utils::PointerMappedString(name));
            if (found != frame.end()) value = found->second.get();
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

bool FluaState::isBool(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value != nullptr && std::holds_alternative<bool>(*value);
}

bool FluaState::getBool(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return std::get<bool>(*value);
}

bool FluaState::asBool(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value != nullptr ? data::to_bool(*value) : data::to_bool(data::Nil());
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

bool FluaState::isVec2(const ValueAccessor& key, bool pedantic) const
{
    const data::GenericValue* value = getRaw(key);
    if (value == nullptr) return false;
    if (std::holds_alternative<Vec2>(*value)) return true;
    if (!pedantic && std::holds_alternative<data::EntityComponent>(*value))
    {
        const auto& comp = std::get<data::EntityComponent>(*value);
        auto implicit = data::try_implicitly_convert_component(comp);
        return implicit && std::holds_alternative<Vec2>(*implicit);
    }
    return false;
}

Vec2 FluaState::getVec2(const ValueAccessor& key, bool pedantic) const
{
    const data::GenericValue* value = getRaw(key);
    if (std::holds_alternative<Vec2>(*value)) return std::get<Vec2>(*value);
    if (!pedantic && std::holds_alternative<data::EntityComponent>(*value))
    {
        const auto& comp = std::get<data::EntityComponent>(*value);
        auto implicit = data::try_implicitly_convert_component(comp);
        return std::get<Vec2>(*implicit);
    }
    assert(false && "Implicit lua member access failed");
    return {};
}

bool FluaState::isVec3(const ValueAccessor& key, bool pedantic) const
{
    const data::GenericValue* value = getRaw(key);
    if (value == nullptr) return false;
    if (std::holds_alternative<Vec3>(*value)) return true;
    if (!pedantic && std::holds_alternative<data::EntityComponent>(*value))
    {
        const auto& comp = std::get<data::EntityComponent>(*value);
        auto implicit = data::try_implicitly_convert_component(comp);
        return implicit && std::holds_alternative<Vec3>(*implicit);
    }
    return false;
}

Vec3 FluaState::getVec3(const ValueAccessor& key, bool pedantic) const
{
    const data::GenericValue* value = getRaw(key);
    if (std::holds_alternative<Vec3>(*value)) return std::get<Vec3>(*value);
    if (!pedantic && std::holds_alternative<data::EntityComponent>(*value))
    {
        const auto& comp = std::get<data::EntityComponent>(*value);
        auto implicit = data::try_implicitly_convert_component(comp);
        return std::get<Vec3>(*implicit);
    }
    assert(false && "Implicit lua member access failed");
    return {};
}

bool FluaState::isVec4(const ValueAccessor& key, bool pedantic) const
{
    const data::GenericValue* value = getRaw(key);
    if (value == nullptr) return false;
    if (std::holds_alternative<Vec4>(*value)) return true;
    if (!pedantic && std::holds_alternative<data::EntityComponent>(*value))
    {
        const auto& comp = std::get<data::EntityComponent>(*value);
        auto implicit = data::try_implicitly_convert_component(comp);
        return implicit && std::holds_alternative<Vec4>(*implicit);
    }
    return false;
}

Vec4 FluaState::getVec4(const ValueAccessor& key, bool pedantic) const
{
    const data::GenericValue* value = getRaw(key);
    if (std::holds_alternative<Vec3>(*value)) return std::get<Vec4>(*value);
    if (pedantic && std::holds_alternative<data::EntityComponent>(*value))
    {
        const auto& comp = std::get<data::EntityComponent>(*value);
        auto implicit = data::try_implicitly_convert_component(comp);
        return std::get<Vec4>(*implicit);
    }
    assert(false && "Implicit lua member access failed");
    return {};
}

bool FluaState::isEntity(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value != nullptr && std::holds_alternative<data::Entity>(*value);
}

flecs::entity FluaState::getEntity(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return std::get<data::Entity>(*value);
}

bool FluaState::isString(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value != nullptr && std::holds_alternative<mem_utils::PointerMappedString>(*value);
}

std::string FluaState::getString(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return *std::get<mem_utils::PointerMappedString>(*value);
}

std::string FluaState::asString(const ValueAccessor& key) const
{
    const data::GenericValue* value = getRaw(key);
    return value != nullptr ? data::to_string(*value) : data::to_string(data::Nil());
}

void FluaState::pushRaw(data::GenericValue& value)
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(std::move(value));
}

void FluaState::pushNil() const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(data::Nil());
}

void FluaState::pushValue(bool value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::pushValue(double value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::pushValue(const Vec2& value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::pushValue(const Vec3& value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::pushValue(const Vec4& value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::pushValue(const std::string& value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(mem_utils::PointerMappedString(value));
}

void FluaState::pushValue(flecs::entity value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(value);
}

void FluaState::pushValue(const std::function<void(FluaState&)>& value) const
{
    m_interpreter->m_externalFunctionOutputs.emplace_back(data::Function(value));
}

void FluaState::setGlobal(const std::string& name, bool value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), value);
}

void FluaState::setGlobal(const std::string& name, double value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), value);
}

void FluaState::setGlobal(const std::string& name, const Vec2& value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), value);
}

void FluaState::setGlobal(const std::string& name, const Vec3& value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), value);
}

void FluaState::setGlobal(const std::string& name, const Vec4& value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), value);
}

void FluaState::setGlobal(const std::string& name, const std::string& value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), mem_utils::PointerMappedString(value));
}

void FluaState::setGlobal(const std::string& name, flecs::entity value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), value);
}

void FluaState::setGlobal(const std::string& name, const std::function<void(FluaState&)>& value) const
{
    m_interpreter->setGlobal(mem_utils::PointerMappedString(name), value);
}

std::mt19937& FluaState::getRandomEngine() const
{
    return m_interpreter->m_rng;
}
}
