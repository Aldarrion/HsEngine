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
// Inplace function
//------------------------------------------------------------------------------

template<class RetT, class... ArgsT>
class FunctionTable
{
    using FunT = RetT(*)(Args&&...);
};

template<int STORAGE_SIZE = 32, int ALIGNMENT = 8>
class JobFunction
{
public:
    //template<class FunctionT>
    //JobFunction(FunctionT&& function)
    //{
    //    static const FunctionTable ftable{ inplace_function_detail::wrapper<C>{} };
    //    functionTable_ = std::addressof(ftable);

    //    ::new (std::addressof(storage_)) C{ std::forward<T>(closure) };
    //}

    template<class RetT, class... ArgsT>
    RetT Call(ArgsT&& args...)
    {

    }

private:
    alignas(ALIGNMENT) char  storage_[STORAGE_SIZE];
    void*                    functionTable_;
};

//------------------------------------------------------------------------------
// Thread
//------------------------------------------------------------------------------
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

constexpr int MAX_JOB_DEPENDENTS = 7;

//------------------------------------------------------------------------------
enum class JobState : int8
{
    INIT,
    WAITING,
    RUNNING,
    DONE,
};

enum JobFlags : int8
{
    JOB_NONE,
};

//------------------------------------------------------------------------------
struct alignas(64) Job
{
    JobFunction<56, 16> function_;
    Job*                dependents_[MAX_JOB_DEPENDENTS];
    int16               dependencies_;
    JobState            state_;
    JobFlags            flags_;
};

static_assert(sizeof(Job) == 128);

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
    void AddDependency(Job* a, Job* b);

    // Move to a job ring allocator, it's not bound to the system since we don't intend to use indices to reference jobs
    Job* AllocateJob(int workerId);

    Array<Thread>           threadPool_;
    Array<WorkerContext>    workerContext_;
    Array<Array<Job>>       jobPools_;
    Array<int>              jobPoolIdx_;
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
        ThreadDestroy(threadPool_[i]);
    }
}

//------------------------------------------------------------------------------
Job* JobSystem::AllocateJob(int workerId)
{
    jobPoolIdx_ = ((jobPoolIdx_ + 1) & (jobPools_[workerId].Count() - 1));
    return jobPools_[workerId][jobPoolIdx_];
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
    while (AtomicLoad(job->
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
