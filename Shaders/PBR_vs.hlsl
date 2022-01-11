#include "ShaderStructs/Common.h"

struct vs_in
{
    float4 Pos      : POSITION;
    float4 Normal   : NORMAL;
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

vs_out main(vs_in vertex, uint vertID : SV_VERTEXID)
{
    vs_out o = (vs_out)0;

    o.Pos = mul(mul(Scene.VP, Instance.World), vertex.Pos);
    o.WorldPos = mul(Instance.World, vertex.Pos).xyz;

    o.Normal = normalize(vertex.Normal.xyz);

    return o;
}
