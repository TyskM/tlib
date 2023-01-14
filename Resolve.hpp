
// https://stackoverflow.com/questions/4364599/c-overloaded-method-pointer

#pragma once

struct cv_none {};
struct cv_const {};
struct cv_volatile {};
struct cv_cv {};
struct ref_none {};
struct ref_rval {};
struct ref_lval {};

namespace detail
{

//
// resolve for free functions
//

template <typename... Ts>
struct resolve_free
{
    template <typename R>
    constexpr auto operator()(R (*f)(Ts...)) const { return f; }
};

//
// resolve for member functions (also free functions for convenience)
// unspecialized template accepts any cv qualifier, any reference qualifier
// and therefore cannot resolve an overload that differs only in qualification
//

template <typename... Ts>
struct resolve
    : resolve_free<Ts...>
    , resolve<cv_none, Ts...>
    , resolve<cv_const, Ts...>
    , resolve<cv_volatile, Ts...>
    , resolve<cv_cv, Ts...>
{
    using resolve_free<Ts...>::operator();
    using resolve<cv_none, Ts...>::operator();
    using resolve<cv_const, Ts...>::operator();
    using resolve<cv_volatile, Ts...>::operator();
    using resolve<cv_cv, Ts...>::operator();
};

//
// specializations with cv tag
// accept any reference qualifier
// cannot resolve overload that differs only in reference qualification
//

template <typename... Ts>
struct resolve<cv_none, Ts...>
    : resolve<cv_none, ref_none, Ts...>
    , resolve<cv_none, ref_rval, Ts...>
    , resolve<cv_none, ref_lval, Ts...>
{
    using resolve<cv_none, ref_none, Ts...>::operator();
    using resolve<cv_none, ref_rval, Ts...>::operator();
    using resolve<cv_none, ref_lval, Ts...>::operator();
};

template <typename... Ts>
struct resolve<cv_const, Ts...>
    : resolve<cv_const, ref_none, Ts...>
    , resolve<cv_const, ref_rval, Ts...>
    , resolve<cv_const, ref_lval, Ts...>
{
    using resolve<cv_const, ref_none, Ts...>::operator();
    using resolve<cv_const, ref_rval, Ts...>::operator();
    using resolve<cv_const, ref_lval, Ts...>::operator();
};

template <typename... Ts>
struct resolve<cv_volatile, Ts...>
    : resolve<cv_volatile, ref_none, Ts...>
    , resolve<cv_volatile, ref_rval, Ts...>
    , resolve<cv_volatile, ref_lval, Ts...>
{
    using resolve<cv_volatile, ref_none, Ts...>::operator();
    using resolve<cv_volatile, ref_rval, Ts...>::operator();
    using resolve<cv_volatile, ref_lval, Ts...>::operator();
};

template <typename... Ts>
struct resolve<cv_cv, Ts...>
    : resolve<cv_cv, ref_none, Ts...>
    , resolve<cv_cv, ref_rval, Ts...>
    , resolve<cv_cv, ref_lval, Ts...>
{
    using resolve<cv_cv, ref_none, Ts...>::operator();
    using resolve<cv_cv, ref_rval, Ts...>::operator();
    using resolve<cv_cv, ref_lval, Ts...>::operator();
};

//
// specializations with reference tag
// accept any cv qualifier
// cannot resolve overload that differs only in cv qualification
//

template <typename... Ts>
struct resolve<ref_none, Ts...>
    : resolve<cv_none, ref_none, Ts...>
    , resolve<cv_const, ref_none, Ts...>
    , resolve<cv_volatile, ref_none, Ts...>
    , resolve<cv_cv, ref_none, Ts...>
{
    using resolve<cv_none, ref_none, Ts...>::operator();
    using resolve<cv_const, ref_none, Ts...>::operator();
    using resolve<cv_volatile, ref_none, Ts...>::operator();
    using resolve<cv_cv, ref_none, Ts...>::operator();
};

template <typename... Ts>
struct resolve<ref_rval, Ts...>
    : resolve<cv_none, ref_rval, Ts...>
    , resolve<cv_const, ref_rval, Ts...>
    , resolve<cv_volatile, ref_rval, Ts...>
    , resolve<cv_cv, ref_rval, Ts...>
{
    using resolve<cv_none, ref_rval, Ts...>::operator();
    using resolve<cv_const, ref_rval, Ts...>::operator();
    using resolve<cv_volatile, ref_rval, Ts...>::operator();
    using resolve<cv_cv, ref_rval, Ts...>::operator();
};

template <typename... Ts>
struct resolve<ref_lval, Ts...>
    : resolve<cv_none, ref_lval, Ts...>
    , resolve<cv_const, ref_lval, Ts...>
    , resolve<cv_volatile, ref_lval, Ts...>
    , resolve<cv_cv, ref_lval, Ts...>
{
    using resolve<cv_none, ref_lval, Ts...>::operator();
    using resolve<cv_const, ref_lval, Ts...>::operator();
    using resolve<cv_volatile, ref_lval, Ts...>::operator();
    using resolve<cv_cv, ref_lval, Ts...>::operator();
};

//
// specializations with cv tag followed by reference tag
//

#define RESOLVE(CV_TAG, REF_TAG, QUALIFIER)                                     \
    template <typename... Ts>                                                     \
    struct resolve<CV_TAG, REF_TAG, Ts...>                                      \
    {                                                                             \
        template <typename R, typename T>                                         \
        constexpr auto operator()(R (T::*f)(Ts...) QUALIFIER) const { return f; } \
    };
RESOLVE(cv_none, ref_none, )
RESOLVE(cv_const, ref_none, const)
RESOLVE(cv_volatile, ref_none, volatile)
RESOLVE(cv_cv, ref_none, const volatile)
RESOLVE(cv_none, ref_rval, &&)
RESOLVE(cv_const, ref_rval, const&&)
RESOLVE(cv_volatile, ref_rval, volatile&&)
RESOLVE(cv_cv, ref_rval, const volatile&&)
RESOLVE(cv_none, ref_lval, &)
RESOLVE(cv_const, ref_lval, const&)
RESOLVE(cv_volatile, ref_lval, volatile&)
RESOLVE(cv_cv, ref_lval, const volatile&)
#undef RESOLVE

//
// specializations with reference tag followed by cv tag
//

#define RESOLVE(CV_TAG, REF_TAG)              \
    template <typename... Ts>                   \
    struct resolve<REF_TAG, CV_TAG, Ts...>    \
        : resolve<CV_TAG, REF_TAG, Ts...> {};
RESOLVE(cv_none, ref_none)
RESOLVE(cv_const, ref_none)
RESOLVE(cv_volatile, ref_none)
RESOLVE(cv_cv, ref_none)
RESOLVE(cv_none, ref_rval)
RESOLVE(cv_const, ref_rval)
RESOLVE(cv_volatile, ref_rval)
RESOLVE(cv_cv, ref_rval)
RESOLVE(cv_none, ref_lval)
RESOLVE(cv_const, ref_lval)
RESOLVE(cv_volatile, ref_lval)
RESOLVE(cv_cv, ref_lval)
#undef RESOLVE

} // namespace detail

//
// c++17 variable templates
//

template <typename... Ts>
constexpr detail::resolve_free<Ts...> resolve_free{};

template <typename... Ts>
constexpr detail::resolve<Ts...> resolve{};


// Examples:
// 
// bool free_func(int, int) { return 42; }
// char free_func(int, float) { return true; }
// struct foo {
//     void mem_func(int) {}
//     void mem_func(int) const {}
//     void mem_func(long double) const {}
// };
// 
// auto f1 = resolve<int, float>(free_func);
// auto f2 = resolve<long double>(&foo::mem_func);
// auto f3 = resolve<cv_none, int>(&foo::mem_func);
// auto f4 = resolve<cv_const, int>(&foo::mem_func);