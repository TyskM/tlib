
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/span.h>

template <typename T, size_t Extent = eastl::dynamic_extent>
using Span = eastl::span<T, Extent>;