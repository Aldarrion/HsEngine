#include "UnitTests.h"

#include "Containers/Array.h"

using namespace hsTest;
using namespace hs;

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

        TEST_TRUE(a.Count() == 32)
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

        Array<int> b{ a };
        b.Add(3);

        TEST_TRUE(b.Last() == 3);
        TEST_TRUE(a.Last() == 2);

        a.Remove(0);

        TEST_TRUE(a.Count() == 2);
        TEST_TRUE(b.Count() == 4);
    }

    void TestMove(TestResult& test_result)
    {
        ArrayT a;
        a.Add(0);
        a.Add(1);
        a.Add(2);

        ArrayT b{ std::move(a) };

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
};

//------------------------------------------------------------------------------
TEST_DEF(Array_Add_InsertsAtTheEnd)
{
    ArrayTester<Array<int>> tester;
    tester.TestAdd(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Insert_Works)
{
    ArrayTester<Array<int>> tester;
    tester.TestAdd(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_RemoveElements_Works)
{
    ArrayTester<Array<int>> tester;
    tester.TestRemove(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_FirstLast_Work)
{
    ArrayTester<Array<int>> tester;
    tester.TestFirstLast(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Data_ReturnPtrToFirst)
{
    ArrayTester<Array<int>> tester;
    tester.TestData(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_Clear_RemovesElements)
{
    ArrayTester<Array<int>> tester;
    tester.TestClear(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_CopyOperations_MakeCopy)
{
    ArrayTester<Array<int>> tester;
    tester.TestCopy(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_MoveOperations_Work)
{
    ArrayTester<Array<int>> tester;
    tester.TestMove(test_result);
}

//------------------------------------------------------------------------------
TEST_DEF(Array_MoveOnlyType_Works)
{
    ArrayTester<Array<int>> tester;
    tester.TestMoveOnlyType(test_result);
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

