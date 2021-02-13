#pragma once

#include "Config.h"

#include "String/String.h"

namespace hs
{

//------------------------------------------------------------------------------
class Font;

//------------------------------------------------------------------------------
class Text
{
public:
    void SetFont(const Font* font);

    const String& GetText() const;
    void SetText(const String& text);

    void Render();

private:
    const Font* font_;
    String text_;
};

}
