#pragma once

#include <iostream>
#include <magic_enum.hpp>

#define OS_WINDOWS _WIN32
#define OS_MAC     __APPLE__
#define OS_LINUX   __linux__ && __FreeBSD__

#pragma region Assertions

#ifndef NDEBUG
    #define TLIB_DEBUG 1
#endif

#ifdef TLIB_DEBUG
#define ASSERT(x) \
if( !(x) ) \
{ \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << '\n'; \
    abort(); \
}
#else
#define ASSERT(x) ((void)0);
#endif

#define RELASSERT(x) \
if( !(x) ) \
{ \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << '\n'; \
    abort(); \
}

#ifdef TLIB_DEBUG
#define ASSERTMSG(x, str) \
if( !(x) ) \
{ \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ \
    << '\n' << str << '\n'; abort(); \
}
#else
#define ASSERTMSG(x) ((void)0);
#endif

#if defined(TLIB_DEBUG) && !defined(TLIB_DISABLE_ASSERT_WARN)
// This assert does NOT abort the application
#define ASSERTWARN(x, str) \
if( !(x) ) \
{ \
    std::cout << "Warning in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ \
    << '\n' << str << '\n'; \
}
#else
#define ASSERTWARN(x, str) ((void)0);
#endif

#pragma endregion

template <typename T>
static constexpr T bit(T value)
{ return 1 << value; }

using namespace magic_enum::bitwise_operators;

/*
    Create operators for enums.
    Use like this:
        enum class RendererType : uint8_t
        {
            Sprite    = bit(0),
            Text      = bit(1),
            Primitive = bit(2)
        }; FLAG_ENUM(RendererType);
*/
#define FLAG_ENUM(name)                        \
template <>                                    \
struct magic_enum::customize::enum_range<name> \
{ static constexpr bool is_flags = true; };    
//inline name operator|(name a, name b)                                                          \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<name>(static_cast<UT>(a) | static_cast<UT>(b));                         \
//}                                                                                              \
//                                                                                               \
//inline name operator|=(name& a, const name& b)                                                 \
//{ return a = a | b; }                                                                          \
//                                                                                               \
//inline name operator~(const name& a)                                                           \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<name>(~static_cast<UT>(a));                                             \
//}                                                                                              \
//                                                                                               \
//template <typename B> inline name operator&(name a, B b)                                       \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) & static_cast<UT>(b);                                            \
//}                                                                                              \
//                                                                                               \
//template <typename B> inline name operator&=(name a, B b)                                      \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) &= static_cast<UT>(b);                                           \
//}                                                                                              \
//                                                                                               \
//template <typename B> inline name operator^(name a, B b)                                       \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<name>(static_cast<UT>(a) ^ static_cast<UT>(b));                         \
//}                                                                                              \
//                                                                                               \
//template <typename B> inline bool operator<(name a, B b)                                       \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) < static_cast<UT>(b);                                            \
//}                                                                                              \
//                                                                                               \
//template <typename B> inline bool operator<=(name a, B b)                                      \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) <= static_cast<UT>(b);                                           \
//}                                                                                              \
//                                                                                               \
//template <typename B> inline bool operator>(name a, B b)                                       \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) > static_cast<UT>(b);                                            \
//}                                                                                              \
//                                                                                               \
//template <typename B> inline bool operator>=(name a, B b)                                      \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) >= static_cast<UT>(b);                                           \
//}                                                                                              \
//template <typename B> inline bool operator<<(name a, B b)                                      \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) << static_cast<UT>(b);                                           \
//}                                                                                              \
//template <typename B> inline bool operator>>(name a, B b)                                      \
//{                                                                                              \
//    using UT = magic_enum::underlying_type<name>::type;                                        \
//    return static_cast<UT>(a) >> static_cast<UT>(b);                                           \
//}                                                                                              \

#define STRING_NAME(X) #X
#define STRING_VALUE(X) STRING_NAME(X)
