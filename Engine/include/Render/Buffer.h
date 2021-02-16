#pragma once

#include "Render/VkTypes.h"
#include "Common/Enums.h"
#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
enum class RenderBufferType
{
    Uniform,
    Vertex,
};

//------------------------------------------------------------------------------
class RenderBuffer
{
public:
    RenderBuffer(RenderBufferType type, uint size);

    RESULT Init();
    void Free();

    void* Map();
    void Unmap();

    VkBuffer GetBuffer() const;
    uint GetSize() const;

protected:
    VkBuffer            buffer_{};
    VmaAllocation       allocation_{};
    VkBufferView        view_{};
    uint                size_;
    RenderBufferType    type_;
};

//------------------------------------------------------------------------------
class TempStagingBuffer
{
public:
    TempStagingBuffer(uint size);
    RESULT Allocate(void* data);

    VkBuffer GetBuffer() const;

private:
    VkBuffer buffer_;
    VmaAllocation allocation_;
    uint size_;
};

}

