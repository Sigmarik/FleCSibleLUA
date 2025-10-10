#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "flecs.h"

namespace flua::ast
{
struct Function;
}

namespace flua::data
{
using namespace flua;

struct List;
struct Dict;

struct GenericClass
{
    void* ptr = nullptr;
    uint32_t typeId = 0;
};

struct Entity
{
    flecs::entity_t entity;
};

struct Nil {};

struct Function;

using GenericValue = std::variant<Nil, bool, double, std::string, List, Dict, Entity, GenericClass, Function>;

struct LuaFunction
{
    ast::Function* body = nullptr;
    std::unordered_map<std::string, GenericValue> frame{};
};

struct LibraryFunction
{
    std::string name{};
};

struct Function : public std::variant<LuaFunction, LibraryFunction>
{
    using std::variant<LuaFunction, LibraryFunction>::variant;
};

struct List : std::vector<GenericValue> {};
struct Dict : std::unordered_map<std::string, GenericValue> {};

std::string to_string(const GenericValue& value);

}
