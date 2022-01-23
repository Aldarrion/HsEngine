#include "ShaderStructs/Common.h"

struct vs_in
{
    float3 Pos      : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD0;
};

struct vs_out
{
    float4 Pos      : SV_POSITION;
    //float3 Color : COLOR;
    float2 UV       : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
    float3 WorldPos : POSITION;
};

ConstantBuffer<SceneData>       Scene       : register(b1, space2);
ConstantBuffer<InstanceData>    Instance    : register(b2, space2);

vs_out main(vs_in vertex)
{
    vs_out o = (vs_out)0;

    o.Pos = mul(mul(Scene.VP, Instance.World), float4(vertex.Pos, 1));
    o.WorldPos = mul(Instance.World, float4(vertex.Pos, 1)).xyz;

    o.Normal = normalize(vertex.Normal);

    o.UV = vertex.UV;

    return o;
}
