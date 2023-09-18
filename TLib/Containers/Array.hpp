
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/array.h>

template <typename T, size_t Size = 1>
using Array = eastl::array<T, Size>;