#include "data_types.h"

#include <ios>
#include <sstream>
#include <format>
#include <iostream>

#include "meta/variant_helper.h"
#include "component_map/comp_map.h"

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

static std::string to_string(const mem_utils::PointerMappedString& value)
{
    return *value;
}

static std::string to_string(const Vec2& vec)
{
    return "(" + to_string(vec.x) + ", " + to_string(vec.y) + ")";
}

static std::string to_string(const Vec3& vec)
{
    return "(" + to_string(vec.x) + ", " + to_string(vec.y) + ", " + to_string(vec.z) + ")";
}

static std::string to_string(const Vec4& vec)
{
    return "(" + to_string(vec.x) + ", " + to_string(vec.y) + ", " +
        to_string(vec.z) + ", " + to_string(vec.w) + ")";
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
    return "component " + *component.name + " of entity " + std::to_string(component.entity.id());
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
        auto maybeChecker = cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.find(component.name);
        if (maybeChecker == cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.end()) return false;
        return maybeChecker->second(flecs::entity(component.entity));
    }
    if (std::holds_alternative<Entity>(value))
    {
        return std::get<Entity>(value).is_alive();
    }

    return true;
}

std::string to_string(const Address& addr)
{
    return (addr.relative ? "+" : "") +
            std::to_string(addr.shift);
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
        [&](const mem_utils::PointerMappedString&) { result = "string"; },
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
    std::optional<GenericValue> result = std::nullopt;
    auto visitor = meta::Overloads
    {
        [&](double val) { result = -val; },
        [&](const Vec2& val) { result = Vec2{ .x = -val.x, .y = -val.y }; },
        [&](const Vec3& val) { result = Vec3{ .x = -val.x, .y = -val.y, .z = -val.z }; },
        [&](const Vec4& val) { result = Vec4{ .x = -val.x, .y = -val.y, .z = -val.z, .w = -val.w }; },
        [&](const auto&) { result = std::nullopt; },
    };
    std::visit(visitor, value);
    return result;
}

