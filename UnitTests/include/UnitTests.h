#pragma once

#include <stdio.h>

namespace hsTest
{

class Test;
class TestCollection;

//------------------------------------------------------------------------------
class TestResult
{
public:
    TestResult(Test* test) : test_(test) {}

    Test* GetTest() const { return test_; }
    void AddFail();

    int GetFailCount() const { return failCount_; }

private:
    Test* test_{};
    int failCount_{};
};

//------------------------------------------------------------------------------
class TestCollection
{
public:
    void Add(Test* test);
    void RunTests();

private:
    Test* first_{};
    Test* last_{};
};

//------------------------------------------------------------------------------
extern TestCollection g_TestCollection;

//------------------------------------------------------------------------------
class Test
{
public:
    Test(const char* name);
    virtual void Run(TestResult& test_result) = 0;

    void SetNext(Test* next);
    Test* GetNext() const;

    const char* GetName() const;

private:
    Test* next_{};
    const char* name_{};
};

}

//------------------------------------------------------------------------------
#define TEST_DEF(name) \
class Test_##name : public hsTest::Test \
{ \
public: \
    Test_##name() : Test(#name) {} \
    void Run(TestResult& test_result) override; \
} object_Test_##name ; \
void Test_##name::Run(TestResult& test_result)

#define TEST_TRUE_MSG(expr, msg) do { if (!(expr)) { test_result.AddFail(); printf("FAIL: %s line: %d, %s\n", test_result.GetTest()->GetName(), __LINE__, msg); return; } } while (false)
#define TEST_TRUE(expr) TEST_TRUE_MSG(expr, #expr##" is false")
#define TEST_FALSE(expr) TEST_TRUE_MSG(!(expr), #expr##" is true")
#define TEST_FAIL(msg) TEST_TRUE_MSG(!(msg), msg)
