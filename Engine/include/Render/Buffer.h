#pragma once

#include "Render/VkTypes.h"
#include "Render/RenderBufferEntry.h"
#include "Common/Enums.h"
#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
enum class RenderBufferType
{
    Uniform,
    Vertex,
    Index,
};

//------------------------------------------------------------------------------
enum class RenderBufferMemory
{
    DeviceLocal,
    HostToDevice,
    DeviceToHost,
};

//------------------------------------------------------------------------------
class RenderBuffer
{
public:
    RESULT Init(RenderBufferType type, RenderBufferMemory memory, int size);
    void Free();

    void* Map();
    void Unmap();

    VkBuffer GetBuffer() const;
    int GetSize() const;

protected:
    VkBuffer            buffer_{};
    VmaAllocation       allocation_{};
    VkBufferView        view_{};
    int                 size_;
    RenderBufferType    type_;
};

//------------------------------------------------------------------------------
void RenderCopyBuffer(VkCommandBuffer cmdBuff, RenderBufferEntry dst, RenderBufferEntry src);

//------------------------------------------------------------------------------
class TempStagingBuffer
{
public:
    TempStagingBuffer(int size);
    RESULT Allocate(const void* data);

    VkBuffer GetBuffer() const;

private:
    VkBuffer        buffer_;
    VmaAllocation   allocation_;
    int             size_;
};

}

