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

static std::string to_string(const Vec2& vec)
{
    return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ")";
}

static std::string to_string(const Vec3& vec)
{
    return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
}

static std::string to_string(const Vec4& vec)
{
    return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " +
        std::to_string(vec.z) + ", " + std::to_string(vec.w) + ")";
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
    if (std::holds_alternative<Entity>(value))
    {
        return std::get<Entity>(value).is_alive();
    }

    return true;
}

std::string get_type_name(const GenericValue& value)
{
    const char* result = nullptr;
    const auto opNameGetter = meta::Overloads{
        [&](Nil) { result = "nil"; },
        [&](bool) { result = "boolean"; },
        [&](double) { result = "number"; },
        [&](const Vec2&) { result = "vec2"; },
        [&](const Vec3&) { result = "vec3"; },
        [&](const Vec4&) { result = "vec4"; },
        [&](const std::string&) { result = "string"; },
        [&](const Table&) { result = "table"; },
        [&](const Entity&) { result = "entity"; },
        [&](const EntityComponent&) { result = "component"; },
        [&](const Function&) { result = "function"; },
    };
    std::visit(opNameGetter, value);
    return result;
}

template <UnaryOpType>
static std::optional<GenericValue> perform_unary(const GenericValue&) { assert(false); return {}; }

template <>
std::optional<GenericValue> perform_unary<UnaryOpType::Not>(const GenericValue& value)
{
    return !to_bool(value);
}

template <>
std::optional<GenericValue> perform_unary<UnaryOpType::Negate>(const GenericValue& value)
{
    if (std::holds_alternative<double>(value)) return -std::get<double>(value);
    return std::nullopt;
}

template <>
std::optional<GenericValue> perform_unary<UnaryOpType::Length>(const GenericValue& value)
{
    if (std::holds_alternative<std::string>(value)) return static_cast<double>(std::get<std::string>(value).size());
    if (std::holds_alternative<Table>(value)) return static_cast<double>(std::get<Table>(value)->size());
    return std::nullopt;
}

std::optional<GenericValue> perform_unary_operation(UnaryOpType op, const GenericValue& value)
{
    switch (op)
    {
        case UnaryOpType::Not: return perform_unary<UnaryOpType::Not>(value);
        case UnaryOpType::Length: return perform_unary<UnaryOpType::Length>(value);
        case UnaryOpType::Negate: return perform_unary<UnaryOpType::Negate>(value);
        default: assert(false && "Undefined unary operator type"); return {};
    }
}

