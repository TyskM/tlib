
// EASTL Boilerplate

#pragma once

#include <new>
#include <EASTL/allocator.h>
#include <mimalloc.h>
using namespace eastl;

/*
* Rendering 20k sprites and rects
* with mimalloc 59-65 fps
* with stl 38-43 fps
*/

#ifndef USESTLMALLOC

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return mi_malloc(size); }

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return mi_malloc(size); }

void operator delete[](void* p, size_t size)
{ mi_free_size(p, size); }

void operator delete[](void* p)
{ mi_free(p); }

#else

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return new uint8_t[size]; }

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{ return new uint8_t[size]; }

#endif
