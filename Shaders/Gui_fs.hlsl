#include "ShaderStructs/Common.h"
#include "GuiCommon.h"

SamplerState GuiSampler : register(s0, space0);

//------------------------------------------------------------------------------
float4 main(ps_in input)
{
    //float4 col = GetTex2D(NonUniformResourceIndex(input.TexIdx)).Sample(GuiSampler, input.UV.xy);
    float4 col = GetTex2D(input.TexIdx).Sample(GuiSampler, input.UV.xy);
    col *= input.Color;

    return col;
}
