#include "ShaderStructs/Common.h"
#include "GuiCommon.h"

SamplerState GuiSampler : register(s0, space0);

//------------------------------------------------------------------------------
float4 main(ps_in input) : SV_Target
{
    eturn float4(1, 0, 0, 1);
    //float4 col = GetTex2D(NonUniformResourceIndex(input.TexIdx)).Sample(GuiSampler, input.UV.xy);
    float4 col = BindlessTex2D[NonUniformResourceIndex(input.TexIdx)].Sample(GuiSampler, input.UV.xy);
    col *= input.Color;

    return col;
}
