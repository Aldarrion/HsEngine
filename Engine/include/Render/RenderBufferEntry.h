#pragma once

#include "Render/VkTypes.h"
#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
struct RenderBufferEntry
{
    VkBuffer buffer_{};
    int offset_{};
    int size_{};
};

}
