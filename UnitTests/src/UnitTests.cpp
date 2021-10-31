#include "UnitTests.h"

namespace hsTest
{

//------------------------------------------------------------------------------
TestCollection g_TestCollection;

//------------------------------------------------------------------------------
void TestResult::AddFail()
{
    ++failCount_;
}

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
    int failCount = 0;
    while (test)
    {
        TestResult result(test);

        test->Run(result);

        failCount += result.GetFailCount();

        // No new errors, we must have succeeded
        if (result.GetFailCount() == 0)
            printf("SUCCESS: %s\n", test->GetName());

        test = test->GetNext();
    }

    printf("\n------------------------------------------------------------------------------\n");
    if (failCount == 0)
    {
        printf("Success, all tests have passed!\n");
    }
    else
    {
        printf("Fail, %d errors!\n", failCount);
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
