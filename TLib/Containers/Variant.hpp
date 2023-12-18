
#pragma once
#include <variant>

template <class... Types>
using Variant = std::variant<Types...>;

template <typename T, typename VariantT>
constexpr bool is(const VariantT& vt)
{ return std::holds_alternative<T>(vt); }