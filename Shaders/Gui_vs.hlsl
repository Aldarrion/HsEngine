#include "ShaderStructs/Common.h"
#include "GuiCommon.h"

struct vertex
{
    float4 Color : COLOR;
    float2 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
    uint TexIdx : TEXCOORD1;
};

ConstantBuffer<GuiData> Gui       : register(b1, space2);

vs_out main(vertex vert)
{
    vs_out o = vs_out(0);

    // Sceeenspace pos to NDC
    o.Pos.x = (vert.Pos.x / Gui.ScreenDimensions.x) * 2 - 1;
    o.Pos.y = (vert.Pos.y / Gui.ScreenDimensions.y) * 2 - 1;
    o.Pos.z = 0.5;
    o.Pos.w = 1;

    o.UV = vert.UV;
    o.Color = vert.Color;
    o.TexIdx = vert.TexIdx;

    return o;
}
