#include "Threading/JobSystem.h"

#include "String/StringUtil.h"
#include "String/String.h"
#include "Threading/Thread.h"
#include "Threading/Atomic.h"
#include "Containers/Array.h"
#include "Common/Types.h"

#include <cstdio>

namespace hs
{

//------------------------------------------------------------------------------
JobSystem* g_JobSystem;

//------------------------------------------------------------------------------
enum class WorkerState
{
    RUNNING,
    TERMINATE,
};

//------------------------------------------------------------------------------
struct alignas(64) WorkerContext
{
    SmallArray<Job*, 128> queue_;
    WorkerState state_;
};

//------------------------------------------------------------------------------
class JobSystem
{
public:
    void Initialize(const JobSystemCreateParams& params);
    void Shutdown();

    void Execute(Span<Job*> jobs);
    void WorkUntil(Job* job);
    void AddDependency(Job* job, Job* dependsOn);

    // Move to a job ring allocator, it's not bound to the system since we don't intend to use indices to reference jobs
    Job* AllocateJob(int workerId);

    Array<Thread>           threadPool_;
    Array<WorkerContext>    workerContext_;
    Array<Array<Job>>       jobPools_;
    Array<int>              jobPoolIdx_;
};

//------------------------------------------------------------------------------
struct WorkerThreadParams
{
    int workerId_;
    JobSystem* jobSystem_;
};

//------------------------------------------------------------------------------
static void WorkerRun(void* params)
{
    // If there is work in queue, dequeue and work
    // If there is work in other queues, steal work
    // Else block and wait for wakeup
}

//------------------------------------------------------------------------------
void JobSystem::Initialize(const JobSystemCreateParams& params)
{
    HS_ASSERT(params.jobPoolSize_ > 2 && IsPow2(params.jobPoolSize_));

    threadPool_.Reserve(params.threadCount_);
    workerContext_.Resize(params.threadCount_);
    jobPools_.Resize(params.threadCount_);
    jobPoolIdx_.Resize(params.threadCount_);

    for (int i = 0; i < jobPools_.Count(); ++i)
        jobPools_.Resize(params.jobPoolSize_);

    WorkerThreadParams workerParams;
    workerParams.jobSystem_ = this;

    for (int i = 0; i < params.threadCount_; ++i)
    {
       char buffer[128];
       snprintf(buffer, 128, "Worker %d", i);

       workerParams.workerId_ = i;
       threadPool_.Add(ThreadCreate(&WorkerRun, &workerParams, StringView(buffer)));
    }
}

//------------------------------------------------------------------------------
void JobSystem::Shutdown()
{
    for (int i = 0; i < threadPool_.Count(); ++i)
    {
        ThreadDestroy(&threadPool_[i]);
    }
}

//------------------------------------------------------------------------------
Job* JobSystem::AllocateJob(int workerId)
{
    jobPoolIdx_[workerId] = ((jobPoolIdx_[workerId] + 1) & (jobPools_[workerId].Count() - 1));
    return &jobPools_[workerId][jobPoolIdx_[workerId]];
}

//------------------------------------------------------------------------------
void JobSystem::Execute(Span<Job*> jobs)
{
    // Add jobs without dependencies to queues
    // Ignore jobs with dependencies, or add them to an array for later check
    // If a job finishes it checks its dependants, decreases dependency count and
    // if a job has 0 dependencies it can be scheduled.
}

//------------------------------------------------------------------------------
void JobSystem::WorkUntil(Job* job)
{
    while (AtomicLoad(&job->state_) != JOB_STATE_DONE)
    {
        // TODO work on jobs here
    }
}

//------------------------------------------------------------------------------
void JobSystem::AddDependency(Job* task, Job* dependsOn)
{
}


//------------------------------------------------------------------------------
void JobSystemCreate(const JobSystemCreateParams& params)
{
    HS_ASSERT(!g_JobSystem);
    g_JobSystem = new JobSystem();
    g_JobSystem->Initialize(params);
}

//------------------------------------------------------------------------------
void JobSystemDestroy(JobSystem* jobSystem)
{
    HS_ASSERT(g_JobSystem);
    g_JobSystem->Shutdown();
    delete g_JobSystem;
    g_JobSystem = nullptr;
}

//------------------------------------------------------------------------------
void JobSystemExecute(JobSystem* jobSystem, Span<Job*> jobs)
{

}

//------------------------------------------------------------------------------
void JobSystemWait(JobSystem* jobSystem, Span<Job*> jobs)
{

}

}
