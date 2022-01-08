#pragma once

#include <Config.h>

#include <Threading/Atomic.h>
#include <Math/Math.h>
#include <Common/Assert.h>
#include <Common/Types.h>

namespace hs
{

//------------------------------------------------------------------------------
/*!
Chase-Lev work stealing queue

Work stealing double-ended queue storing T returning specified invalid value
if an operation does not succeed.

Sources:
    * https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/
    * https://github.com/toffaletti/chase-lev/blob/master/chase_lev.cpp
*/
template <typename T, T INVALID_VALUE>
class alignas(64) WorkStealingDequeue
{
public:
    //------------------------------------------------------------------------------
    WorkStealingDequeue() = default;
    WorkStealingDequeue(const WorkStealingDequeue&) = delete;
    WorkStealingDequeue& operator=(const WorkStealingDequeue&) = delete;

    //------------------------------------------------------------------------------
    void Init(int32 size)
    {
        arrayCapacity_  = size;
        HS_ASSERT(IsPow2(arrayCapacity_));
        array_ = new T[size]{};
    }

    //------------------------------------------------------------------------------
    void Destroy()
    {
        delete[] array_;
    }

    //------------------------------------------------------------------------------
    void Push(T x)
    {
        const int32 b = AtomicLoad(&bottom_);
        const int32 t = AtomicLoad(&top_);

        HS_ASSERT(b - t < arrayCapacity_);

        array_[b & (arrayCapacity_ - 1)] = x;
        AtomicStore(&bottom_, b + 1);
    }

    //------------------------------------------------------------------------------
    T Pop()
    {
        int32 b = AtomicLoad(&bottom_) - 1;
        AtomicStore(&bottom_, b);
        int32 t = AtomicLoad(&top_);

        if (t <= b)
        {
            // Non-empty queue
            T result = array_[b & (arrayCapacity_ - 1)];
            if (t != b)
            {
                return result;
            }
            else // t == b
            {
                // Last element in queue
                if (!AtomicCompareExchange(&top_, t, t + 1))
                {
                    // Failed race
                    result = INVALID_VALUE;
                }

                AtomicStore(&bottom_, t + 1);
                return result;
            }
        }
        else // t > b
        {
            // Empty queue
            AtomicStore(&bottom_, t);
            return INVALID_VALUE;
        }
    }

    //------------------------------------------------------------------------------
    T Steal()
    {
        int32 t = AtomicLoad(&top_);
        int32 b = AtomicLoad(&bottom_);

        if (t < b)
        {
            // Non-empty queue
            T result = array_[t & (arrayCapacity_ - 1)];

            if (!AtomicCompareExchange(&top_, t, t + 1))
            {
                // Failed race with a pop or a steal
                return INVALID_VALUE;
            }

            return result;
        }
        else // t >= b
        {
            // Empty queue
            return INVALID_VALUE;
        }
    }

private:
    int32   top_{};
    int32   bottom_{};
    int32   arrayCapacity_;
    T*      array_;
};

}
