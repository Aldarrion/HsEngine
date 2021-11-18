#include "UnitTests.h"

#include "Threading/Atomic.h"

using namespace hsTest;
using namespace hs;

TEST_DEF(Atomic_LoadStore_Works)
{
    int x;
    AtomicStore(&x, 42);
    int newX = AtomicLoad(&x);

    TEST_TRUE(newX = 42);
}

TEST_DEF(Atomic_Exchange_ReturnsOldValue)
{
    int x;
    AtomicStore(&x, 42);
    int old = AtomicExchange(&x, 123);
    int n = AtomicLoad(&x);

    TEST_TRUE(old == 42);
    TEST_TRUE(n == 123);
}

TEST_DEF(Atomic_Increment_AddsOneAndReturnsResult)
{
    int x = 0;
    int n = AtomicIncrement(&x);

    TEST_TRUE(n == 1);
}

TEST_DEF(Atomic_Decrement_SubtractsOneAndReturnsResult)
{
    int x = 0;
    int n = AtomicDecrement(&x);

    TEST_TRUE(n == -1);
}

TEST_DEF(Atomic_Add_AddsAndReturnsResult)
{
    int x = 100;
    int n = AtomicAdd(&x, 42);

    TEST_TRUE(n == 142);
}

TEST_DEF(Atomic_Subtract_SubtractsAndRetunsResult)
{
    int x = 142;
    int n = AtomicSubtract(&x, 42);

    TEST_TRUE(n == 100);
}

TEST_DEF(Atomic_CompareExchange_ExchangesOnTrue)
{
    int x = 123;
    int expected = 123;
    int desired = 42;

    bool exchanged = AtomicCompareExchange(&x, expected, desired);
    int n = AtomicLoad(&x);

    TEST_TRUE(exchanged);
    TEST_TRUE(n == 42);
}

TEST_DEF(Atomic_CompareExchange_DoesNothingOnFalse)
{
    int x = 123;
    int expected = 1;
    int desired = 42;

    bool exchanged = AtomicCompareExchange(&x, expected, desired);
    int n = AtomicLoad(&x);

    TEST_FALSE(exchanged);
    TEST_TRUE(n == 123);
}

