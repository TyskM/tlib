#pragma once

#include <iostream>

#ifdef _DEBUG
#define ASSERT(x) if( !(x) ) { std::cout << "Assertion in " << __FILE__ << " in function " << __func__ << " on line " << __LINE__ << std::endl; abort(); }
#else
#define ASSERT(x) ((void)0);
#endif