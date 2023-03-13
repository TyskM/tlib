
// https://plflib.org/colony.htm
#include <TLib/EASTL.hpp>
#include <TLib/thirdparty/plf/plf_colony.h>

template <typename T, class Allocator = mi_stl_allocator<T>, plf::colony_priority priority = plf::performance>
using Colony = plf::colony<T, Allocator, priority>;