template <>
std::optional<GenericValue> perform_unary<UnaryOpType::Length>(const GenericValue& value)
{
    if (std::holds_alternative<mem_utils::PointerMappedString>(value))
    {
        return static_cast<double>(std::get<mem_utils::PointerMappedString>(value)->size());
    }
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

enum class VectorComponent
{
    eX, eY, eZ, eW
};

static std::optional<double> try_get_entity_component_field(const EntityComponent& comp,
    const mem_utils::PointerMappedString& field)
{
    if (!comp.entity) return std::nullopt;

    auto checker = cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.find(comp.name);
    if (checker == cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.end()) return std::nullopt;
    if (!checker->second(comp.entity)) return std::nullopt;

    auto accessor = cmp_info::ENTITY_MEMBER_MAP.find(*comp.name + " " + *field);
    if (accessor == cmp_info::ENTITY_MEMBER_MAP.end()) return std::nullopt;

    cmp_info::GenericComponentPtr accessed = accessor->second(comp.entity);
    std::optional<double> result = std::nullopt;
    auto visitors = meta::Overloads
    {
        [&](int* ptr) { result = *ptr; },
        [&](unsigned* ptr) { result = *ptr; },
        [&](float* ptr) { result = *ptr; },
        [&](double* ptr) { result = *ptr; },
        [&](auto*) { result = std::nullopt; },
    };
    std::visit(visitors, accessed);
    return result;
}

static bool try_set_entity_component_field(const EntityComponent& comp,
    const mem_utils::PointerMappedString& field, double value)
{
    if (!comp.entity) return false;

    auto checker = cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.find(comp.name);
    if (checker == cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.end()) return false;
    if (!checker->second(comp.entity)) return false;

    auto accessor = cmp_info::ENTITY_MEMBER_MAP.find(*comp.name + " " + *field);
    if (accessor == cmp_info::ENTITY_MEMBER_MAP.end()) return false;

    cmp_info::GenericComponentPtr accessed = accessor->second(comp.entity);
    bool result = true;
    auto visitors = meta::Overloads
    {
        [&](int* ptr) { *ptr = static_cast<int>(value); },
        [&](unsigned* ptr) { *ptr = static_cast<unsigned>(value); },
        [&](float* ptr) { *ptr = static_cast<float>(value); },
        [&](double* ptr) { *ptr = value; },
        [&](auto*) { result = false; },
    };
    std::visit(visitors, accessed);
    return result;
}

static const mem_utils::PointerMappedString CACHED_X("x");
static const mem_utils::PointerMappedString CACHED_Y("y");
static const mem_utils::PointerMappedString CACHED_Z("z");
static const mem_utils::PointerMappedString CACHED_W("w");

static std::optional<double> try_get_component(const GenericValue& value, VectorComponent component)
{

    switch (component)
    {
    case VectorComponent::eX:
    {
        std::optional<double> result = std::nullopt;
        auto visitor = meta::Overloads
        {
            [&](double val) { result = val; },
            [&](const Vec2& val) { result = val.x; },
            [&](const Vec3& val) { result = val.x; },
            [&](const Vec4& val) { result = val.x; },
            [&](const EntityComponent& val)
            {
                result = try_get_entity_component_field(val, CACHED_X);
            },
            [&](auto) { result = std::nullopt; },
        };
        std::visit(visitor, value);
        return result;
    }
    case VectorComponent::eY:
    {
        std::optional<double> result = std::nullopt;
        auto visitor = meta::Overloads
        {
            [&](double val) { result = val; },
            [&](const Vec2& val) { result = val.y; },
            [&](const Vec3& val) { result = val.y; },
            [&](const Vec4& val) { result = val.y; },
            [&](const EntityComponent& val)
            {
                result = try_get_entity_component_field(val, CACHED_Y);
            },
            [&](auto) { result = std::nullopt; },
        };
        std::visit(visitor, value);
        return result;
    }
    case VectorComponent::eZ:
    {
        std::optional<double> result = std::nullopt;
        auto visitor = meta::Overloads
        {
            [&](double val) { result = val; },
            [&](const Vec3& val) { result = val.z; },
            [&](const Vec4& val) { result = val.z; },
            [&](const EntityComponent& val)
            {
                result = try_get_entity_component_field(val, CACHED_Z);
            },
            [&](auto) { result = std::nullopt; },
        };
        std::visit(visitor, value);
        return result;
    }
    case VectorComponent::eW:
    {
        std::optional<double> result = std::nullopt;
        auto visitor = meta::Overloads
        {
            [&](double val) { result = val; },
            [&](const Vec4& val) { result = val.w; },
            [&](const EntityComponent& val)
            {
                result = try_get_entity_component_field(val, CACHED_W);
            },
            [&](auto) { result = std::nullopt; },
        };
        std::visit(visitor, value);
        return result;
    }
    default: assert(false && "Unknown enum option");
    }

    return std::nullopt;
}

std::optional<GenericValue> std_to_geom(const std::vector<double>& components)
{
    if (components.size() == 2) return Vec2{components[0], components[1]};
    if (components.size() == 3) return Vec3{components[0], components[1], components[2]};
    if (components.size() == 4) return Vec4{components[0], components[1], components[2], components[3]};
    return std::nullopt;
}

static std::optional<GenericValue> perform_floaty(const GenericValue& alpha, const GenericValue& beta,
    const std::function<double(double, double)>& fnc)
{
    if (std::holds_alternative<double>(alpha) && std::holds_alternative<double>(beta))
        return fnc(std::get<double>(alpha), std::get<double>(beta));

    std::vector<double> outputComponents;
    for (VectorComponent comp : {VectorComponent::eX, VectorComponent::eY, VectorComponent::eZ, VectorComponent::eW})
    {
        auto alp = try_get_component(alpha, comp);
        if (!alp) break;
        auto bet = try_get_component(beta, comp);
        if (!bet) break;

        outputComponents.emplace_back(fnc(*alp, *bet));
    }

    return std_to_geom(outputComponents);
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Add>(const GenericValue& alpha, const GenericValue& beta)
{
    return perform_floaty(alpha, beta, [](double alp, double bet)
    {
        return alp + bet;
    });
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Subtract>(const GenericValue& alpha, const GenericValue& beta)
{
    return perform_floaty(alpha, beta, [](double alp, double bet)
    {
        return alp - bet;
    });
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Multiply>(const GenericValue& alpha, const GenericValue& beta)
{
    return perform_floaty(alpha, beta, [](double alp, double bet)
    {
        return alp * bet;
    });
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Divide>(const GenericValue& alpha, const GenericValue& beta)
{
    return perform_floaty(alpha, beta, [](double alp, double bet)
    {
        return alp / bet;
    });
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Mod>(const GenericValue& alpha, const GenericValue& beta)
{
    return perform_floaty(alpha, beta, [](double alp, double bet)
    {
        double whole = alp / bet;
        return alp - std::floor(whole) * bet;
    });
}

template <>
std::optional<GenericValue> perform_binary<BinaryOpType::Pow>(const GenericValue& alpha, const GenericValue& beta)
{
    return perform_floaty(alpha, beta, [](double alp, double bet)
    {
        return std::pow(alp, bet);
    });
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
    return mem_utils::PointerMappedString(to_string(alpha) + to_string(beta));
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
        [&](const std::string&)
        {
            result = std::get<mem_utils::PointerMappedString>(alpha) == std::get<mem_utils::PointerMappedString>(beta);
        },
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

std::optional<GenericValue> try_implicitly_convert_component(const EntityComponent& comp)
{
    std::vector<double> components;
    for (const mem_utils::PointerMappedString& cmp : {CACHED_X, CACHED_Y, CACHED_Z, CACHED_W})
    {
        auto field = try_get_entity_component_field(comp, cmp);
        if (field) components.emplace_back(*field);
        else break;
    }
    return std_to_geom(components);
}

bool try_implicitly_write_to_component(const EntityComponent& comp, const GenericValue& value)
{
    static const std::vector<std::pair<VectorComponent, mem_utils::PointerMappedString>> kCmpMappings =
    {
        {VectorComponent::eX, CACHED_X},
        {VectorComponent::eY, CACHED_Y},
        {VectorComponent::eZ, CACHED_Z},
        {VectorComponent::eW, CACHED_W},
    };
    for (const auto& [geomCmp, cmpName] : kCmpMappings)
    {
        std::optional<double> valueSource = try_get_component(value, geomCmp);
        if (!valueSource)
        {
            if (try_set_entity_component_field(comp, cmpName, 0)) return false;
            break;
        }
        bool success = try_set_entity_component_field(comp, cmpName, *valueSource);
        if (!success) return false;
    }
    return true;
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
