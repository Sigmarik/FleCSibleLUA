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

    void operator()(FluaState* lua)
    {
        if (it == end) return;

        lua->pushValue(it->first);
        lua->pushRaw(*it->second);
        ++it;
    }
};

void ipairs(FluaState* lua)
{
    auto* raw = lua->getRaw(0);
    if (raw == nullptr || !std::holds_alternative<data::Table>(*raw))
        throw Error("Attempt to iterate over a non-table object " + lua->asString(0));
    data::GenericValue iterator = data::Function(data::LibraryFunction(PairIterator(*raw)));
    lua->pushRaw(iterator);
}
}
