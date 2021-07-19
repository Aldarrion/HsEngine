#include "UnitTests.h"

#include "Containers/Array.h"

#include <vector>

using namespace hsTest;
using namespace hs;

//------------------------------------------------------------------------------
template<bool IS_MOVEABLE>
struct alignas(32) NotTrivialType
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

    template<class = std::enable_if_t<IS_MOVEABLE, void>>
    NotTrivialType(NotTrivialType&& other)
    {
        x_ = other.x_;
        copyCount_ = other.copyCount_;
        moveCount_ = 1 + other.moveCount_;
    }

    template<class = std::enable_if_t<IS_MOVEABLE, void>>
    NotTrivialType& operator=(NotTrivialType&& other)
    {
        x_ = other.x_;
        copyCount_ = other.copyCount_;
        moveCount_ = 1 + other.moveCount_;

        return *this;
    }

    bool operator==(const NotTrivialType& other)
    {
        return x_ == other.x_;
    }
};

static_assert(!std::is_trivial_v<NotTrivialType<true>>);
static_assert(!std::is_trivial_v<NotTrivialType<false>>);

//------------------------------------------------------------------------------
template<class ElementT, class ArrayT>
void AddToArray(int N, ElementT start, ArrayT& arr)
{
    ElementT item = start;
    for (int i = 0; i < N; ++i)
    {
        arr.Add(item);
        item++;
    }
}

//------------------------------------------------------------------------------
template<class ArrayT>
class ArrayTester
{
public:
    void TestAdd(TestResult& test_result)
    {
        ArrayT a;
        AddToArray(33, 0, a);

        TEST_TRUE(a[0] == 0);
        TEST_TRUE(a[1] == 1);
        TEST_TRUE(a[2] == 2);
        TEST_TRUE(a[32] == 32);
    }

    void TestInsert(TestResult& test_result)
    {
        ArrayT a;
        AddToArray(32, 0, a);

        a.Insert(0, 0);
        a.Insert(0, 1);
        a.Insert(0, 2);

        TEST_TRUE(a[0] == 2);
        TEST_TRUE(a[1] == 1);
        TEST_TRUE(a[2] == 0);
        TEST_TRUE(a[34] == 31);


        a.Insert(5, 42);

        TEST_TRUE(a[1] == 1);
        TEST_TRUE(a[5] == 42);


        a.Insert(a.Count(), 123);

        TEST_TRUE(a[a.Count() - 1] == 123);
    }

    void TestRemove(TestResult& test_result)
    {
        ArrayT a;
        AddToArray(33, 0, a);

        a.Remove(0);

        TEST_TRUE(a.Count() == 32);
        TEST_TRUE(a[0] == 1);
        TEST_TRUE(a[1] == 2);

        a.RemoveLast();

        TEST_TRUE(a.Count() == 31);
        TEST_TRUE(a[0] == 1);
    }

    void TestFirstLast(TestResult& test_result)
    {
        ArrayT a;
        AddToArray(33, 0, a);

        TEST_TRUE(a.First() == 0);
        TEST_TRUE(a.Last() == 32);
    }

    void TestData(TestResult& test_result)
    {
        ArrayT a;
        AddToArray(33, 0, a);

        TEST_TRUE(*a.Data() == 0);
        TEST_TRUE(*(a.Data() + 1) == 1);
        TEST_TRUE(*(a.Data() + 2) == 2);
    }

    void TestClear(TestResult& test_result)
    {
        ArrayT a;
        a.Add(0);
        a.Add(1);
        a.Add(2);

        a.Clear();

        TEST_TRUE(a.IsEmpty());
        TEST_TRUE(a.Capacity() > 0);
    }

    void TestCopy(TestResult& test_result)
    {
        ArrayT a;
        a.Add(0);
        a.Add(1);
        a.Add(2);

        auto aDataBefore = a.Data();
        ArrayT b{ a };
        TEST_TRUE(a.Data() == aDataBefore);

        b.Add(3);

        TEST_TRUE(b.Last() == 3);
        TEST_TRUE(a.Last() == 2);

        a.Remove(0);

        TEST_TRUE(a.Count() == 2);
        TEST_TRUE(b.Count() == 4);
        TEST_TRUE(a.Data() != b.Data());
    }

    void TestMove(TestResult& test_result)
    {
        ArrayT a;
        a.Add(0);
        a.Add(1);
        a.Add(2);

        auto aDataBefore = a.Data();
        ArrayT b{ std::move(a) };

        if (ArrayT::MemoryPolicy_t::IS_MOVEABLE)
            TEST_TRUE(a.Data() != aDataBefore);
        else
            TEST_TRUE(a.Data() == aDataBefore);

        b.Add(3);
        b.Add(4);
        b.Add(5);
        b.Add(6);


        TEST_TRUE(b.First() == 0);
        TEST_TRUE(b.Last() == 6);
        TEST_TRUE(b.Count() == 7);
    }