template <BinaryOpType>
static std::optional<GenericValue> perform_binary(const GenericValue&, const GenericValue&)
{
    assert(false); return {};
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Add>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) + std::get<double>(beta);
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Subtract>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) - std::get<double>(beta);
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Multiply>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) * std::get<double>(beta);
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Divide>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) / std::get<double>(beta);
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Mod>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    double whole = std::get<double>(alpha) / std::get<double>(beta);
    return std::get<double>(alpha) - std::floor(whole) * std::get<double>(beta);
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Pow>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::pow(std::get<double>(alpha), std::get<double>(beta));
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::And>(const GenericValue& alpha, const GenericValue& beta)
{
    return to_bool(alpha) && to_bool(beta);
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Or>(const GenericValue& alpha, const GenericValue& beta)
{
    return to_bool(alpha) || to_bool(beta);
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Xor>(const GenericValue& alpha, const GenericValue& beta)
{
    return to_bool(alpha) != to_bool(beta);
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Concatenate>(const GenericValue& alpha, const GenericValue& beta)
{
    return to_string(alpha) + to_string(beta);
}

static constexpr double CMP_EPS = 1e-6;

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::CmpEq>(const GenericValue& alpha, const GenericValue& beta)
{
    if (alpha.index() != beta.index()) return false;
    bool result = false;
    const auto comparators = meta::Overloads
    {
        [&](Nil) { result = true; },
        [&](bool) { result = std::get<bool>(alpha) == std::get<bool>(beta); },
        [&](double) { result = std::abs(std::get<double>(alpha) - std::get<double>(beta)) < CMP_EPS; },
        [&](const Entity&) { result = std::get<Entity>(alpha).id() == std::get<Entity>(beta).id(); },
        [&](const std::string&) { result = std::get<std::string>(alpha) == std::get<std::string>(beta); },
        [&](const Table&) { result = &std::get<Table>(alpha) == &std::get<Table>(beta); },
        [&](const Vec2&)
        {
            auto alp = std::get<Vec2>(alpha);
            auto bet = std::get<Vec2>(beta);
            result = std::abs(alp.x - bet.x) + std::abs(alp.y - bet.y) < CMP_EPS * 2;
        },
        [&](const Vec3&)
        {
            auto alp = std::get<Vec3>(alpha);
            auto bet = std::get<Vec3>(beta);
            result = std::abs(alp.x - bet.x) + std::abs(alp.y - bet.y) + std::abs(alp.z - bet.z) < CMP_EPS * 3;
        },
        [&](const Vec4&)
        {
            auto alp = std::get<Vec4>(alpha);
            auto bet = std::get<Vec4>(beta);
            result = std::abs(alp.x - bet.x) + std::abs(alp.y - bet.y) +
                std::abs(alp.z - bet.z) + std::abs(alp.w - bet.w) < CMP_EPS * 4;
        },
        [&](const auto&) { result = to_string(alpha) == to_string(beta); },
    };
    std::visit(comparators, alpha);
    return result;
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::CmpLt>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) < std::get<double>(beta);
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::CmpLe>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) - std::get<double>(beta) < CMP_EPS;
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::CmpGt>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) > std::get<double>(beta);
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::CmpGe>(const GenericValue& alpha, const GenericValue& beta)
{
    if (!std::holds_alternative<double>(alpha) || !std::holds_alternative<double>(beta)) return std::nullopt;
    return std::get<double>(alpha) - std::get<double>(beta) > -CMP_EPS;
}
template <>
std::optional<GenericValue> perform_binary<BinaryOpType::CmpNeq>(const GenericValue& alpha, const GenericValue& beta)
{
    return !std::get<bool>(*perform_binary<BinaryOpType::CmpEq>(alpha, beta));
}

std::optional<GenericValue> perform_binary_operation(BinaryOpType op, const GenericValue& alpha,
    const GenericValue& beta)
{
    switch (op)
    {
        case BinaryOpType::Add:         return perform_binary<BinaryOpType::Add>(alpha, beta);
        case BinaryOpType::Subtract:    return perform_binary<BinaryOpType::Subtract>(alpha, beta);
        case BinaryOpType::Multiply:    return perform_binary<BinaryOpType::Multiply>(alpha, beta);
        case BinaryOpType::Divide:      return perform_binary<BinaryOpType::Divide>(alpha, beta);
        case BinaryOpType::Mod:         return perform_binary<BinaryOpType::Mod>(alpha, beta);
        case BinaryOpType::Pow:         return perform_binary<BinaryOpType::Pow>(alpha, beta);
        case BinaryOpType::And:         return perform_binary<BinaryOpType::And>(alpha, beta);
        case BinaryOpType::Or:          return perform_binary<BinaryOpType::Or>(alpha, beta);
        case BinaryOpType::Xor:         return perform_binary<BinaryOpType::Xor>(alpha, beta);
        case BinaryOpType::Concatenate: return perform_binary<BinaryOpType::Concatenate>(alpha, beta);
        case BinaryOpType::CmpEq:       return perform_binary<BinaryOpType::CmpEq>(alpha, beta);
        case BinaryOpType::CmpLt:       return perform_binary<BinaryOpType::CmpLt>(alpha, beta);
        case BinaryOpType::CmpLe:       return perform_binary<BinaryOpType::CmpLe>(alpha, beta);
        case BinaryOpType::CmpGt:       return perform_binary<BinaryOpType::CmpGt>(alpha, beta);
        case BinaryOpType::CmpGe:       return perform_binary<BinaryOpType::CmpGe>(alpha, beta);
        case BinaryOpType::CmpNeq:      return perform_binary<BinaryOpType::CmpNeq>(alpha, beta);
        default: assert(false); return std::nullopt;
    }
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
