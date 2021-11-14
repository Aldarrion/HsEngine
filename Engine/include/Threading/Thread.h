#pragma once

#include "Config.h"

#include "String/String.h"
#include "String/StringUtil.h"
#if HS_WINDOWS
    #include "Platform/hs_Windows.h"
    #include <processthreadsapi.h>
#endif

namespace hs
{

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

//------------------------------------------------------------------------------
#if HS_WINDOWS
    DWORD ThreadFunctionWin32Wrapper(void* data)
    {
        auto wrapperData = static_cast<ThreadFunctionWrapperData*>(data);
        wrapperData->function_(wrapperData->data_);
        return 0;
    }
#endif

//------------------------------------------------------------------------------
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
            SetThreadDescription(thread.hThread_, wname.buffer_);
            // Do not check result since it is possible the thread already finished execution
        }

    #else
        HS_NOT_IMPLEMENTED;
        thread = {};
    #endif

    return thread;
}

//------------------------------------------------------------------------------
void ThreadDestroy(Thread* thread)
{
    #if HS_WINDOWS
        bool closed = CloseHandle(thread->hThread_);
        HS_ASSERT(closed);
    #else
        HS_NOT_IMPLEMENTED;
    #endif
}
}
