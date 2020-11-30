#pragma once

#include "Config.h"

#include "Containers/Span.h"

#include "Common/Pointers.h"
#include "Common/Enums.h"
#include "Common/Types.h"
#include "Math/hs_Math.h"

namespace hs
{

struct RenderPassContext;

//------------------------------------------------------------------------------
class Material
{
public:
    virtual ~Material() = default;
    virtual RESULT Init() = 0;
    virtual void Draw(const RenderPassContext& ctx) = 0;
};

//------------------------------------------------------------------------------
struct SpriteDrawData
{
    Texture* texture_;
    Vec4 uvBox_;
    Vec2 size_;
    Mat44 world_;
};

//------------------------------------------------------------------------------
class SpriteMaterial : public Material
{
public:
    ~SpriteMaterial();

    RESULT Init() override;
    void Draw(const RenderPassContext& ctx) override;
    void DrawSprite(const RenderPassContext& ctx, const SpriteDrawData& data);

private:
    Shader* vs_{};
    Shader* fs_{};
    uint    vertexLayout_{};
};

//------------------------------------------------------------------------------
class DebugShapeMaterial : public Material
{
public:
    ~DebugShapeMaterial();

    RESULT Init() override;
    void Draw(const RenderPassContext& ctx) override;
    void DrawShape(const RenderPassContext& ctx, Span<const Vec3> verts, const Color& color);

private:
    Shader* shapeVert_{};
    Shader* shapeFrag_{};
    uint    shapeVertexLayout_{};
};




//------------------------------------------------------------------------------
// Old materials
//------------------------------------------------------------------------------
class TexturedTriangleMaterial : public Material
{
public:
    RESULT Init() override;
    void Draw(const RenderPassContext& ctx) override;

private:
    Shader*     triangleVert_{};
    Shader*     triangleFrag_{};
    Texture*    texture_{};
    Texture*    textureTree_{};
    Texture*    textureBox_{};
};


//------------------------------------------------------------------------------
class PhongMaterial : public Material
{
public:
    RESULT Init() override;
    void Draw(const RenderPassContext& ctx) override;

private:
    Shader*         phongVert_{};
    Shader*         phongFrag_{};
};

//------------------------------------------------------------------------------
class SkyboxMaterial : public Material
{
public:
    RESULT Init() override;
    void Draw(const RenderPassContext& ctx) override;

private:
    Shader*     skyboxVert_{};
    Shader*     skyboxFrag_{};

    Texture*    skyboxCubemap_{};
};

//------------------------------------------------------------------------------
class PBRMaterial : public Material
{
public:
    RESULT Init() override;
    void Draw(const RenderPassContext& ctx) override;

private:
    Shader*     pbrVert_{};
    Shader*     pbrFrag_{};
};

}
