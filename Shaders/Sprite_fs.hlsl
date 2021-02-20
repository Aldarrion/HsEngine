#include "ShaderStructs/Common.h"
#include "SpriteCommon.h"

SamplerState PointSampler : register(s0, space0);

float4 main(ps_in input) : SV_Target
{
    float4 col = GetTex2D(0).Sample(PointSampler, input.UV.xy);
    col *= input.Color;
    return col;
}
