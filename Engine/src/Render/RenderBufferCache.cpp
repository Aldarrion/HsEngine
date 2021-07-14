#include "Render/RenderBufferCache.h"

#include "Render/Buffer.h"
#include "Render/Render.h"

#include "Common/Logging.h"

namespace hs
{

//------------------------------------------------------------------------------
RenderBufferCache::RenderBufferCache(RenderBufferType cacheType)
    : cacheType_(cacheType)
{
    switch (cacheType_)
    {
        case RenderBufferType::Uniform:
        {
            minAlignment_ = (uint)g_Render->GetPhysDevProps().limits.minUniformBufferOffsetAlignment;
            break;
        }
        case RenderBufferType::Vertex:
        {
            break;
        }
    }
}

//------------------------------------------------------------------------------
RenderBufferCache::~RenderBufferCache()
{
    for (int i = 0; i < entries_.Count(); ++i)
    {
        entries_[i].buffer_->Free();
        delete entries_[i].buffer_;
    }
}

//------------------------------------------------------------------------------
RESULT RenderBufferCache::MakeEntry(CacheEntry& entry)
{
    entry.buffer_ = new RenderBuffer(cacheType_, BUFFER_SIZE);
    RESULT res = entry.buffer_->Init();
    return res;
}

//------------------------------------------------------------------------------
RESULT RenderBufferCache::Init()
{
    CacheEntry entry;
    if (HS_FAILED(MakeEntry(entry)))
        return R_FAIL;

    entries_.Add(entry);

    return R_OK;
}

//------------------------------------------------------------------------------
RenderBufferEntry RenderBufferCache::BeginAlloc(uint size, uint align, void** data)
{
    align = Max(align, minAlignment_);
    entries_.First().begin_ = Align(entries_.First().begin_, align);

    if ((int)BUFFER_SIZE - (int)entries_.First().begin_ < (int)size)
    {
        if (entries_.Last().safeToUseFrame_ <= g_Render->GetCurrentFrame())
        {
            auto last = entries_.Last();
            entries_.Insert(0, last);
            entries_.RemoveLast();
            entries_.First().begin_ = 0;
        }
        else
        {
            CacheEntry newEntry;
            if (HS_FAILED(MakeEntry(newEntry)))
            {
                LOG_ERR("Failed to create new buffer entry");
                HS_ASSERT(false);
                return {};
            }
            entries_.Insert(0, newEntry);
        }
    }

    RenderBufferEntry result;
    result.buffer_ = entries_.First().buffer_->GetBuffer();
    result.offset_ = entries_.First().begin_;
    result.size_ = size;

    *data = (uint8*)entries_.First().buffer_->Map() + entries_.First().begin_;

    entries_.First().safeToUseFrame_ = g_Render->GetSafeFrame();
    entries_.First().begin_ += size;

    return result;
}

//------------------------------------------------------------------------------
void RenderBufferCache::EndAlloc()
{
    entries_.First().buffer_->Unmap();
}

//------------------------------------------------------------------------------
uint RenderBufferCache::GetMaxSize() const
{
    return BUFFER_SIZE;
}

//------------------------------------------------------------------------------
int RenderBufferCache::GetRemainingBufferSize(uint align) const
{
    return (int)BUFFER_SIZE - (int)Align(entries_.First().begin_, align);
}

}
