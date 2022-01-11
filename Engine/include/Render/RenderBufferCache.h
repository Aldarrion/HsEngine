#pragma once

#include "Config.h"

#include "Render/RenderBufferEntry.h"
#include "Render/Buffer.h"

#include "Containers/Array.h"

#include "Common/Types.h"

namespace hs
{

//------------------------------------------------------------------------------
class RenderBufferCache
{
public:
    RenderBufferCache(RenderBufferType cacheType);
    ~RenderBufferCache();

    RESULT Init();

    RenderBufferEntry BeginAlloc(int size, int align, void** data);

    template<class ItemT>
    RenderBufferEntry BeginAlloc(int count, ItemT** data);

    void EndAlloc();
    uint GetMaxSize() const;
    int GetRemainingBufferSize(uint align) const;

private:
    constexpr static uint BUFFER_SIZE = 512 * 1024;

    struct CacheEntry
    {
        RenderBuffer* buffer_{};
        uint64 safeToUseFrame_{};
        uint begin_{};
    };

    RenderBufferType cacheType_;
    Array<CacheEntry> entries_;
    int minAlignment_{};

    RESULT MakeEntry(CacheEntry& entry);
};

//------------------------------------------------------------------------------
template<class ItemT>
RenderBufferEntry RenderBufferCache::BeginAlloc(int count, ItemT** data)
{
    return BeginAlloc((int)sizeof(ItemT) * count, alignof(ItemT), (void**)data);
}

}
