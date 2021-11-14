#include "UnitTests.h"

#include "Threading/Atomic.h"
#include "Threading/Thread.h"

using namespace hsTest;
using namespace hs;

//------------------------------------------------------------------------------
int g_Counter = 0;
void TestThread(void*)
{
    AtomicStore(&g_Counter, 42);
}

//------------------------------------------------------------------------------
TEST_DEF(Thread_CreateAndDestroyThread)
{
    Thread t = ThreadCreate(&TestThread, nullptr, StringView("TestThreadName"));

    while (AtomicLoad(&g_Counter) != 42)
    {
        // Active wait for the other thread to write the value
    }

    ThreadDestroy(&t);
}
