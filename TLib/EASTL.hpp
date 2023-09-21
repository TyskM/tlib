
// EASTL Boilerplate

#pragma once

#include <new>
#include <EASTL/allocator.h>
#include <mimalloc.h>
using namespace eastl;

inline void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return new uint8_t[size]; }

inline void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return new uint8_t[size]; }

// Microsofts mimalloc
// https://github.com/microsoft/mimalloc
struct MiAllocator
{
    EASTL_ALLOCATOR_EXPLICIT MiAllocator(const char* = NULL) { }
    MiAllocator(const MiAllocator&) { }
    MiAllocator(const MiAllocator&, const char*) { }

    MiAllocator& operator=(const MiAllocator&) { return *this; }

    void* allocate(size_t size, int flags = 0)
    { return mi_malloc(size); }

    void* allocate(size_t size, size_t alignment, size_t alignmentOffset, int flags = 0)
    { return mi_malloc(size); }

    void  deallocate(void* p, size_t size)
    { mi_free_size(p, size); }

    const char* get_name() const { return ""; }
    void        set_name(const char*) { }

    inline bool operator==(const MiAllocator&) { return true;  }
    inline bool operator!=(const MiAllocator&) { return false; }
};


