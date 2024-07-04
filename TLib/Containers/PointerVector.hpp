
#pragma once
#include <TLib/Pointers.hpp>
#include <TLib/Containers/Vector.hpp>

template <typename T, typename Allocator = MiAllocator>
struct PointerVector
{
    using value_type = T;

private:
    Vector<UPtr<T>> cont;

public:
    auto& container()
    { return cont; }

    T& emplace_back()
    { return *cont.emplace_back(makeUnique<T>()); }

    void push_back(UPtr<T>&& v)
    { cont.push_back(v); }

    void clear()
    { cont.clear(); }

    size_t size() const
    { return cont.size(); }

    T& at(size_t index)
    { return *cont.at(index); }

    const T& at(size_t index) const
    { return *cont.at(index); }

    T& operator[](size_t index)
    { return *cont[index]; }

    const T& operator[](size_t index) const
    { return *cont[index]; }

    void erase(size_t index)
    { cont.erase(cont.begin() + index); }

    auto begin()   noexcept       { return cont.begin()  ; }
    auto begin()   const noexcept { return cont.begin()  ; }
    auto cbegin()  const noexcept { return cont.cbegin() ; }
    auto end()     noexcept       { return cont.end()    ; }
    auto end()     const noexcept { return cont.end()    ; }
    auto cend()    const noexcept { return cont.cend()   ; }
    auto rbegin()                 { return cont.rbegin() ; }
    auto rbegin()  const noexcept { return cont.rbegin() ; }
    auto crbegin() const noexcept { return cont.crbegin(); }
    auto rend()    noexcept       { return cont.rend()   ; }
    auto rend()    const noexcept { return cont.rend()   ; }
    auto crend()   const noexcept { return cont.crend()  ; }
};