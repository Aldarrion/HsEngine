#include "Threading/JobSystem.h"

#include "String/StringUtil.h"
#include "String/String.h"

#include "Containers/Array.h"
#include "Common/Types.h"

#include <cstdio>

#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
    #include <processthreadsapi.h>
#endif

namespace hs
{

//------------------------------------------------------------------------------
// Thread

struct Thread
{
    #if HS_WINDOWS
        HANDLE hThread_;
    #else
    #endif
};

//------------------------------------------------------------------------------
using ThreadFunction = void (*)(void*);

//------------------------------------------------------------------------------
struct ThreadFunctionWrapperData
{
    ThreadFunction function_;
    void* data_;
};

#if HS_WINDOWS
    DWORD ThreadFunctionWin32Wrapper(void* data)
    {
        auto wrapperData = reinterpret_cast<ThreadFunctionWrapperData*>(data);
        wrapperData->function_(wrapperData->data_);
        return 0;
    }
#endif

Thread ThreadCreate(ThreadFunction threadFunction, void* args, StringView name)
{
    Thread thread;
    #if HS_WINDOWS
        ThreadFunctionWrapperData wrapperArgs;
        wrapperArgs.function_ = threadFunction;
        wrapperArgs.data_ = args;

        thread.hThread_ = CreateThread(
          nullptr,                      // Thread attributes
          0,                            // Stack size
          ThreadFunctionWin32Wrapper,   // Function to run in the thread
          &wrapperArgs,                 // Args to threadFunction
          0,                            // Creation flags
          nullptr                       // Thread ID
        );

        if (name.Length())
        {
            auto wname = MakeWString(name);
            auto hr = SetThreadDescription(thread.hThread_, wname.buffer_);

            HS_ASSERT(hr == S_OK);
        }

    #else
        HS_NOT_IMPLEMENTED;
        thread = {};
    #endif

    return thread;
}

//------------------------------------------------------------------------------
void ThreadDestroy(Thread& thread)
{
    #if HS_WINDOWS
        bool closed = CloseHandle(thread.hThread_);
        HS_ASSERT(closed);
    #else
        HS_NOT_IMPLEMENTED;
    #endif
}

//------------------------------------------------------------------------------

JobSystem* g_JobSystem;

constexpr int MAX_JOB_DEPENDENTS = 8;

//------------------------------------------------------------------------------
enum class JobState : int8
{
    INIT,
    WAITING,
    RUNNING,
    DONE,
};

//------------------------------------------------------------------------------
struct Job
{
    Job* dependents_[MAX_JOB_DEPENDENTS]; // TODO use index instead of pointer?
    JobState state_;
    int8 dependencies_;
};

//------------------------------------------------------------------------------
enum class WorkerState
{
    RUNNING,
    TERMINATE
};

//------------------------------------------------------------------------------
struct alignas(64) WorkerContext
{
    Array<Job> queue_;
    WorkerState state_;
};

//------------------------------------------------------------------------------
class JobSystem
{
public:
    void Initialize(const JobSystemCreateParams& params);
    void Shutdown();

    void Execute(Span<Job*> jobs);
    void Wait(Job* job);
    void AddDependency(Job* a, Job* b);

    Array<Thread> threadPool_;
    Array<WorkerContext> workerContext_;
    // Mutex(es) over queues
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
    WorkerThreadParams workerParams;
    workerParams.jobSystem_ = this;

    threadPool_.Reserve(params.threadCount_);
    workerContext_.Resize(params.threadCount_);

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
        ThreadDestroy(threadPool_[i]);
    }
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
void JobSystem::Wait(Job* job)
{

}

//------------------------------------------------------------------------------
void JobSystem::AddDependency(Job* a, Job* b)
{
    // a depends on b
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
