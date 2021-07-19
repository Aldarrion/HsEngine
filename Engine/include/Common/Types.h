#pragma once
#include "Config.h"

#include "Common/hs_Assert.h"
#include "Common/Enums.h"

#include <cstdint>
#include <cstddef>

namespace hs
{

using uint8     = uint8_t;
using uint16    = uint16_t;
using uint      = uint32_t;
using uint64    = uint64_t;

using int8      = int8_t;
using int16     = int16_t;
using int64     = int64_t;

class Texture;
class Shader;
class VertexBuffer;
class DynamicUniformBuffer;

//------------------------------------------------------------------------------
template<class DataT>
struct [[nodiscard]] Result
{
public:
    Result() = default;
    Result(RESULT res, DataT data)
        : data_(std::move(data))
        , result_(res)
    {
    }

    bool IsOk() const
    {
        return result_ == R_OK;
    }

    bool IsErr() const
    {
        return result_ != R_OK;
    }

    DataT GetValue() const
    {
        HS_ASSERT(IsOk());
        return data_;
    }

    DataT GetValue()
    {
        HS_ASSERT(IsOk());
        return data_;
    }

    explicit operator bool() const
    {
        return IsOk();
    }

private:
    DataT data_;
    RESULT result_{ R_FAIL };
};

//------------------------------------------------------------------------------
template<class DataT>
Result<DataT> Ok(DataT data)
{
    return Result<DataT>(R_OK, std::move(data));
}

//------------------------------------------------------------------------------
template<class DataT>
Result<DataT> Err()
{
    return {};
}

}


namespace internal
{

//------------------------------------------------------------------------------
template<class T, hs::uint N>
constexpr hs::uint ArrSizeInternal(T(&)[N])
{
    return N;
}

}

#define HS_ARR_LEN(arr) ::internal::ArrSizeInternal(arr)

#define HS_FAILED(res) (((int)(res)) < 0)
#define HS_SUCCEEDED(res) (((int)(res)) >= 0)
#if HS_DEBUG
    #define HS_CHECK(res) HS_ASSERT(HS_SUCCEEDED(res))
#else
    #define HS_CHECK(res) (void)res
#endif

