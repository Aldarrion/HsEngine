#pragma once

#include "Config.h"
#include "Containers/Span.h"

namespace hs
{

constexpr int MAX_JOB_DEPENDENTS = 7;

//------------------------------------------------------------------------------
enum JobState : int8
{
    JOB_STATE_INIT,
    JOB_STATE_RUNNING,
    JOB_STATE_DONE,
};

enum JobFlags : int8
{
    JOB_NONE,
};

//------------------------------------------------------------------------------
struct Job;
using JobFunction = void (*)(int workerId, Job* job);

//------------------------------------------------------------------------------
struct alignas(64) Job
{
    JobFunction     function_;
    Job*            dependents_[MAX_JOB_DEPENDENTS];
    int16           dependencies_;
    JobState        state_;
    JobFlags        flags_;
};
static_assert(sizeof(Job) == 128);

//------------------------------------------------------------------------------
class JobSystem;
extern JobSystem* g_JobSystem;

struct JobSystemCreateParams
{
    int threadCount_;
    int jobPoolSize_;
};

void JobSystemCreate(const JobSystemCreateParams& params);
void JobSystemDestroy(JobSystem* jobSystem);

void JobSystemExecute(JobSystem* jobSystem, Span<Job*> jobs);
void JobSystemWait(JobSystem* jobSystem, Span<Job*> jobs);

}