    void TestMoveOnlyType(TestResult& test_result)
    {
        struct MoveOnly
        {
            int x_;

            MoveOnly(int x) : x_(x) {}
            MoveOnly(const MoveOnly&) = delete;
            MoveOnly(MoveOnly&&) = default;

            MoveOnly& operator=(const MoveOnly&) = delete;
            MoveOnly& operator=(MoveOnly&&) = default;
        };

        MoveOnly mo(3);

        Array<MoveOnly> a;
        a.Add(MoveOnly(1));
        a.Add(MoveOnly(2));
        a.Add(std::move(mo));

        TEST_TRUE(a.Count() == 3);
        TEST_TRUE(a.First().x_ == 1);
        TEST_TRUE(a.Last().x_ == 3);
    }

    void TestSelfAsignment(TestResult& test_result)
    {
        ArrayT a;
        AddToArray(33, 0, a);

        auto aDataBefore = a.Data();
        a = a;

        TEST_TRUE(aDataBefore == a.Data());
        TEST_TRUE(a[0] == 0);
        TEST_TRUE(a[12] == 12);
        TEST_TRUE(a.Count() == 33);

        a = std::move(a);

        TEST_TRUE(aDataBefore == a.Data());
        TEST_TRUE(a[0] == 0);
        TEST_TRUE(a[12] == 12);
        TEST_TRUE(a.Count() == 33);
    }

    void TestDataAlignment(TestResult& test_result)
    {
        ArrayT a;
        AddToArray(33, 0, a);

        TEST_TRUE((uintptr)a.Data() % alignof(ArrayT::Item_t) == 0);
    }

    void TestGrowAfterReserve(TestResult& test_result)
    {
        ArrayT a;
        TEST_TRUE(a.Capacity() == 0);

        a.Reserve(17);
        TEST_TRUE(a.Capacity() == 17);

        a.Reserve(10);
        TEST_TRUE(a.Capacity() == 17);

        AddToArray(18, 0, a);
        TEST_TRUE(IsPow2(a.Capacity()));
    }
};

//------------------------------------------------------------------------------
#define TEST_ALL_ARRAYS(testName) \
    ArrayTester<Array<int>> tester; \
    tester.testName(test_result); \
    \
    ArrayTester<StaticArray<int, 128>> testerStatic; \
    testerStatic.testName(test_result);\
    \
    ArrayTester<Array<NotTrivialType<true>>> testerNotTrivial; \
    testerNotTrivial.testName(test_result); \
    \
    ArrayTester<StaticArray<NotTrivialType<true>, 128>> testerNotTrivialStatic; \
    testerNotTrivialStatic.testName(test_result);

//------------------------------------------------------------------------------
TEST_DEF(Array_Add_InsertsAtTheEnd)
{
    TEST_ALL_ARRAYS(TestAdd);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Insert_Works)
{
    TEST_ALL_ARRAYS(TestInsert);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_RemoveElements_Works)
{
    TEST_ALL_ARRAYS(TestRemove);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_FirstLast_Work)
{
    TEST_ALL_ARRAYS(TestFirstLast);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Data_ReturnPtrToFirst)
{
    TEST_ALL_ARRAYS(TestData)
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Clear_RemovesElements)
{
    TEST_ALL_ARRAYS(TestClear);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_CopyOperations_MakeCopy)
{
    TEST_ALL_ARRAYS(TestCopy);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_MoveOperations_Work)
{
    TEST_ALL_ARRAYS(TestMove);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_MoveOnlyType_Works)
{
    TEST_ALL_ARRAYS(TestMoveOnlyType);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_SelfAsignment_IsHandled)
{
    TEST_ALL_ARRAYS(TestSelfAsignment);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Data_IsAligned)
{
    TEST_ALL_ARRAYS(TestDataAlignment);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Grow_ResizesToPow2)
{
    ArrayTester<Array<int>> tester;
    tester.TestGrowAfterReserve(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(StaticArray_Move_IsDisabled)
{
    StaticArray<int, 8> a;
    a.Add(0);
    a.Add(1);
    a.Add(2);

    StaticArray<int, 8> b{ std::move(a) };

    b.Add(3);
    b.Add(4);
    b.Add(5);
    b.Add(6);

    TEST_TRUE(a.First() == 0);
    TEST_TRUE(a.Last() == 2);
    TEST_TRUE(a.Count() == 3);

    TEST_TRUE(b.First() == 0);
    TEST_TRUE(b.Last() == 6);
    TEST_TRUE(b.Count() == 7);
}

