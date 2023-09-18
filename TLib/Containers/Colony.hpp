
// https://plflib.org/colony.htm
#include <TLib/EASTL.hpp>
#include <TLib/thirdparty/plf/plf_colony.h>

template <typename T, class Allocator = MiAllocator, plf::colony_priority priority = plf::performance>
using Colony = plf::colony<T, Allocator, priority>;