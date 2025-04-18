
#pragma once
#include <TLib/thirdparty/function2.hpp>

//     Function<void(int, float) const>
// Return type ~^    ^    ^      ^
// Parameters  ~~~~~|~~~~~|     ^
// Qualifier ~~~~~~~~~~~~~~~~~~~|

template <typename... Signatures>
using Function = fu2::function<Signatures...>;
