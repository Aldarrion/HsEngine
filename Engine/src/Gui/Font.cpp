#include "Gui/Font.h"

#include "Render/Texture.h"

#include "stb/stb_image.h"

#include <cstdio>

namespace hs
{

//------------------------------------------------------------------------------
Font::~Font() = default;

//------------------------------------------------------------------------------
RESULT Font::Init(const char* name)
{
    char path[256];
    sprintf(path, "fonts/%s.png", name);

    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        texture_ = new Texture(VK_FORMAT_R8G8B8A8_UNORM, VkExtent3D{ (uint)texWidth, (uint)texHeight, 1 }, Texture::Type::TEX_2D);

        auto texAllocRes = texture_->Allocate((void**)&pixels, name);
        stbi_image_free(pixels);

        if (HS_FAILED(texAllocRes))
            return R_FAIL; // TODO release resources
    }

    sprintf(path, "fonts/%s.font", name);

    FILE* fontConfig = fopen(path, "r");

    int glyphsPerRow;
    int glyphHeight;
    fscanf(fontConfig, "%d %d", &glyphsPerRow, &glyphHeight);

    fclose(fontConfig);

    const char glyphs[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

    const int glyphWidth = texture_->GetWidth() / glyphsPerRow;
    for (uint i = 0; i < HS_ARR_LEN(glyphs); ++i)
    {
        const int row = i / glyphsPerRow;
        const int col = i % glyphsPerRow;
        const Vec2 glyphPos(col * glyphWidth, row * glyphHeight);

        GlyphInfo info;
        info.size_ = Vec2(glyphWidth, glyphHeight);
        info.uvSize_ = Vec2((float)glyphWidth / texture_->GetWidth(), (float)glyphHeight / texture_->GetHeight());
        info.uvPos_ = Vec2(glyphPos.x / texture_->GetWidth(), 1 - ((glyphPos.y + glyphHeight) / texture_->GetHeight()));

        glyphs_.emplace(glyphs[i], info);
    }

    spaceWidth_ = glyphWidth;

    return R_OK;
}

//------------------------------------------------------------------------------
const Texture* Font::GetTexture() const
{
    return texture_;
}

//------------------------------------------------------------------------------
const float Font::GetSpaceWidth() const
{
    return spaceWidth_;
}

//------------------------------------------------------------------------------
const GlyphInfo* Font::GetGlyphInfo(uint codepoint) const
{
    auto glyph = glyphs_.find(codepoint);
    if (glyph == glyphs_.end())
        return nullptr;

    return &glyph->second;
}

}
