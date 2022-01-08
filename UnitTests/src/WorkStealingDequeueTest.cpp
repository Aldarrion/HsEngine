#include "UnitTests.h"

#include "Containers/WorkStealingDequeue.h"

using namespace hsTest;
using namespace hs;

TEST_DEF(WorkStealingDequeue_CanDoBasicOperationsOnASingleThread)
{
    WorkStealingDequeue<int*, nullptr> queue;
    queue.Init(128);

    int a = 42;
    int b = 123;

    queue.Push(&a);
    queue.Push(&b);
    int* ret_b = queue.Pop();
    int* ret_a = queue.Steal();

    int* ret_null = queue.Pop();
    int* steal_null = queue.Steal();

    TEST_TRUE(a == *ret_a);
    TEST_TRUE(b == *ret_b);
    TEST_TRUE(ret_null == nullptr);
    TEST_TRUE(steal_null == nullptr);

    queue.Destroy();
}
