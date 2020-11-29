#ifdef __cplusplus
#pragma once

#include "Math/hs_Math.h"

namespace hs
{
#endif

#ifndef __cplusplus
    #define HS_PI           3.14159265359
    #define HS_PI_HALF      (HS_PI / 2)
    #define HS_TAU          (2 * HS_PI)

    #define Mat44 float4x4
    #define Vec3 float3
    #define Vec4 float4

    struct BindingUBO
    {
        uint4 SRV[2]; // TODO use constant
    };

    #define BindingIdx(x) Bindings.SRV[x >> 2][x & 3]

    Texture2D BindlessTex2D[] : register(t0, space1);
    ConstantBuffer<BindingUBO> Bindings : register(b0, space2);

    #define GetTex2D(x) BindlessTex2D[BindingIdx(0)]
#endif

#ifdef __cplusplus
namespace sh
{
#endif

//------------------------------------------------------------------------------
struct SceneData
{
    Mat44   VP;
    Vec4    ViewPos;
};

//------------------------------------------------------------------------------
struct SpriteData
{
    Mat44 World;
};

//------------------------------------------------------------------------------
struct PBRData
{
    Vec3 Albedo;
    float Metallic;
    float Roughness;
    float AO;
};

#ifdef __cplusplus
} // namespace shaders
}
#endif
