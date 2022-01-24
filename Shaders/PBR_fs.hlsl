#include "ShaderStructs/Common.h"

//------------------------------------------------------------------------------
struct ps_in
{
    float2 UV       : TEXCOORD0;
    float3 Normal   : TEXCOORD1;
    float3 FragPos  : POSITION;
};

//------------------------------------------------------------------------------
ConstantBuffer<SceneData>   Scene   : register(b1, space2);
ConstantBuffer<PBRData>     PBR     : register(b3, space2); // Space3 maybe?

SamplerState SamplerAlbedo : register(s1, space0);

//------------------------------------------------------------------------------
static const float3 LightPositions[4] =
{
    float3(10, -10, 0),
    float3(-10, -10, 0),
    float3(-10, 10, 0),
    float3(10, 10, 0)
};

//------------------------------------------------------------------------------
static const float3 LightColors[4] =
{
    float3(1, 0.9, 0.7),
    float3(1, 0.9, 0.7),
    float3(1, 0.9, 0.7),
    float3(1, 0.9, 0.7),
};

//------------------------------------------------------------------------------
float3 FresnelShlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//------------------------------------------------------------------------------
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a         = roughness * roughness;
    float a2        = a * a;
    float NdotH     = max(dot(N, H), 0.0);
    float NdotH2    = NdotH * NdotH;

    float num   = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = HS_PI * denom * denom;

    return num / denom;
}

//------------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

//------------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0); // TODO optimize this, we calculate these max(dot... multiple times
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

#define R3(c) return float4(c, 1.0)
#define R2(c) return float4(c.x, c.y, 0.0, 1.0)
#define R1(c) return float4(c, c, c, 1.0)

//------------------------------------------------------------------------------
float4 main(ps_in input) : SV_Target
{
    float3 N = normalize(input.Normal);
    float3 V = normalize(Scene.ViewPos.xyz - input.FragPos);

    float3 Lo = (float3)0;
    float3 albedo = GetTex2D(0).Sample(SamplerAlbedo, input.UV.xy).rgb;
    albedo *= PBR.Albedo;

    float2 roughnessMetalness = GetTex2D(1).Sample(SamplerAlbedo, input.UV.xy).rg;
    float roughness = roughnessMetalness.r * PBR.Roughness;
    float metalness = roughnessMetalness.g * PBR.Metallic;

    for (int i = 0; i < 4; ++i)
    {
        float3 L = normalize(LightPositions[i] - input.FragPos);
        float3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);

        float dist = distance(LightPositions[i], input.FragPos);
        float attenuation = 1.0 / (dist * dist);
        attenuation = 1.0; // TODO do this correctly
        float3 radiance = LightColors[i] * attenuation;

        // Fresnel
        float3 F0   = float3(0.04, 0.04, 0.04); // Average value for all dielectrics
        F0          = lerp(F0, albedo, metalness);
        float3 F    = FresnelShlick(max(dot(H, V), 0.0), F0);

        // Normal distribution function
        float NDF = DistributionGGX(N, H, roughness);

        // Geometry function
        float G = GeometrySmith(N, V, L, roughness);

        // Cook-Torrance BRDF
        float3 num      = NDF * G * F;
        float denom     = 4.0 * NdotV * NdotL;
        float3 specular = num / max(denom, 0.001); // Avoid division by zero

        float3 kS = F;
        float3 kD = float3(1, 1, 1) - kS;

        // Metallic surfaces don't have a diffuse term, the light energy is absorbed instead
        kD *= 1.0 - metalness;

        Lo += (kD * albedo / HS_PI + specular) * radiance * NdotL;
    }

    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * PBR.AO;
    float3 color = ambient + Lo;

    return float4(color, 1.0);
}
