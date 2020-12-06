#include "ShaderStructs/Common.h"

static const float3 CubeVerts[] =
{
    float3(-1.0, 1.0, -1.0),
    float3(1.0, 1.0, -1.0),
    float3(1.0, 1.0, 1.0),
    float3(-1.0, 1.0, 1.0),
    float3(-1.0, -1.0, -1.0),
    float3(1.0, -1.0, -1.0),
    float3(1.0, -1.0, 1.0),
    float3(-1.0, -1.0, 1.0),
    float3(-1.0, -1.0, 1.0),
    float3(-1.0, -1.0, -1.0),
    float3(-1.0, 1.0, -1.0),
    float3(-1.0, 1.0, 1.0),
    float3(1.0, -1.0, 1.0),
    float3(1.0, -1.0, -1.0),
    float3(1.0, 1.0, -1.0),
    float3(1.0, 1.0, 1.0),
    float3(-1.0, -1.0, -1.0),
    float3(1.0, -1.0, -1.0),
    float3(1.0, 1.0, -1.0),
    float3(-1.0, 1.0, -1.0),
    float3(-1.0, -1.0, 1.0),
    float3(1.0, -1.0, 1.0),
    float3(1.0, 1.0, 1.0),
    float3(-1.0, 1.0, 1.0),
};

static const float3 CubeNormals[] =
{
    float3(0.0, 1.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, -1.0, 0.0),
    float3(0.0, -1.0, 0.0),
    float3(0.0, -1.0, 0.0),
    float3(0.0, -1.0, 0.0),
    float3(-1.0, 0.0, 0.0),
    float3(-1.0, 0.0, 0.0),
    float3(-1.0, 0.0, 0.0),
    float3(-1.0, 0.0, 0.0),
    float3(1.0, 0.0, 0.0),
    float3(1.0, 0.0, 0.0),
    float3(1.0, 0.0, 0.0),
    float3(1.0, 0.0, 0.0),
    float3(0.0, 0.0, -1.0),
    float3(0.0, 0.0, -1.0),
    float3(0.0, 0.0, -1.0),
    float3(0.0, 0.0, -1.0),
    float3(0.0, 0.0, 1.0),
    float3(0.0, 0.0, 1.0),
    float3(0.0, 0.0, 1.0),
    float3(0.0, 0.0, 1.0),
};

static const float3 CubeColors[] =
{
    float3(0.2, 1.0, 0.0),
    float3(0.2, 1.0, 0.0),
    float3(0.2, 1.0, 0.0),
    float3(1.0, 0.2, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(1.0, 1.0, 0.0),
    float3(1.0, 1.0, 0.0),
    float3(1.0, 1.0, 0.0),
    float3(1.0, 0.2, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.5, 0.7, 0.5),
    float3(0.5, 0.7, 0.5),
    float3(0.5, 0.7, 0.5),
    float3(0.1, 0.7, 0.5),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(0.1, 1.0, 0.0),
    float3(1.0, 0.2, 0.0),
};

static const uint CubeIndices[] =
{
    3, 1, 0,
    2, 1, 3,

    6, 4, 5,
    7, 4, 6,

    11, 9, 8,
    10, 9, 11,

    14, 12, 13,
    15, 12, 14,

    19, 17, 16,
    18, 17, 19,

    22, 20, 21,
    23, 20, 22
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

vs_out main(uint vertID : SV_VERTEXID)
{
    vs_out o = vs_out(0);

    float4 pos = float4(CubeVerts[CubeIndices[vertID]], 1);
    o.Pos = pos * Instance.World * Scene.VP;
    o.WorldPos = (pos * Instance.World).xyz;

    //o.Color = CubeColors[CubeIndices[vertID]];
    o.Normal = normalize(CubeNormals[CubeIndices[vertID]]);

    return o;
}
