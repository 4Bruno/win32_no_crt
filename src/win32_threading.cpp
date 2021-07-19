#include <windows.h>
#include "threading.h"

HANDLE
Win32CreateSemaphore(LONG MaxSemaphoreCount)
{
    SECURITY_ATTRIBUTES * NullSecurityAttrib = 0;
    const char * NullSempahoreName = 0;
    DWORD ReservedFlags = 0;

    LONG InitialCount = 0;

    HANDLE Hnd = CreateSemaphoreEx(
                NullSecurityAttrib,
                InitialCount,MaxSemaphoreCount,
                NullSempahoreName,
                ReservedFlags,
                SEMAPHORE_ALL_ACCESS
            );

    Assert(Hnd);

    return Hnd;
}

bool32
ThreadQueueDoNextTask(thread_work_queue * Queue)
{
    bool32 TimeToSleep = false;

    uint32 NextRead = Queue->NextRead;
    uint32 NextReadAfter = (NextRead + 1) % ArrayCount(Queue->Tasks);

    // First initialization all 0
    if (NextRead != Queue->NextWrite)
    {
        // returns variable destination initial value
        uint32 Index = InterlockedCompareExchange(
                        (LONG volatile *)&Queue->NextRead,  /* on variable */
                        NextReadAfter,                      /* write this value */
                        NextRead                            /* if variable equals */
                    );
        if (Index == NextRead)
        {
            thread_work_queue_task Entry = Queue->Tasks[Index];
            Entry.Task(Queue,Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->TasksCompleted);
        }
    }
    else
    {
        TimeToSleep = true;
    }

    return TimeToSleep;
}

void
QueueCompleteTasks(thread_work_queue * Queue)
{
    while (Queue->TasksCompleted != Queue->TasksPending)
    {
        ThreadQueueDoNextTask(Queue);
    }

    Queue->TasksPending = 0;
    Queue->TasksCompleted = 0;
}

DWORD WINAPI 
ThreadProc(LPVOID lpParameter)
{
    
    DWORD Result = 0;
    thread_work_queue * Queue = (thread_work_queue *)lpParameter;

    for (;;)
    {
        // if nothing to do will signal to sleep
        if (ThreadQueueDoNextTask(Queue))
        {
            WaitForSingleObjectEx(Queue->Semaphore,INFINITE,FALSE);
        }
    }
}

void
Win32CreateThread(SIZE_T StackSize, thread_work_queue * Queue)
{
    SECURITY_ATTRIBUTES * DefaultThreadAttrib = 0;
    DWORD CreationFlags = THREAD_RUN_INMMEDIATE;
    void * ThreadParam = (void *)Queue;
    DWORD ThreadID = 0;

    HANDLE Thread = 
        CreateThread(DefaultThreadAttrib, StackSize,
                ThreadProc, ThreadParam,
                THREAD_RUN_INMMEDIATE, &ThreadID);

    // The thread object remains in the system until:
    //    1) the thread has terminated 
    //    2) all handles to it have been closed through a call to CloseHandle.
    CloseHandle(Thread);
}

void
CreateThreadQueue(thread_work_queue * Queue, uint32 CountThreads)
{
    HANDLE Semaphore = Win32CreateSemaphore(CountThreads);
    Queue->Semaphore = Semaphore;

    Queue->TasksCompleted = 0;
    Queue->TasksPending = 0;

    Queue->NextWrite = 0;
    Queue->NextRead = 0;

    for (uint32 ThreadCount = 0;
            ThreadCount < CountThreads;
            ++ThreadCount)
    {
        Win32CreateThread(Megabytes(1), Queue);
    }
}

void
AddWorkToQueue(thread_work_queue * Queue, thread_queue_callback * Callback,void * Data)
{
    uint32 NextWriteAfter = (Queue->NextWrite + 1) % ArrayCount(Queue->Tasks);
    Assert(NextWriteAfter != Queue->NextRead);

    thread_work_queue_task * Entry = Queue->Tasks + Queue->NextWrite;
    Entry->Task = Callback;
    Entry->Data = Data;
    ++Queue->TasksPending;

    //_WriteBarrier(); // deprecated (vs 2019)?
    MemoryBarrier();
    Queue->NextWrite = NextWriteAfter;
    ReleaseSemaphore(Queue->Semaphore, 1, 0);
}

extern "C" BOOL WINAPI DllMain (
    HINSTANCE const instance,  // handle to DLL module
    DWORD     const reason,    // reason for calling function
    LPVOID    const reserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#if BUILD_DLL
BOOL WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpReserved)
{
	return DllMain(hinstDLL,fdwReason,lpReserved);
}
#endif
