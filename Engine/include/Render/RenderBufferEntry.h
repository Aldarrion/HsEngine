#pragma once

#include "Render/VkTypes.h"
#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
struct RenderBufferEntry
{
    VkBuffer buffer_{};
    uint offset_{};
    uint size_{};
};

}
