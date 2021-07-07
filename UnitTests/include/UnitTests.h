#pragma once

#include <stdio.h>

namespace hsTest
{

extern int g_ErrorCount;

class Test;
class TestCollection;

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
    virtual void Run() = 0;

    void SetNext(Test* next);
    Test* GetNext() const;

    const char* GetName() const;

private:
    Test* next_{};
    const char* name_{};
};

}

//------------------------------------------------------------------------------
#define DEF_TEST(name) \
class Test_##name : public hsTest::Test \
{ \
public: \
    Test_##name() : Test(#name) {} \
    void Run() override; \
} object_Test_##name ; \
void Test_##name::Run()

#define TEST_TRUE_MSG(expr, msg) { if (!(expr)) { ++hsTest::g_ErrorCount; printf("FAIL: %s line: %d, %s\n", GetName(), __LINE__, msg); return; } }
#define TEST_TRUE(expr) TEST_TRUE_MSG(expr, #expr##" is false")
#define TEST_FALSE(expr) TEST_TRUE_MSG(!(expr), #expr##" is true")
#define TEST_FAIL(msg) TEST_TRUE_MSG(!(msg), msg)
