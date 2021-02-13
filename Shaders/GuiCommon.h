#if VS
struct vs_out
#else
struct ps_in
#endif
{
    #if VS
        float4 Pos      : SV_POSITION;
    #endif
    float4 Color    : COLOR;
    float2 UV       : TEXCOORD0;
    nointerpolation uint TexIdx     : TEXCOORD1;
};
