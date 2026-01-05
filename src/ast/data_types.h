#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <functional>

#include "flecs.h"
#include "mem_utils/copyable_ptr.h"
#include "component_map/comp_map.h"

namespace flua
{
    class FluaState;
}

namespace flua::ast
{
struct Function;
}

namespace flua::data
{
using namespace flua;

struct Table;

using Entity = flecs::entity;

struct Nil {};

struct Function;

class GenericValue;

std::string to_string(const GenericValue& value);
bool to_bool(const GenericValue& value);

struct LuaFunction
{
    ast::Function* body = nullptr;
    std::shared_ptr<std::unordered_map<std::string, mem_utils::CopyMovePtr<GenericValue>>> frame{};
};

using LibraryFunction = std::function<void(FluaState*)>;

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

struct EntityComponent
{
    flecs::entity entity;
    std::string name;
};

class GenericValue : public std::variant<Nil, bool, double, std::string, Table, Entity, EntityComponent, Function>
{
    using std::variant<Nil, bool, double, std::string, Table, Entity, EntityComponent, Function>::variant;
};

std::string get_type_name(const GenericValue& value);

using MaybeFixedValuePtr = std::variant<GenericValue*, cmp_info::GenericComponentPtr>;

struct ValueSequence
{
    struct ValueBackrefPair
    {
        GenericValue value = Nil();
        MaybeFixedValuePtr reference = nullptr;
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
