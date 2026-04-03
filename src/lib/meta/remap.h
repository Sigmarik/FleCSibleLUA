#pragma once

#include <unordered_map>

namespace flua::meta
{

template <class NewKeyT, class OldKeyT, class ValueT>
std::map<NewKeyT, ValueT> remapHashToCache(const std::unordered_map<OldKeyT, ValueT>& map)
{
    std::map<NewKeyT, ValueT> result;
    for (const auto& [key, value] : map)
    {
        result[NewKeyT(key)] = value;
    }
    return result;
}

}
