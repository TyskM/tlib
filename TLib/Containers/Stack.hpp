
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/stack.h>
#include <TLib/Containers/Vector.hpp>

template <typename T, typename Container = Vector<T>>
using Stack = eastl::stack<T, Container>;