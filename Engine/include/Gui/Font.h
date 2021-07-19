#pragma once

#include "Config.h"

#include "Math/Math.h"

#include "Common/Enums.h"
#include "Common/Types.h"

#include <unordered_map>

namespace hs
{

//------------------------------------------------------------------------------
struct GlyphInfo
{
    Vec2 size_;
    Vec2 uvPos_;
    Vec2 uvSize_;
};

//------------------------------------------------------------------------------
//! Bitmap font for now, also create a TrueType font and maybe use a common base class
class Font
{
public:
    ~Font();

    RESULT Init(const char* name);

    [[nodiscard]] const Texture* GetTexture() const;
    [[nodiscard]] const float GetSpaceWidth() const;
    [[nodiscard]] const GlyphInfo* GetGlyphInfo(uint codepoint) const;

private:
    Texture* texture_;
    float spaceWidth_;
    std::unordered_map<uint, GlyphInfo> glyphs_;

};

}
