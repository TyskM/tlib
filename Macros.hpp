#pragma once

#include <iostream>

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

inline void debugWriteln(const std::string& str) noexcept
{
    if constexpr (_DEBUG) { std::cout << str << "\n"; }
}

inline void debugWrite(const std::string& str) noexcept
{
    if constexpr (_DEBUG) { std::cout << str; }
}