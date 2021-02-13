#pragma once

#include "Config.h"

#include "Render/VertexBufferEntry.h"

#include "String/String.h"

#include "Math/hs_Math.h"

#include "Containers/Array.h"

#include "Common/Enums.h"

namespace hs
{

class Font;
class Shader;
struct GuiVertex;
struct RenderPassContext;

//------------------------------------------------------------------------------
class GuiRenderer
{
public:
    ~GuiRenderer();

    RESULT Init();

    void AddText(Font* font, StringView text, Vec2 pos);

    void Draw(const RenderPassContext& ctx);
    void DrawText(Font* font, StringView text, Vec2 pos);

private:
    struct Text
    {
        Font* font_;
        String text_;
        Vec2 pos_;
    };

    Array<Text> texts_;

    const RenderPassContext* ctx_;

    VertexBufferEntry guiVbEntry_;
    GuiVertex* guiVerts_;

    Shader* guiVert_;
    Shader* guiFrag_;
    uint guiVertexLayout_;
};

}
