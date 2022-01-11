#pragma once

#include "Config.h"

#include "Render/Types.h"

#include "Containers/Array.h"

#include "Math/Math.h"

#include "Common/Pointers.h"
#include "Common/Enums.h"
#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
struct RenderPassContext;
class RenderBuffer;

//------------------------------------------------------------------------------
enum class DrawMode
{
    Lines,
    CatmullRom
};

//------------------------------------------------------------------------------
class DrawCanvas
{
public:
    ~DrawCanvas();

    RESULT Init();
    void Draw(const RenderPassContext& ctx);

private:
    UniquePtr<RenderBuffer> linesBuffer_{};

    Shader*         lineVert_{};
    Shader*         lineFrag_{};
    uint            lineVertType_{};

    Array<Vec2>     points_;

    DrawMode        drawMode_{};
};

}
