#include "tables.h"

#include "ast/data_types.h"

namespace flua::lib
{
using namespace flua;

struct PairIterator
{
    PairIterator(const data::GenericValue& table) : it(std::get<data::Table>(table)->begin()),
                                              end(std::get<data::Table>(table)->end()) {}

    data::Table::MapType::iterator it;
    data::Table::MapType::iterator end;

    void operator()(FluaState& state)
    {
        if (it == end) return;

        state.pushValue(it->first);
        state.pushRaw(*it->second);
        ++it;
    }
};

void ipairs(FluaState& state)
{
    auto* raw = state.getRaw(0);
    if (raw == nullptr || !std::holds_alternative<data::Table>(*raw))
        throw Error("Attempt to iterate over a non-table object " + state.asString(0));
    data::GenericValue iterator = data::Function(data::LibraryFunction(PairIterator(*raw)));
    state.pushRaw(iterator);
}
}
