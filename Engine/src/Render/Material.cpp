#include "Render/Material.h"

#include "Render/Render.h"
#include "Render/Texture.h"
#include "Render/ShaderManager.h"
#include "Render/Buffer.h"
#include "Render/RenderBufferCache.h"
#include "Render/VertexTypes.h"
#include "Input/Input.h"

#include "Render/Image.h"

#include "Common.h"

#include <string>

namespace hs
{

//------------------------------------------------------------------------------
struct SpriteVertex
{
    Vec4 position_;
    Vec2 uv_;
    uint color_;
    uint pad_[1];
};

//------------------------------------------------------------------------------
uint SpriteVertexLayout()
{
    static VkVertexInputAttributeDescription attributeDescriptions[3]{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = 16;

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_B8G8R8A8_UNORM;
    attributeDescriptions[2].offset = 24;

    static VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(SpriteVertex);
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
uint PosColVertLayout()
{
    static VkVertexInputAttributeDescription attributeDescriptions[2]{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_B8G8R8A8_UNORM;
    attributeDescriptions[1].offset = 16;

    static VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(SpriteVertex);
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
uint PbrVertexLayout()
{
    static VkVertexInputAttributeDescription attributeDescriptions[2]{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = 16;

    static VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ObjectVertex);
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
void SetSceneData()
{
    void* mapped;
    RenderBufferEntry constBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(sh::SceneData), sizeof(sh::SceneData), &mapped);
    auto ubo = (sh::SceneData*)mapped;

    Mat44 camMat = g_Render->GetCamera()->toCamera_;
    Mat44 projMat = camMat * g_Render->GetCamera()->toProjection_;
    ubo->VP = projMat;

    g_Render->GetUBOCache()->EndAlloc();
    g_Render->SetDynamicUbo(0, constBuffer);
}

//------------------------------------------------------------------------------
// Sprite Material
//------------------------------------------------------------------------------
SpriteMaterial::~SpriteMaterial() = default;

//------------------------------------------------------------------------------
RESULT SpriteMaterial::Init()
{
    //
    vs_ = g_Render->GetShaderManager()->GetOrCreateShader("Sprite_vs");
    fs_ = g_Render->GetShaderManager()->GetOrCreateShader("Sprite_fs");

    if (!vs_ || !fs_)
        return R_FAIL;

    //
    vertexLayout_ = SpriteVertexLayout();

    return R_OK;
}

//------------------------------------------------------------------------------
void SpriteMaterial::Draw(const RenderPassContext& ctx, const DrawData& drawData)
{
    HS_ASSERT(false);
}

//------------------------------------------------------------------------------
void SpriteMaterial::DrawSprite(const RenderPassContext& ctx, const SpriteDrawData& data)
{
    {
        SpriteVertex* mapped{};
        RenderBufferEntry vbEntry = g_Render->GetVertexCache()->BeginAlloc(6 * sizeof(SpriteVertex), sizeof(SpriteVertex), (void**)&mapped);

        Vec3 vertPos = Vec3::ZERO();

        mapped[0].position_ = Vec4{
            vertPos.x,
            vertPos.y,
            vertPos.z,
            1
        };
        mapped[0].uv_ = Vec2{ data.uvBox_.x, data.uvBox_.y + data.uvBox_.w };
        mapped[0].color_ = 0xffffffff;

        mapped[1].position_ = Vec4{
            data.size_.x + vertPos.x,
            vertPos.y,
            vertPos.z,
            1
        };
        mapped[1].uv_ = Vec2{ data.uvBox_.x + data.uvBox_.z, data.uvBox_.y + data.uvBox_.w };
        mapped[1].color_ = 0xffffffff;

        mapped[2].position_ = Vec4{
            data.size_.x + vertPos.x,
            data.size_.y + vertPos.y,
            vertPos.z,
            1
        };
        mapped[2].uv_ = Vec2{ data.uvBox_.x + data.uvBox_.z, data.uvBox_.y };
        mapped[2].color_ = 0xffffffff;

        mapped[3].position_ = Vec4{
            vertPos.x,
            vertPos.y,
            vertPos.z,
            1
        };
        mapped[3].uv_ = Vec2{ data.uvBox_.x, data.uvBox_.y + data.uvBox_.w };
        mapped[3].color_ = 0xffffffff;

        mapped[4].position_ = Vec4{
            data.size_.x + vertPos.x,
            data.size_.y + vertPos.y,
            vertPos.z,
            1
        };
        mapped[4].uv_ = Vec2{ data.uvBox_.x + data.uvBox_.z, data.uvBox_.y };
        mapped[4].color_ = 0xffffffff;

        mapped[5].position_ = Vec4{
            vertPos.x,
            data.size_.y + vertPos.y,
            vertPos.z,
            1
        };
        mapped[5].uv_ = Vec2{ data.uvBox_.x, data.uvBox_.y };
        mapped[5].color_ = 0xffffffff;

        g_Render->GetVertexCache()->EndAlloc();

        g_Render->SetVertexBuffer(0, vbEntry);
    }

    {
        void* mapped;
        RenderBufferEntry uniformBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(sh::SpriteData), sizeof(sh::SpriteData), &mapped);

        auto ubo = (sh::SpriteData*)mapped;
            ubo->World = data.world_;
        g_Render->GetUBOCache()->EndAlloc();

        g_Render->SetDynamicUbo(1, uniformBuffer);
    }

    SetSceneData();

    g_Render->SetVertexLayout(0, vertexLayout_);

    g_Render->SetTexture(0, data.texture_);

    g_Render->SetShader<PS_VERT>(vs_);
    g_Render->SetShader<PS_FRAG>(fs_);

    g_Render->Draw(ctx, 6, 0);
}


//------------------------------------------------------------------------------
// Debug shape Material
//------------------------------------------------------------------------------
DebugShapeMaterial::~DebugShapeMaterial() = default;

//------------------------------------------------------------------------------
RESULT DebugShapeMaterial::Init()
{
    shapeVert_ = g_Render->GetShaderManager()->GetOrCreateShader("Shape_vs");
    shapeFrag_ = g_Render->GetShaderManager()->GetOrCreateShader("Shape_fs");

    if (!shapeVert_ || !shapeFrag_)
        return R_FAIL;

    shapeVertexLayout_ = PosColVertLayout();

    return R_OK;
}

//------------------------------------------------------------------------------
void DebugShapeMaterial::Draw(const RenderPassContext& ctx, const DrawData& drawData)
{
    HS_ASSERT(false);
}

//------------------------------------------------------------------------------
struct DebugShapeVertex
{
    Vec4 position_;
    uint color_;
    uint pad_[3];
};

//------------------------------------------------------------------------------
void DebugShapeMaterial::DrawShape(const RenderPassContext& ctx, Span<const Vec3> verts, const Color& color)
{
    {
        DebugShapeVertex* mapped{};
        RenderBufferEntry vbEntry = g_Render->GetVertexCache()->BeginAlloc(verts.Count() * sizeof(DebugShapeVertex), sizeof(DebugShapeVertex), (void**)&mapped);

        for (uint i = 0; i < verts.Count(); ++i)
        {
            mapped[i].color_ = color.ToSrgbUint();
            mapped[i].position_ = verts[i].ToVec4Pos();
        }

        g_Render->GetVertexCache()->EndAlloc();
        g_Render->SetVertexBuffer(0, vbEntry);
    }

    SetSceneData();

    g_Render->SetVertexLayout(0, shapeVertexLayout_);

    g_Render->SetShader<PS_VERT>(shapeVert_);
    g_Render->SetShader<PS_FRAG>(shapeFrag_);
    g_Render->SetPrimitiveTopology(VkrPrimitiveTopology::LINE_STRIP);

    g_Render->Draw(ctx, verts.Count(), 0);
}

//------------------------------------------------------------------------------
// Textured triangle Material
//------------------------------------------------------------------------------
RESULT TexturedTriangleMaterial::Init()
{
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("textures/grass_tile.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        texture_ = new Texture(VK_FORMAT_R8G8B8A8_UNORM, VkExtent3D{ (uint)texWidth, (uint)texHeight, 1 }, Texture::Type::TEX_2D);

        auto texAllocRes = texture_->Allocate((void**)&pixels, "GrassTile");
        stbi_image_free(pixels);

        if (HS_FAILED(texAllocRes))
            return R_FAIL; // TODO release resources
    }

    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("textures/tree.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        textureTree_ = new Texture(VK_FORMAT_R8G8B8A8_UNORM, VkExtent3D{ (uint)texWidth, (uint)texHeight, 1 }, Texture::Type::TEX_2D);

        auto texAllocRes = textureTree_->Allocate((void**)&pixels, "Tree");
        stbi_image_free(pixels);

        if (HS_FAILED(texAllocRes))
            return R_FAIL; // TODO release resources
    }

    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load("textures/box.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        textureBox_ = new Texture(VK_FORMAT_R8G8B8A8_UNORM, VkExtent3D{ (uint)texWidth, (uint)texHeight, 1 }, Texture::Type::TEX_2D);

        auto texAllocRes = textureBox_->Allocate((void**)&pixels, "Box");
        stbi_image_free(pixels);

        if (HS_FAILED(texAllocRes))
            return R_FAIL; // TODO release resources
    }

    triangleVert_ = g_Render->GetShaderManager()->GetOrCreateShader("Triangle_vs");
    triangleFrag_ = g_Render->GetShaderManager()->GetOrCreateShader("Triangle_fs");

    if (!triangleVert_ || !triangleFrag_)
        return R_FAIL;

    return R_OK;
}

//------------------------------------------------------------------------------
void TexturedTriangleMaterial::Draw(const RenderPassContext& ctx, const DrawData& drawData)
{
    g_Render->SetShader<PS_VERT>(triangleVert_);
    g_Render->SetShader<PS_FRAG>(triangleFrag_);
    g_Render->SetTexture(0, texture_);
    g_Render->SetTexture(1, textureBox_);
    g_Render->SetTexture(2, textureTree_);
    g_Render->Draw(ctx, 3, 0);
}


//------------------------------------------------------------------------------
// Phong Material
//------------------------------------------------------------------------------
RESULT PhongMaterial::Init()
{
    phongVert_ = g_Render->GetShaderManager()->GetOrCreateShader("Phong_vs");
    phongFrag_ = g_Render->GetShaderManager()->GetOrCreateShader("Phong_fs");

    if (!phongVert_ || !phongFrag_)
        return R_FAIL;

    return R_OK;
}

//------------------------------------------------------------------------------
void PhongMaterial::Draw(const RenderPassContext& ctx, const DrawData& drawData)
{
    // TODO move scene CB elsewhere
    struct SceneData
    {
        Mat44   Projection;
        Vec4    ViewPos;
    };

    SceneData* ubo{};
    RenderBufferEntry constBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(SceneData), sizeof(SceneData), (void**)&ubo);

    ubo->Projection = g_Render->GetCamera()->toCamera_ * g_Render->GetCamera()->toProjection_;
    ubo->ViewPos    = g_Render->GetCamera()->pos_.ToVec4Pos();

    g_Render->GetUBOCache()->EndAlloc();

    g_Render->SetDynamicUbo(0, constBuffer);


    // This material setup
    g_Render->SetShader<PS_VERT>(phongVert_);
    g_Render->SetShader<PS_FRAG>(phongFrag_);

    g_Render->Draw(ctx, 3 * 12, 0);
}

//------------------------------------------------------------------------------
// Skybox Material
//------------------------------------------------------------------------------
RESULT SkyboxMaterial::Init()
{
    {
        stbi_uc* pixels[6]{};

        int texWidth, texHeight, texChannels;
        pixels[0] = stbi_load("textures/skybox/right.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        pixels[1] = stbi_load("textures/skybox/left.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        pixels[2] = stbi_load("textures/skybox/top.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        pixels[3] = stbi_load("textures/skybox/bottom.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        pixels[4] = stbi_load("textures/skybox/front.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        pixels[5] = stbi_load("textures/skybox/back.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        for (uint i = 0; i < HS_ARR_LEN(pixels); ++i)
        {
            if (!pixels[i])
            {
                HS_ASSERT(!"Could not load a skybox image");
                return R_FAIL;
            }
        }

        skyboxCubemap_ = new Texture(VK_FORMAT_R8G8B8A8_UNORM, VkExtent3D{ (uint)texWidth, (uint)texHeight, 1 }, Texture::Type::TEX_CUBE);

        auto texAllocRes = skyboxCubemap_->Allocate((void**)pixels, "Skybox");
        for (uint i = 0; i < HS_ARR_LEN(pixels); ++i)
            stbi_image_free(pixels[i]);

        if (HS_FAILED(texAllocRes))
            return R_FAIL; // TODO release resources
    }

    skyboxVert_ = g_Render->GetShaderManager()->GetOrCreateShader("Skybox_vs");
    if (!skyboxVert_)
        return R_FAIL;

    skyboxFrag_ = g_Render->GetShaderManager()->GetOrCreateShader("Skybox_fs");
    if (!skyboxFrag_)
        return R_FAIL;

    return R_OK;
}

//------------------------------------------------------------------------------
void SkyboxMaterial::Draw(const RenderPassContext& ctx, const DrawData& drawData)
{
    // TODO move scene CB elsewhere
    struct SceneData
    {
        Mat44   Projection;
        Vec4    ViewPos;
    };

    void* mapped;
    RenderBufferEntry constBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(SceneData), sizeof(SceneData), &mapped);
    auto ubo = (SceneData*)mapped;

    Mat44 camMat = g_Render->GetCamera()->toCamera_;
    camMat.SetPosition(Vec3{});

    Mat44 projMat = camMat * g_Render->GetCamera()->toProjection_;
    ubo->Projection = projMat;

    g_Render->GetUBOCache()->EndAlloc();

    g_Render->SetDynamicUbo(0, constBuffer);


    // This material setup
    g_Render->SetShader<PS_VERT>(skyboxVert_);
    g_Render->SetShader<PS_FRAG>(skyboxFrag_);

    // Bind skybox texture
    g_Render->SetTexture(0, skyboxCubemap_);

    g_Render->SetDepthState(false);

    g_Render->Draw(ctx, 3 * 12, 0);
}


//------------------------------------------------------------------------------
// PBR Material
//------------------------------------------------------------------------------
RESULT PBRMaterial::Init()
{
    pbrVert_ = g_Render->GetShaderManager()->GetOrCreateShader("PBR_vs");
    pbrFrag_ = g_Render->GetShaderManager()->GetOrCreateShader("PBR_fs");

    if (!pbrVert_ || !pbrFrag_)
        return R_FAIL;

    vertexLayout_ = PbrVertexLayout();

    return R_OK;
}

//------------------------------------------------------------------------------
void PBRMaterial::Draw(const RenderPassContext& ctx, const DrawData& drawData)
{
    // Scene data
    {
        sh::SceneData* scene{};
        RenderBufferEntry constBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(sh::SceneData), sizeof(sh::SceneData), (void**)&scene);

        scene->VP       = g_Render->GetCamera()->toCamera_ * g_Render->GetCamera()->toProjection_;
        scene->ViewPos  = g_Render->GetCamera()->pos_.ToVec4Pos();

        g_Render->GetUBOCache()->EndAlloc();
        g_Render->SetDynamicUbo(0, constBuffer);
    }

    // Instance data
    {
        sh::InstanceData* inst{};
        RenderBufferEntry instCBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(sh::InstanceData), sizeof(sh::InstanceData), (void**)&inst);

        inst->World = drawData.transform_;

        g_Render->GetUBOCache()->EndAlloc();
        g_Render->SetDynamicUbo(1, instCBuffer);
    }

    // PBR data
    {
        sh::PBRData* pbr{};
        RenderBufferEntry pbrConstBuffer = g_Render->GetUBOCache()->BeginAlloc(sizeof(sh::PBRData), sizeof(sh::PBRData), (void**)&pbr);

        pbr->Albedo = albedo_;
        pbr->Roughness = roughness_;
        pbr->Metallic = metallic_;
        pbr->AO = ao_;

        g_Render->GetUBOCache()->EndAlloc();
        g_Render->SetDynamicUbo(2, pbrConstBuffer);
    }

    // Material setup
    g_Render->SetVertexBuffer(0, drawData.object_->vertexBuffer_.buffer_);
    g_Render->SetVertexLayout(0, vertexLayout_);
    g_Render->SetIndexBuffer(0, drawData.object_->indexBuffer_.buffer_);

    g_Render->SetShader<PS_VERT>(pbrVert_);
    g_Render->SetShader<PS_FRAG>(pbrFrag_);

    // TODO get the data better
    g_Render->DrawIndexed(ctx, drawData.object_->indexBuffer_.buffer_.size_ / 4, 0, 0);
}

//------------------------------------------------------------------------------
float PBRMaterial::GetRoughness() const
{
    return roughness_;
}
//------------------------------------------------------------------------------
void PBRMaterial::SetRoughness(float roughness)
{
    roughness_ = roughness;
}
//------------------------------------------------------------------------------
float PBRMaterial::GetMetallic() const
{
    return metallic_;
}
//------------------------------------------------------------------------------
void PBRMaterial::SetMetallic(float metallic)
{
    metallic_ = metallic;
}
//------------------------------------------------------------------------------
float PBRMaterial::GetAo() const
{
    return ao_;
}
//------------------------------------------------------------------------------
void PBRMaterial::SetAo(float ao)
{
    ao_ = ao;
}
//------------------------------------------------------------------------------
const Vec3& PBRMaterial::GetAlbedo() const
{
    return albedo_;
}
//------------------------------------------------------------------------------
void PBRMaterial::SetAlbedo(const Vec3& albedo)
{
    albedo_ = albedo;
}

}
