
#pragma once
#include <variant>

template <typename T, typename VariantT>
constexpr bool is(const VariantT& variant)
{ return std::holds_alternative<T>(variant); }

template <typename T, typename VariantT>
const T* try_get(const VariantT& variant)
{
    if (is<T>(variant))
    { return &std::get<T>(variant); }
    return nullptr;
}

template <typename T, typename VariantT>
T* try_get(VariantT& variant)
{
    if (is<T>(variant))
    { return &std::get<T>(variant); }
    return nullptr;
}

template <class... Types>
struct Variant : std::variant<Types...>
{
    using std::variant<Types...>::variant;

    template <typename T>
    constexpr bool is() const
    { return std::holds_alternative<T>(*this); }

    template <typename T>
    const T& get() const
    { return std::get<T>(*this); }

    template <typename T>
    T& get()
    { return std::get<T>(*this); }

    template <typename T>
    const T* try_get() const
    {
        if (this->is<T>())
        { return &get<T>(); }
        return nullptr;
    }
    
    template <typename T>
    T* try_get()
    {
        if (this->is<T>())
        { return &get<T>(); }
        return nullptr;
    }
};
