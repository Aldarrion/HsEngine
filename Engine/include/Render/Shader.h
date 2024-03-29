#pragma once

#include "Render/VkTypes.h"
#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
class Shader
{
public:
    VkShaderModule vkShader_;
    PipelineStage type_;
    uint16 id_;
};

}
