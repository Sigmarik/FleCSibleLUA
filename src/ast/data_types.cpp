#include "data_types.h"

#include <ios>
#include <sstream>
#include <format>

#include "meta/variant_helper.h"

namespace flua::data
{

static std::string to_string(Nil)
{
    return "nil";
}

static std::string to_string(bool value)
{
    return value ? "true" : "false";
}

static std::string to_string(double value)
{
    return std::format("{:.6}", value);
}

static std::string to_string(const std::string& value)
{
    return value;
}

static std::string pointer_to_hex_string(const void* ptr) {
    std::stringstream oss;
    oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(ptr);
    return oss.str();
}


static std::string to_string(const Table& dict)
{
    return "table: " + pointer_to_hex_string(dict.get());
}

static std::string to_string(const Entity& entity)
{
    return "entity: " + std::to_string(entity.id());
}

static std::string to_string(const EntityComponent& component)
{
    return "component " + component.name + " of entity " + std::to_string(component.entity.id());
}

static std::string to_string(const Function& function)
{
    if (const LibraryFunction* fnc = std::get_if<LibraryFunction>(&function))
    {
        return "external functor";
    }

    if (const LuaFunction* fnc = std::get_if<LuaFunction>(&function))
    {
        return "function: " + pointer_to_hex_string(fnc->body);
    }

    assert(false && "Unknown function type");

    return "[UNKNOWN FUNCTION TYPE]";
}

std::string to_string(const GenericValue& value)
{
    std::string result;
    std::visit([&](const auto& typedValue) { result = to_string(typedValue); }, value);
    return result;
}

bool to_bool(const GenericValue& value)
{
    if (std::holds_alternative<Nil>(value)) return false;
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    if (std::holds_alternative<EntityComponent>(value))
    {
        const EntityComponent& component = std::get<EntityComponent>(value);
        auto maybeChecker = cmp_info::ENTITY_COMPONENT_CHECKERS.find(component.name);
        if (maybeChecker == cmp_info::ENTITY_COMPONENT_CHECKERS.end()) return false;
        return maybeChecker->second(flecs::entity(component.entity));
    }

    return true;
}

const char* get_type_name(const GenericValue& value)
{
    const char* result = nullptr;
    const auto opNameGetter = meta::Overloads{
        [&](const Nil&) { result = "Nil"; },
        [&](const bool&) { result = "Boolean"; },
        [&](const double&) { result = "Numeric"; },
        [&](const std::string&) { result = "String"; },
        [&](const Table&) { result = "Table"; },
        [&](const Entity&) { result = "Entity"; },
        [&](const EntityComponent&) { result = "Component"; },
        [&](const Function&) { result = "Function"; },
    };
    std::visit(opNameGetter, value);
    return result;
}

void ValueSequence::add(const GenericValue& value)
{
    sequence.emplace_back(ValueBackrefPair{.value = value, .reference = nullptr});
}

void ValueSequence::add(GenericValue&& value)
{
    sequence.emplace_back(ValueBackrefPair{.value = std::move(value), .reference = nullptr});
}

void ValueSequence::addReferenced(GenericValue& value)
{
    sequence.emplace_back(ValueBackrefPair{.value = value, .reference = &value});
}

GenericValue ValueSequence::spit()
{
    if (sequence.empty()) return Nil{};
    GenericValue result = std::move(sequence.front().value);
    sequence.clear();
    return result;
}
}
