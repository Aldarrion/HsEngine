#include "Render/Buffer.h"

#include "Render/Render.h"
#include "Render/Allocator.h"

namespace hs
{

//------------------------------------------------------------------------------
// RenderBuffer
//------------------------------------------------------------------------------
RenderBuffer::RenderBuffer(RenderBufferType type, uint size)
    : size_(size)
    , type_(type)
{
}

//------------------------------------------------------------------------------
RESULT RenderBuffer::Init()
{
    VkBufferCreateInfo bufferInfo{};
    switch (type_)
    {
        case RenderBufferType::Uniform:
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case RenderBufferType::Vertex:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        default:
            hs_assert(false);
            break;
    }

    bufferInfo.sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size           = size_;
    bufferInfo.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage         = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if (VKR_FAILED(vmaCreateBuffer(g_Render->GetAllocator(), &bufferInfo, &allocInfo, &buffer_, &allocation_, nullptr)))
        return R_FAIL;

    return R_OK;
}

//------------------------------------------------------------------------------
void RenderBuffer::Free()
{
    if (buffer_ && allocation_)
        vmaDestroyBuffer(g_Render->GetAllocator(), buffer_, allocation_);
}

//------------------------------------------------------------------------------
void* RenderBuffer::Map()
{
    void* mapped{};
    if (VKR_FAILED(vmaMapMemory(g_Render->GetAllocator(), allocation_, &mapped)))
        return nullptr;

    return mapped;
}

//------------------------------------------------------------------------------
void RenderBuffer::Unmap()
{
    vmaUnmapMemory(g_Render->GetAllocator(), allocation_);
}

//------------------------------------------------------------------------------
VkBuffer RenderBuffer::GetBuffer() const
{
    return buffer_;
}

//------------------------------------------------------------------------------
uint RenderBuffer::GetSize() const
{
    return size_;
}

//------------------------------------------------------------------------------
// TempStagingBuffer
//------------------------------------------------------------------------------
TempStagingBuffer::TempStagingBuffer(uint size)
    : size_(size)
{
}

//------------------------------------------------------------------------------
RESULT TempStagingBuffer::Allocate(void* data)
{
    // Create the buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size         = size_;
    bufferInfo.usage        = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    if (VKR_FAILED(vmaCreateBuffer(g_Render->GetAllocator(), &bufferInfo, &allocInfo, &buffer_, &allocation_, nullptr)))
        return R_FAIL;

    g_Render->DestroyLater(buffer_, allocation_);

    if (data)
    {
        void* mapped;
        if (VKR_FAILED(vmaMapMemory(g_Render->GetAllocator(), allocation_, &mapped)))
            return R_FAIL;

        memcpy(mapped, data, size_);

        vmaUnmapMemory(g_Render->GetAllocator(), allocation_);
    }

    return R_OK;
}

//------------------------------------------------------------------------------
VkBuffer TempStagingBuffer::GetBuffer() const
{
    return buffer_;
}

}

