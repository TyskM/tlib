
#pragma once
#include <variant>
#include <any>

using NullState = std::monostate;
using NullPtr   = std::nullptr_t;

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
    using base = std::variant<Types...>;

    template <typename T>
    constexpr bool is() const
    { return std::holds_alternative<T>(*this); }

    template <typename T>
    const T& get() const { return std::get<T>(*this); }

    template <typename T>
    T& get() { return std::get<T>(*this); }

    template <typename T>
    const T& as() const { return std::get<T>(*this); }

    template <typename T>
    T& as() { return std::get<T>(*this); }

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

namespace eastl
{
    template<class... Types>
    struct hash<Variant<Types...>>
    {
        size_t operator()(const Variant<Types...>& key) const
        {
            return std::hash<std::variant<Types...>>()(key);
        }
    };
}

/*
    Use this to get the number of arguments in a variant:
        using YourVariant = Variant<NullState, Passable, Buildable>;
        constexpr size_t ArgumentCount = std::variant_size_v<YourVariant::base>;
*/
template <typename T, typename Variant>
struct get_index;
template <typename T, typename... Args>
struct get_index<T, std::variant<T, Args...>>
{ static constexpr size_t value = 0; };
template <typename T, typename U, typename... Args>
struct get_index<T, std::variant<U, Args...>>
{ static constexpr size_t value = 1 + get_index<T, std::variant<Args...>>::value; };

using Any = std::any;
