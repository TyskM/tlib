#pragma once

#include <iostream>


#define OS_WINDOWS _WIN32
#define OS_MAC     __APPLE__
#define OS_LINUX   __linux__ && __FreeBSD__

#pragma region Assertions

#ifndef _DEBUG
    #define _DEBUG 0
#endif

#ifdef _DEBUG
#define ASSERT(x) \
if( !(x) ) \
{ \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << std::endl; \
    abort(); \
}
#else
#define ASSERT(x) ((void)0);
#endif

#ifdef _DEBUG
#define ASSERTMSG(x, str) \
if( !(x) ) \
{ \
    std::cerr << "Assertion failed in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ \
    << '\n' << str << std::endl; abort(); \
}
#else
#define ASSERTMSG(x) ((void)0);
#endif

#if defined(_DEBUG) && !defined(TLIB_DISABLE_ASSERT_WARN)
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

// Create a bitwise OR operator for enums used as flags
#define FLAG_ENUM(enumName)                                                  \
inline enumName operator|(enumName a, enumName b)                            \
{ return static_cast<enumName>(static_cast<int>(a) | static_cast<int>(b)); } \
inline enumName& operator|=(enumName & a, const enumName & b)                \
{ return a = a | b; }                                                        \
inline bool operator&(enumName a, enumName b)                                \
{ return static_cast<int>(a) & static_cast<int>(b); }

#define STRING_MACRO_NAME(X) #X
#define STRING_MACRO_VALUE(X) STRING_MACRO_NAME(X)