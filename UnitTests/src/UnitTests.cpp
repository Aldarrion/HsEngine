#include "UnitTests.h"

namespace hsTest
{

//------------------------------------------------------------------------------
TestCollection g_TestCollection;
int g_ErrorCount{};

//------------------------------------------------------------------------------
void TestCollection::Add(Test* test)
{
    if (!last_)
    {
        first_ = test;
        last_ = first_;
    }
    else
    {
        last_->SetNext(test);
        last_ = test;
    }
}

//------------------------------------------------------------------------------
void TestCollection::RunTests()
{
    printf("Running unit tests\n\n");

    Test* test = first_;
    while (test)
    {
        int errorCount = g_ErrorCount;
        test->Run();

        // No new errors, we must have succeeded
        if (errorCount == g_ErrorCount)
            printf("SUCCESS: %s\n", test->GetName());

        test = test->GetNext();
    }

    printf("\n------------------------------------------------------------------------------\n");
    if (g_ErrorCount == 0)
    {
        printf("Success, all tests passed!\n");
    }
    else
    {
        printf("Fail, %d errors!\n", g_ErrorCount);
    }
}

//------------------------------------------------------------------------------
Test::Test(const char* name)
    : name_(name)
{
    g_TestCollection.Add(this);
}

//------------------------------------------------------------------------------
void Test::SetNext(Test* next)
{
    next_ = next;
}

//------------------------------------------------------------------------------
Test* Test::GetNext() const
{
    return next_;
}

//------------------------------------------------------------------------------
const char* Test::GetName() const
{
    return name_;
}

}
