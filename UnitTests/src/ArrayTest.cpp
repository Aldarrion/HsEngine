#include "UnitTests.h"

#include "Containers/Array.h"

using namespace hsTest;
using namespace hs;

//------------------------------------------------------------------------------
DEF_TEST(Array_Add_InsertsAtTheEnd)
{
    Array<int> a;
    a.Add(0);
    a.Add(1);
    a.Add(2);

    TEST_TRUE(a[0] == 0);
    TEST_TRUE(a[1] == 1);
    TEST_TRUE(a[2] == 2);
}

//------------------------------------------------------------------------------
DEF_TEST(Array_Insert_Works)
{
    Array<int> a;
    a.Insert(0, 0);
    a.Insert(0, 1);
    a.Insert(0, 2);

    TEST_TRUE(a[0] == 2);
    TEST_TRUE(a[1] == 1);
    TEST_TRUE(a[2] == 0);


    a.Insert(1, 42);

    TEST_TRUE(a[1] == 42);


    a.Insert(a.Count(), 123);

    TEST_TRUE(a[a.Count() - 1] == 123);
}

//------------------------------------------------------------------------------
DEF_TEST(Array_RemoveElements_Works)
{
    Array<int> a;
    a.Add(0);
    a.Add(1);
    a.Add(2);

    a.Remove(0);

    TEST_TRUE(a.Count() == 2)
    TEST_TRUE(a[0] == 1);
    TEST_TRUE(a[1] == 2);

    a.RemoveLast();

    TEST_TRUE(a.Count() == 1);
    TEST_TRUE(a[0] == 1);
}

//------------------------------------------------------------------------------
DEF_TEST(Array_FirstLast_Work)
{
    Array<int> a;
    a.Add(0);
    a.Add(1);
    a.Add(2);

    TEST_TRUE(a.First() == 0);
    TEST_TRUE(a.Last() == 2);
}

//------------------------------------------------------------------------------
DEF_TEST(Array_Data_ReturnPtrToFirst)
{
    Array<int> a;
    a.Add(0);
    a.Add(1);
    a.Add(2);

    TEST_TRUE(*a.Data() == 0);
    TEST_TRUE(*(a.Data() + 1) == 1);
    TEST_TRUE(*(a.Data() + 2) == 2);
}

//------------------------------------------------------------------------------
DEF_TEST(Array_Clear_RemovesElements)
{
    Array<int> a;
    a.Add(0);
    a.Add(1);
    a.Add(2);

    a.Clear();

    TEST_TRUE(a.IsEmpty());
    TEST_TRUE(a.Capacity() > 0);
}

//------------------------------------------------------------------------------
DEF_TEST(Array_CopyOperations_MakeCopy)
{
    Array<int> a;
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

//------------------------------------------------------------------------------
DEF_TEST(Array_MoveOperations_Work)
{
    Array<int> a;
    a.Add(0);
    a.Add(1);
    a.Add(2);

    Array<int> b{ std::move(a) };

    b.Add(3);
    b.Add(4);
    b.Add(5);
    b.Add(6);

    TEST_TRUE(b.First() == 0);
    TEST_TRUE(b.Last() == 6);
    TEST_TRUE(b.Count() == 7);
}

