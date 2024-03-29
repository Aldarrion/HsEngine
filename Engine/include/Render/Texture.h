#pragma once

#include "Render/VkTypes.h"
#include "Common/Enums.h"
#include "Common/Types.h"

namespace hs
{

class Texture
{
public:
    enum class Type
    {
        TEX_2D,
        TEX_CUBE,
    };

    static RESULT CreateTex2D(const char* file, const char* name, Texture** tex);

    void Init(VkFormat format, VkExtent3D size, Type type);
    RESULT Allocate(const void** data, const char* diagName = nullptr);
    void Free();

    VkImageView GetView() const;
    uint GetBindlessIndex() const;

    uint GetWidth() const;
    uint GetHeight() const;
    uint GetDepth() const;

private:
    VkImage         image_;
    VmaAllocation   allocation_;
    VkFormat        format_;
    VkExtent3D      size_;

    VkImageView     srv_{};
    uint            bindlessIdx_;
    Type            type_;
};

}
