#include "System/Memory.h"

#include "Math/Math.h"

#include <cstdlib>
#include <cstring>

namespace hs
{

//------------------------------------------------------------------------------
void* Alloc(uint64 size)
{
    return std::malloc(size);
}

//------------------------------------------------------------------------------
void* AllocAligned(uint64 size, uint64 alignment)
{
    HS_ASSERT(IsPow2(alignment));

    size = Align(size, alignment);

    #if HS_MSVC
        return _aligned_malloc(size, alignment);
    #else
        return std::aligned_alloc(alignment, size);
    #endif
}

//------------------------------------------------------------------------------
void* Realloc(void* memory, uint64 newSize)
{
    return realloc(memory, newSize);
}

//------------------------------------------------------------------------------
void* ReallocAligned(void* memory, uint64 oldSize, uint64 newSize, uint64 alignment)
{
    HS_ASSERT(IsPow2(alignment));

    newSize = Align(newSize, alignment);

    #if HS_MSVC
        return _aligned_realloc(memory, newSize, alignment);
    #else
        void* newMemory = std::aligned_alloc(alignment, newSize);
        memcpy(newMemory, memory, oldSize);
        std::free(memory);
    #endif
}

//------------------------------------------------------------------------------
void Free(void* memory)
{
    std::free(memory);
}

//------------------------------------------------------------------------------
void FreeAligned(void* memory)
{
    #if HS_MSVC
        _aligned_free(memory);
    #else
        std::free(memory);
    #endif
}

}
