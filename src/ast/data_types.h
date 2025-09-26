#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "flecs.h"

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

using GenericValue = std::variant<Nil, bool, double, std::string, List, Dict, Entity, GenericClass>;

struct List : public std::vector<GenericValue> {};
struct Dict : public std::unordered_map<std::string, GenericValue> {};

std::string to_string(const flua::data::GenericValue& value);

}
