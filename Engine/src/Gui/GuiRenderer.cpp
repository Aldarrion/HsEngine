#include "Gui/GuiRenderer.h"

#include "Gui/Font.h"

#include "Render/ShaderManager.h"
#include "Render/VertexBuffer.h"
#include "Render/DynamicUniformBuffer.h"
#include "Render/Texture.h"
#include "Render/Render.h"

#include "Common/Logging.h"

#include "Common.h"

namespace hs
{

//------------------------------------------------------------------------------
struct GuiVertex
{
    uint color_;
    Vec2 pos_;
    Vec2 uv_;
    uint texIdx_;
    uint _pad[2];
};

//------------------------------------------------------------------------------
uint GuiVertexLayout()
{
    static VkVertexInputAttributeDescription attributeDescriptions[4]{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = 4;

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = 12;

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32_UINT;
    attributeDescriptions[3].offset = 20;

    static VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(GuiVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = HS_ARR_LEN(attributeDescriptions);
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    return g_Render->GetOrCreateVertexLayout(vertexInputInfo);
}

//------------------------------------------------------------------------------
GuiRenderer::~GuiRenderer() = default;

//------------------------------------------------------------------------------
RESULT GuiRenderer::Init()
{
    guiVert_ = g_Render->GetShaderManager()->GetOrCreateShader("Gui_vs.hlsl");
    guiFrag_ = g_Render->GetShaderManager()->GetOrCreateShader("Gui_fs.hlsl");

    if (!guiVert_ || !guiFrag_)
        return R_FAIL;

    guiVertexLayout_ = GuiVertexLayout();

    return R_OK;
}

//------------------------------------------------------------------------------
void GuiRenderer::AddText(Font* font, StringView text, Vec2 pos)
{
    texts_.Add(Text{ font, text, pos });
}

//------------------------------------------------------------------------------
void GuiRenderer::Draw(const RenderPassContext& ctx)
{
    ctx_ = &ctx;
    for (const auto& text : texts_)
        DrawText(text.font_, StringView(text.text_), text.pos_);

    texts_.Clear();
}

//------------------------------------------------------------------------------
void GuiRenderer::DrawText(Font* font, StringView text, Vec2 pos)
{
    hs_assert(font);

    static constexpr uint VERTS_PER_GLYPH = 6; // Two triangles
    static constexpr uint BYTES_PER_GLYPH = VERTS_PER_GLYPH * sizeof(GuiVertex);

    VertexBufferCache* vbCache = g_Render->GetVertexCache();
    uint remainingSize = vbCache->GetRemainingBufferSize(sizeof(GuiVertex));
    uint remainingGlyphs = remainingSize / BYTES_PER_GLYPH;
    uint glyphsToRender = text.Length();

    //while (glyphsToRender > remainingGlyphs)
    //{
    //    // Add what we can and issue a draw call
    //    // Start new vertex buffer
    //    remainingSize = vbCache->GetMaxSize();
    //    //guiVbEntry_ = g_Render->GetVertexCache()->BeginAlloc(0, sizeof(GuiVertex), (void**)&guiVerts_);
    //}

    guiVbEntry_ = vbCache->BeginAlloc(0, sizeof(GuiVertex), (void**)&guiVerts_);

    auto SetVert = [](const Font* font, GuiVertex& vert, Vec2 pos, Vec2 uv)
    {
        vert.color_ = 0xffffffff;
        vert.pos_ = pos;
        vert.uv_ = uv;
        vert.texIdx_ = font->GetTexture()->GetBindlessIndex();
    };

    // TODO(pavel): Move this 5 away. It's the number of pixels per texel. It should come from the outside probably as a scale parameter.
    float spaceWidth = font->GetSpaceWidth() * 5;
    int vertsToDraw = 0;
    for (char c : text)
    {
        if (c == ' ')
        {
            pos.x += spaceWidth;
            continue;
        }

        const GlyphInfo* glyphInfo = font->GetGlyphInfo(c);
        if (!glyphInfo)
        {
            // TODO(pavel): Render tofu
            LOG_ERR("Could not find glyph %c", c);
            pos.x += spaceWidth;
            continue;
        }

        // TODO(pavel): Move this 5 away. It's the number of pixels per texel. It should come from the outside probably as a scale parameter.
        const Vec2 size = glyphInfo->size_ * 5;
        const Vec2 uvPos = glyphInfo->uvPos_;
        const Vec2 uvSize = glyphInfo->uvSize_;

        SetVert(font, guiVerts_[0], pos,                          uvPos);
        SetVert(font, guiVerts_[1], pos + Vec2(size.x, 0),        uvPos + Vec2(uvSize.x, 0));
        SetVert(font, guiVerts_[2], pos + Vec2(size.x, size.y),   uvPos + uvSize);

        SetVert(font, guiVerts_[3], pos,                          uvPos);
        SetVert(font, guiVerts_[4], pos + Vec2(size.x, size.y),   uvPos + uvSize);
        SetVert(font, guiVerts_[5], pos + Vec2(0, size.y),        uvPos + Vec2(0, uvSize.y));

        guiVerts_ += VERTS_PER_GLYPH;
        vertsToDraw += VERTS_PER_GLYPH;
        pos.x += size.x;
    }

    vbCache->EndAlloc();

    // Draw
    sh::GuiData* guiData;
    DynamicUBOEntry constBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(sh::GuiData), (void**)&guiData);
    guiData->ScreenDimensions = Vec2(g_Render->GetWidth(), g_Render->GetHeight());
    g_Render->GetUBOCache()->EndAlloc();

    g_Render->SetDynamicUbo(0, constBuffer);

    g_Render->SetVertexBuffer(0, guiVbEntry_);
    g_Render->SetVertexLayout(0, guiVertexLayout_);

    g_Render->SetShader<PS_VERT>(guiVert_);
    g_Render->SetShader<PS_FRAG>(guiFrag_);

    g_Render->Draw(*ctx_, vertsToDraw, 0);
}

}
