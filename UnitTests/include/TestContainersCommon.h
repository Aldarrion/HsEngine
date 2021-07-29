#pragma once

namespace hsTest
{

//------------------------------------------------------------------------------
template<bool IS_MOVEABLE>
struct alignas(128) NotTrivialType
{
    int x_;
    int moveCount_{};
    int copyCount_{};

    NotTrivialType() = delete;
    NotTrivialType(int x) // Implicit by design
        : x_(x)
    {
    }

    NotTrivialType(const NotTrivialType& other)
    {
        x_ = other.x_;
        copyCount_ = 1 + other.copyCount_;
        moveCount_ = other.moveCount_;
    }

    NotTrivialType& operator=(const NotTrivialType& other)
    {
        x_ = other.x_;
        copyCount_ = 1 + other.copyCount_;
        moveCount_ = other.moveCount_;

        return *this;
    }

    template<class DummyT = void>
    NotTrivialType(NotTrivialType&& other,
        typename std::enable_if_t<IS_MOVEABLE, DummyT>* = 0)
    {
        x_ = other.x_;
        copyCount_ = other.copyCount_;
        moveCount_ = 1 + other.moveCount_;
    }

    template<class DummyT = NotTrivialType&>
    typename std::enable_if_t<IS_MOVEABLE, DummyT>
     operator=(NotTrivialType&& other)
    {
        x_ = other.x_;
        copyCount_ = other.copyCount_;
        moveCount_ = 1 + other.moveCount_;

        return *this;
    }

    bool operator==(const NotTrivialType<IS_MOVEABLE>& other) const
    {
        return x_ == other.x_;
    }
};

static_assert(!std::is_trivial_v<NotTrivialType<true>>);
static_assert(!std::is_trivial_v<NotTrivialType<false>>);

}
