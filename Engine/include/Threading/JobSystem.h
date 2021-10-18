#pragma once

#include "Config.h"
#include "Containers/Span.h"

namespace hs
{

struct Job;
class JobSystem;
extern JobSystem* g_JobSystem;

struct JobSystemCreateParams
{
    int threadCount_;
};

void JobSystemCreate(const JobSystemCreateParams& params);
void JobSystemDestroy(JobSystem* jobSystem);
void JobSystemExecute(JobSystem* jobSystem, Span<Job*> jobs);
void JobSystemWait(JobSystem* jobSystem, Span<Job*> jobs);

}
