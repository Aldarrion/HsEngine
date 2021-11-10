#include "UnitTests.h"

#include "Threading/JobSystem.h"

using namespace hsTest;
using namespace hs;

struct TestState
{
    int x;
    int y;
};

using JobFunction = void (*)(void* arg);

template<class ArgT, void (*func)(ArgT*)>
void JobRunWrapper(void* arg)
{
    func(static_cast<ArgT*>(arg));
}

void TestFunction(TestState* state)
{
    // ...
}

struct Job
{
    // State, dependencies, dependents
    JobFunction jobFunction_;
    void*       arg_;
};

template<class ArgT, void (*func)(ArgT*)>
Job* MakeJob(ArgT* arg)
{
    Job* j = new Job;

    j->jobFunction_ = &JobRunWrapper<ArgT, func>;
    j->arg_ = arg;

    return j;
}

TEST_DEF(JobSystem)
{
    int x{};
    int y{};

    JobSystem* jobSystem = GetJobsystem();

    TestState state{};

    //Job* a = MakeJob([x, y](/*???*/){ /* return? */ });
    Job* b = MakeJob<TestState, &TestFunction>(&state)

    JobSystemAddDependency(jobSystem, a, b);

    JobSystemExecute(jobSystem, a);
    JobSystemExecute(jobSystem, b);

    JobSystemWorkUntil(jobSystem, a);
    JobSystemWorkUntil(jobSystem, b);
}
