
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/bitset.h>
#include <TLib/Containers/Pair.hpp>
#include <TLib/Containers/InitializerList.hpp>

template <size_t Size, typename WordType = uint64_t>
struct Bitset : public eastl::bitset<Size, WordType>
{
    Bitset() = default;

    template<typename... Args>
    Bitset(InitializerList<Pair<WordType, bool>> flags)
    {
        for (auto& flag : flags)
        { this->set(flag.first, flag.second); }
    }
};
