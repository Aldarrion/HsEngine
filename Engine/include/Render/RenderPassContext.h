#pragma once

#include "Render/VkTypes.h"
#include "Math/hs_Math.h"

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
    RenderPassType  passType_;
    VkRenderPass    renderPass_;
};

//------------------------------------------------------------------------------
struct DrawData
{
    Mat44 transform_;
};

}
