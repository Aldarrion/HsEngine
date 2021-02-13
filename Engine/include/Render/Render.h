#pragma once

#include "Config.h"

#include "World/Camera.h"

#include "Render/RenderPassContext.h"
#include "Render/VertexBufferEntry.h"
#include "Render/DynamicUniformBufferEntry.h"
#include "Render/VkTypes.h"

#include "Containers/Hash.h"
#include "Containers/Array.h"

#include "Math/hs_Math.h"

#include "Common/Pointers.h"
#include "Common/Enums.h"
#include "Common/Types.h"

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
#endif

#include <unordered_map> // TODO use custom hashmap

//------------------------------------------------------------------------------
bool CheckResult(VkResult result, const char* file, int line, const char* fun);

#define VKR_SUCCEED(x) CheckResult(x, __FILE__, __LINE__, #x)
#define VKR_CHECK(x) VKR_SUCCEED(x)
#define VKR_FAILED(x) !VKR_SUCCEED(x)

#if HS_LINUX
    struct GLFWwindow;
#endif

//------------------------------------------------------------------------------
namespace hs
{

//------------------------------------------------------------------------------
static constexpr uint SRV_SLOT_COUNT = 8;
static constexpr uint IMMUTABLE_SAMPLER_COUNT = 1;
static constexpr uint DYNAMIC_UBO_COUNT = 3;

//------------------------------------------------------------------------------
extern class Render* g_Render;

//------------------------------------------------------------------------------
RESULT CreateRender(uint width, uint height);
void DestroyRender();

//------------------------------------------------------------------------------
VkResult SetDiagName(VkDevice device, uint64 object, VkObjectType type, const char* name);

//------------------------------------------------------------------------------
class Shader;
class Material;
class Texture;
class ShaderManager;
class VertexBuffer;
class DynamicUBOCache;
class VertexBufferCache;
struct RenderPassContext;

class DrawCanvas;
class SpriteRenderer;
class DebugShapeRenderer;
class GuiRenderer;

class SerializationManager;

//
//------------------------------------------------------------------------------
struct Mesh
{
    // Some visual data
    // This is held by the resource manager
};

//------------------------------------------------------------------------------
struct VisualObject
{
    Mat44       transform_;
    Material*   material_;
    Mesh*       mesh_;
};

//------------------------------------------------------------------------------
// Render has an array of visual objects for each RenderPass type
// During each pass materials in each array are drawn
// Arrays may be shared between passes - depth pre pass + ambient pass etc.
// Objects in arrays need to be sorted by the material sortId
// Objects with the exact same material instance should be next to each other so we can use instancing
// We also need to consider mesh identity, so mesh + material is highest prio, we can move some of the per material data to InstaceData later

// How to solve that some materials don't need a mesh to work? Pass a null mesh? Dummy mesh?
// Material has textures, mesh has vertex and index buffers etc.
//

/*

How to work with special renderers, such as sprite and debug shape? Both can be kept for now since they are low prio but they can be merged with the rest later on.
However, there may be some special handling needed for sprites for example such as batching (altough they are transparent so sort is necessary).


During a render pass we want to iterate visual objects as we have them in the arrays,
there should probably be a preprocess step which can alter the order in these but it will not be needed right away.

This should be fine for now even if it's not very efficient since it sets all the state every time and draws one by one
If we have some dummy mesh for procedural geometry we can easily pass it here

for (VisualObject& vo : visualObjects)
{
    vo.material_->Draw(ctx, vo.mesh_);
}

*/



//------------------------------------------------------------------------------
enum DepthState
{
    DS_TEST = 1,
    DS_WRITE = 2,
};

//------------------------------------------------------------------------------
struct RenderState
{
    static constexpr uint MAX_CONST_BUFF = 1;
    static constexpr uint MAX_VERT_BUFF = 1;
    static constexpr uint INVALID_DESC = (uint)-1;
    static constexpr uint INVALID_HANDLE = (uint)-1;

    Shader*                 shaders_[PS_COUNT]{};
    uint                    fsTextures_[SRV_SLOT_COUNT]{};
    DynamicUBOEntry         dynamicUBOs_[DYNAMIC_UBO_COUNT]{};

    DynamicUBOEntry         bindlessUBO_{};

    VkBuffer                vertexBuffers_[MAX_VERT_BUFF];
    VkDeviceSize            vbOffsets_[MAX_VERT_BUFF];
    VkDescriptorSet         uboDescSet_{};

