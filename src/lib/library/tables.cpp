#include "tables.h"

#include "types/data_types.h"

namespace flua::lib
{
using namespace flua;

struct PairIterator
{
    explicit PairIterator(const data::GenericValue& table) : it(std::get<data::Table>(table)->begin()),
                                              end(std::get<data::Table>(table)->end()) {}

    data::Table::MapType::iterator it;
    data::Table::MapType::iterator end;

    void operator()(FluaState& state)
    {
        if (it == end) return;

        data::GenericValue rawKey = it->first;
        state.pushRaw(rawKey);
        state.pushRaw(*it->second);
        ++it;
    }
};

struct IPairIterator
{
    explicit IPairIterator(const data::GenericValue& table) : table(std::get<data::Table>(table)) {}

    const data::Table& table;
    unsigned index = 1;

    void operator()(FluaState& state)
    {
        auto it = table->find(mem_utils::PointerMappedString(std::to_string(index)));
        if (it == table->end()) return;

        data::GenericValue rawKey = it->first;
        state.pushRaw(rawKey);
        state.pushRaw(*it->second);
        ++index;
    }
};

void pairs(FluaState& state)
{
    auto* raw = state.getRaw(0);
    if (raw == nullptr || !std::holds_alternative<data::Table>(*raw))
        throw Error("Attempt to iterate over a non-table object " + state.asString(0));
    data::GenericValue iterator = data::Function(data::LibraryFunction(PairIterator(*raw)));
    state.pushRaw(iterator);
}

void ipairs(FluaState& state)
{
    auto* raw = state.getRaw(0);
    if (raw == nullptr || !std::holds_alternative<data::Table>(*raw))
        throw Error("Attempt to iterate over a non-table object " + state.asString(0));
    data::GenericValue iterator = data::Function(data::LibraryFunction(IPairIterator(*raw)));
    state.pushRaw(iterator);
}
}
