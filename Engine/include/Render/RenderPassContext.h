#pragma once

#include "Render/VkTypes.h"

namespace hs
{

//------------------------------------------------------------------------------
enum RenderPassType
{
    RPT_MAIN,
    RPT_OVERLAY,
    RPT_COUNT,
};

//------------------------------------------------------------------------------
struct RenderPassContext
{
    RenderPassType passType_;
    VkRenderPass renderPass_;
};

}