    uint                    vertexLayouts_[MAX_VERT_BUFF];

    VkrPrimitiveTopology    primitiveTopology_{};
    VkrCullMode             cullMode_{};

    uint                    depthState_{ DS_TEST | DS_WRITE };

    void Reset();
};

//------------------------------------------------------------------------------
class Render
{
    friend RESULT CreateRender(uint width, uint height);
    friend void DestroyRender();

public:
    RESULT OnWindowResized(uint width, uint height);
    RESULT ReloadShaders();
    #if HS_WINDOWS
        RESULT InitWin32(HWND hwnd, HINSTANCE hinst);
    #elif HS_LINUX
        RESULT InitLinux(GLFWwindow* window);
    #endif
    RESULT Init();
    RESULT InitImgui();

    void ClearPipelineCache();

    // Setting state
    template<PipelineStage stage>
    void SetShader(Shader* shader)
    {
        state_.shaders_[stage] = shader;
    }
    void SetTexture(uint slot, Texture* texture);
    void SetVertexBuffer(uint slot, const VertexBufferEntry& entry);
    void SetVertexLayout(uint slot, uint layoutHandle);
    void SetPrimitiveTopology(VkrPrimitiveTopology primitiveTopology);
    void SetDynamicUbo(uint slot, const DynamicUBOEntry& entry);
    void SetDepthState(uint state);

    // Drawing
    void Draw(const RenderPassContext& ctx, uint vertexCount, uint firstVertex);

    void Update(float dTime);

    VkDevice GetDevice() const;
    VmaAllocator GetAllocator() const;
    ShaderManager* GetShaderManager() const;
    VkCommandBuffer CmdBuff() const;
    uint64 GetCurrentFrame() const;
    uint64 GetSafeFrame() const;

    void DestroyLater(VkBuffer buffer, VmaAllocation allocation);

    void TransitionBarrier(
        VkImage img, VkImageSubresourceRange subresource,
        VkAccessFlags accessBefore, VkAccessFlags accessAfter,
        VkImageLayout layoutBefore, VkImageLayout layoutAfter,
        VkPipelineStageFlags stageBefore, VkPipelineStageFlags stageAfter
    );

    uint AddBindlessTexture(VkImageView view);

    const VkPhysicalDeviceProperties& GetPhysDevProps() const;

    DynamicUBOCache* GetUBOCache() const;
    VertexBufferCache* GetVertexCache() const;

    uint GetWidth() const;
    uint GetHeight() const;
    float GetAspect() const;
    Vec2 GetDimensions() const;

    void ResetState();

    //----------------------
    // Vertex layout manager
    uint GetOrCreateVertexLayout(VkPipelineVertexInputStateCreateInfo info);

    //----------------------
    // Camera
    const Camera& GetCamera() const;
    Camera& GetCamera();

    SpriteRenderer* GetSpriteRenderer() const;
    DebugShapeRenderer* GetDebugShapeRenderer() const;
    GuiRenderer* GetGuiRenderer() const;

    void RenderObject(VisualObject* object);
    void RenderObjects(Span<VisualObject> objects);
    void RenderObjects(Span<VisualObject*> objects);

private:
    static constexpr auto VK_VERSION = VK_API_VERSION_1_1;
    static constexpr uint VKR_INVALID = -1;

    static constexpr uint BB_IMG_COUNT = 2;

    #if HS_WINDOWS
        // Win32
        HINSTANCE   hinst_;
        HWND        hwnd_;
    #elif HS_LINUX
        GLFWwindow* wnd_;
    #endif

    uint                width_{};
    uint                height_{};

    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities_;

    // Core Vulkan
    VkInstance          vkInstance_{};
    VkPhysicalDevice    vkPhysicalDevice_{};
    VkDevice            vkDevice_{};

    VkPhysicalDeviceProperties vkPhysicalDeviceProperties_{};

    // Debug
    #if HS_DEBUG
        VkDebugReportCallbackEXT debugReportCallback_{};
    #endif

    // Swapchain
    VkSurfaceKHR        vkSurface_{};
    VkSwapchainKHR      vkSwapchain_{};
    uint                currentBBIdx_{};
    VkImage             bbImages_[BB_IMG_COUNT]{};
    VkImageView         bbViews_[BB_IMG_COUNT]{};
    VkFormat            swapChainFormat_{};

    VkImage             depthImages_[BB_IMG_COUNT]{};
    VmaAllocation       depthMemory_[BB_IMG_COUNT]{};
    VkImageView         depthViews_[BB_IMG_COUNT]{};

