
#pragma once
#include <EASTL/bitset.h>

template <size_t Size, typename WordType = uint64_t>
using Bitset = eastl::bitset<Size, WordType>;
