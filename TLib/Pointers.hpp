#pragma once

#include <memory>
#include <EASTL/safe_ptr.h>

//// Shared Ptr
template <typename T>
using SharedPtr = std::shared_ptr<T>;

using std::make_shared;
template <typename T, typename... Args>
auto makeShared(Args&... args)
{ return make_shared<T>(args...); }

//// Weak Ptr
template <typename T>
using WeakPtr = std::weak_ptr<T>;

//// Unique Ptr
using std::make_unique;
template <typename T, typename... Args>
auto makeUnique(Args&&... args)
{ return make_unique<T>(args...); }

template <typename T, class Deleter = std::default_delete<T>>
using UPtr = std::unique_ptr<T, Deleter>;

// Example:
// using Material = UPtr<PxMaterial, Deleter<[](PxMaterial* mat) { mat->release(); }>>;
template <auto fn>
struct Deleter
{
    // Thanks Justin
    // https://stackoverflow.com/questions/19053351/how-do-i-use-a-custom-deleter-with-a-stdunique-ptr-member
    template <typename T>
    constexpr void operator()(T* arg) const
    { fn(arg); }
};

//template <typename T>
//struct UPtr : public std::unique_ptr<T>
//{
//    using std::unique_ptr<T>::unique_ptr;
//
//    void init()
//    {
//        auto temp = makeUnique<T>();
//        this->swap(temp);
//    }
//};

//// Safe Ptr
using SafeObj = eastl::safe_object;

template <typename T>
using SafePtr = eastl::safe_ptr<T>;

template<typename T, typename... Args>
SafePtr<T> makeSafePtr(Args&&... args)
{
    return SafePtr<T>(new T(std::forward<Args>(args)...));
}