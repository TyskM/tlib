
#pragma once
#include <TLib/EASTL.hpp>
#include <TLib/thirdparty/plf_hive.h>

template <typename T, class Allocator = std::allocator<T>>
using Hive = plf::hive<T, Allocator>;