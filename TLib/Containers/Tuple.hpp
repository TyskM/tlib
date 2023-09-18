
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/tuple.h>

template <typename T, typename... Ts>
using Tuple = eastl::tuple<T, Ts...>;