#pragma once

#include "Config.h"
#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
void* Alloc(uint64 size);
void* AllocAligned(uint64 size, uint64 alignment);

//------------------------------------------------------------------------------
void* Realloc(void* memory, uint64 newSize);
void* ReallocAligned(void* memory, uint64 oldSize, uint64 newSize, uint64 alignment);

//------------------------------------------------------------------------------
void Free(void* memory);
void FreeAligned(void* memory);

}
