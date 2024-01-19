
#pragma once
#include <optional>
template <class T>
using Optional = std::optional<T>;

using NullOption_t = std::nullopt_t;
constexpr inline NullOption_t nullOption{NullOption_t::_Tag{}};