#pragma once

#include <iostream>
#include <boost/preprocessor.hpp>

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
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << std::endl; \
    abort(); \
}
#else
#define ASSERT(x) ((void)0);
#endif

#ifdef TLIB_DEBUG
#define ASSERTMSG(x, str) \
if( !(x) ) \
{ \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ \
    << '\n' << str << std::endl; abort(); \
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
    << '\n' << str << std::endl; \
}
#else
#define ASSERTWARN(x, str) ((void)0);
#endif

#pragma endregion

// Create operators for enums
#define FLAG_ENUM(name)                                                                                                             \
inline name  operator|  (name a, name b)                   { return static_cast<name>(static_cast<int>(a) | static_cast<int>(b)); } \
inline name& operator|= (name & a, const name & b)         { return a = a | b; }                                                    \
inline bool  operator&  (name a, name b)                   { return static_cast<int>(a) & static_cast<int>(b);  }                   \
template <typename B> inline bool operator<  (name a, B b) { return static_cast<int>(a) <  static_cast<int>(b); }                   \
template <typename B> inline bool operator<= (name a, B b) { return static_cast<int>(a) <= static_cast<int>(b); }                   \
template <typename B> inline bool operator>  (name a, B b) { return static_cast<int>(a) >  static_cast<int>(b); }                   \
template <typename B> inline bool operator>= (name a, B b) { return static_cast<int>(a) >= static_cast<int>(b); }

#define STRING_NAME(X) #X
#define STRING_VALUE(X) STRING_NAME(X)

#define COMMA_SEP(r, token, i, e) BOOST_PP_COMMA_IF(i) token(e)
#define WRAP(token, ...) BOOST_PP_SEQ_FOR_EACH_I(COMMA_SEP, token, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
