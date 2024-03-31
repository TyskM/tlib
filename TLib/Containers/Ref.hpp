
#pragma once
#include <functional>

//template <typename T>
//using Ref = std::reference_wrapper<T>;

template <typename T>
struct Ref : std::reference_wrapper<T>
{
    using std::reference_wrapper<T>::reference_wrapper;

    bool operator==(const Ref& other) const { return &this->get() == &other.get(); }
    bool operator!=(const Ref& other) const { return !(this->operator==(other));   }
};