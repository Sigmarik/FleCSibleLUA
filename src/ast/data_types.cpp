#include "data_types.h"

#include <ios>
#include <sstream>

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
    return std::to_string(value);
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

static std::string to_string(const List& list)
{
    return "table: " + pointer_to_hex_string(&list);
}

static std::string to_string(const Dict& dict)
{
    return "table: " + pointer_to_hex_string(&dict);
}

static std::string to_string(const Entity& entity)
{
    return "entity: " + std::to_string(entity.entity);
}

static std::string to_string(const GenericClass& generic)
{
    return "generic of type " + std::to_string(generic.typeId) + ": " + pointer_to_hex_string(generic.ptr);
}

std::string to_string(const GenericValue& value)
{
    std::string result;
    std::visit([&](const auto& typedValue) { result = to_string(typedValue); }, value);
    return result;
}

}
