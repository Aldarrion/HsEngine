#include "Render/Buffer.h"

#include "Render/Render.h"
#include "Render/Allocator.h"
#include "Render/Vulkan.h"

namespace hs
{

//------------------------------------------------------------------------------
// RenderBuffer
//------------------------------------------------------------------------------
RESULT RenderBuffer::Init(RenderBufferType type, RenderBufferMemory memory, int size)
{
    size_ = size;
    type_ = type;

    VkBufferCreateInfo bufferInfo{};
    switch (type_)
    {
        case RenderBufferType::Uniform:
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case RenderBufferType::Vertex:
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case RenderBufferType::Index:
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        default:
            HS_NOT_IMPLEMENTED;
            break;
    }

    bufferInfo.sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size           = size_;
    bufferInfo.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};

    switch (memory)
    {
        // TODO(pavel): Don't foce VK_BUFFER_USAGE_TRANSFER, we may not use it
        case RenderBufferMemory::DeviceLocal:
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case RenderBufferMemory::HostToDevice:
            allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        default:
            HS_NOT_IMPLEMENTED;
            break;
    }

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
int RenderBuffer::GetSize() const
{
    return size_;
}

//------------------------------------------------------------------------------
void RenderCopyBuffer(VkCommandBuffer cmdBuff, RenderBufferEntry dst, RenderBufferEntry src)
{
    HS_ASSERT(dst.size_ == src.size_);

    VkBufferCopy region{};
    region.dstOffset = dst.offset_;
    region.srcOffset = src.offset_;
    region.size = src.size_;

    vkCmdCopyBuffer(cmdBuff, src.buffer_, dst.buffer_, 1, &region);
}

//------------------------------------------------------------------------------
// TempStagingBuffer
//------------------------------------------------------------------------------
TempStagingBuffer::TempStagingBuffer(int size)
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

