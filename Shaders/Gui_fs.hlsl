#include "ShaderStructs/Common.h"
#include "GuiCommon.h"

SamplerState GuiSampler : register(s0, space0);

//------------------------------------------------------------------------------
float4 main(ps_in input) : SV_Target
{
    float4 col = GetTex2D(NonUniformResourceIndex(input.TexIdx)).Sample(GuiSampler, input.UV.xy);
    col *= input.Color;

    return col;
}