    uint64              frame_{};

    // Synchronization
    #if defined(VKR_USE_TIMELINE_SEMAPHORES)
        VkSemaphore         directQueueSemaphore_{};
        uint64              semaphoreValues[BB_IMG_COUNT]{ 1, 1 };
    #endif

    VkFence             directQueueFences_[BB_IMG_COUNT]{};
    VkFence             nextImageFence_;

    VkSemaphore         submitSemaphores_[BB_IMG_COUNT]{};

    // Queues
    uint                directQueueFamilyIdx_{ VKR_INVALID };
    VkQueue             vkDirectQueue_{};

    // Command buffers
    VkCommandPool       directCmdPool_{};
    VkCommandBuffer     directCmdBuffers_[BB_IMG_COUNT]{};

    VkRenderPass        mainRenderPass_{};
    VkFramebuffer       mainFrameBuffer_[BB_IMG_COUNT]{};

    VkRenderPass        overlayRenderPass_{};
    VkFramebuffer       overlayFrameBuffer_[BB_IMG_COUNT]{};

    // Descriptors
    VkDescriptorPool    bindlessPool_{};
    VkDescriptorSet     bindlessSet_{};
    uint                lastFreeBindlessIndex_{ 1 }; // 0 is invalid "null" descriptor

    VkDescriptorPool    immutableSamplerPool_{};
    VkDescriptorSet     immutableSamplerSet_{};

    VkDescriptorPool    dynamicUBODPool_[BB_IMG_COUNT]{};

    // Imgui
    // TODO(pavel): Rework this, how big descriptor pool does Imgui need? Can we use one of ours?
    VkDescriptorPool    imguiDescriptorPool_;

    // Pipelines
    /*struct PipelineKey
    {
        uint64 k[1];
    };*/
    using PipelineKey = uint64;
    std::unordered_map<PipelineKey, VkPipeline, FibonacciHash<PipelineKey>> pipelineCache_;

    // Caches
    UniquePtr<DynamicUBOCache>    uboCache_;
    UniquePtr<VertexBufferCache>  vbCache_;

    // Allocator
    VmaAllocator        allocator_;

    // Keep alive objects
    struct BufferToRelease
    {
        VkBuffer        buffer_;
        VmaAllocation   allocation_;
    };
    Array<VkPipeline>       destroyPipelines_[BB_IMG_COUNT];
    Array<BufferToRelease>  destroyBuffers_[BB_IMG_COUNT];

    // Shaders
    UniquePtr<ShaderManager>    shaderManager_{};
    VkDescriptorSetLayout       fsSamplerLayout_{};
    VkDescriptorSetLayout       bindlessTexturesLayout_{};
    VkDescriptorSetLayout       dynamicUBOLayout_{};
    VkPipelineLayout            pipelineLayout_{};

    RenderState state_;

    UniquePtr<DrawCanvas>       drawCanvas_;

    UniquePtr<SpriteRenderer>       spriteRenderer_;
    UniquePtr<DebugShapeRenderer>   debugShapeRenderer_;
    UniquePtr<GuiRenderer>          guiRenderer_;

    Array<VisualObject*>            renderObjects_[RPT_COUNT];

    RESULT CreateInstance();
    RESULT CreateSurface();
    RESULT FindPhysicalDevice();
    RESULT CreateDevice();
    RESULT CreateSwapchain();

    RESULT CreateMainRenderPass();
    RESULT CreateMainFrameBuffer();

    RESULT CreateOverlayRenderPass();
    RESULT CreateOverlayFrameBuffer();

    void DestroySurface();
    void DestroySwapchain();

    void DestroyRenderPass(VkRenderPass& renderPass);
    void DestroyFrameBuffer(VkFramebuffer* frameBufferArr);

    //----------------------
    // Vertex layout manager
    Array<VkPipelineVertexInputStateCreateInfo> vertexLayouts_;

    static PipelineKey StateToPipelineKey(const RenderPassContext& ctx, const RenderState& state);

    RESULT PrepareForDraw(const RenderPassContext& ctx);
    void AfterDraw();

    RESULT WaitForFence(VkFence fence);

    void Free();

    template<bool present, bool wait>
    void FlushGpu();

    //----------------------
    // Serialization
    UniquePtr<SerializationManager> serializationManager_;

    //----------------------
    // Camera
    Camera camera_;
};



}
