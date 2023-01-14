#pragma once

#include <memory>
#include <EASTL/safe_ptr.h>

//// Shared Ptr
template <typename T>
using SharedPtr = std::shared_ptr<T>;

using std::make_shared;

//// Unique Ptr
template <typename T>
using UPtr = std::unique_ptr<T>;

using std::make_unique;

//// Safe Ptr
using SafeObj = eastl::safe_object;

template <typename T>
using SafePtr = eastl::safe_ptr<T>;

template<typename T, typename... Args>
SafePtr<T> makeSafePtr(Args&&... args)
{
    return SafePtr<T>(new T(std::forward<Args>(args)...));
}