#pragma once

// The allocator needs these anyway
#include "Render/hs_Vulkan.h"
#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#endif

#include "vma/vk_mem_alloc.h"
