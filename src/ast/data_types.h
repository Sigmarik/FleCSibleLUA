#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "flecs.h"
#include "mem_utils/copyable_ptr.h"

namespace flua::ast
{
struct Function;
}

namespace flua::data
{
using namespace flua;

struct Table;

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

using GenericValue = std::variant<Nil, bool, double, std::string, Table, Entity, GenericClass, Function>;

std::string to_string(const GenericValue& value);
bool to_bool(const GenericValue& value);

struct LuaFunction
{
    ast::Function* body = nullptr;
    std::shared_ptr<std::unordered_map<std::string, mem_utils::CopyMovePtr<GenericValue>>> frame{};
};

struct LibraryFunction
{
    std::string name{};
};

struct Function : std::variant<LuaFunction, LibraryFunction>
{
    using std::variant<LuaFunction, LibraryFunction>::variant;
};

struct Table : std::shared_ptr<std::unordered_map<std::string, std::unique_ptr<GenericValue>>>
{
    using MapType = std::unordered_map<std::string, std::unique_ptr<GenericValue>>;

    using std::shared_ptr<MapType>::shared_ptr;

    Table() : std::shared_ptr<MapType>(std::make_shared<MapType>()) {}
};

struct ValueSequence
{
    struct ValueBackrefPair
    {
        GenericValue value = Nil();
        GenericValue* reference = nullptr;
    };

    std::vector<ValueBackrefPair> sequence{};

    ValueSequence() = default;
    ValueSequence(const ValueSequence&) = default;
    ValueSequence(ValueSequence&&) = default;
    ValueSequence& operator=(const ValueSequence&) = default;
    ValueSequence& operator=(ValueSequence&&) = default;

    template<class T>
    ValueSequence& operator=(T&& thingy)
    {
        sequence = {ValueBackrefPair{.value = std::forward<T>(thingy), .reference = nullptr}};
        return *this;
    }

    void clear() { sequence.clear(); }
    void add(const GenericValue& value);
    void add(GenericValue&& value);
    void addReferenced(GenericValue& value);

    [[nodiscard]] GenericValue spit();
};

}
