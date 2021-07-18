#include <stdint.h>
#include <windows.h>
#include "win32_window.h"

#define THREAD_RUN_INMMEDIATE 0

#if 0
#define _NO_CRT_STDIO_INLINE 
#include <stdio.h>
#define Log(format, ...) printf(format, __VA_ARGS__)
#else
#include "mbed_printf_implementation.cpp"
void
Log(const char * format, ...)
{
    char buffer[512];
    va_list list;
    va_start(list,format);
    if (mbed_minimal_formatted_string(buffer, 512, format, list, 0))
    {
        OutputDebugStringA(buffer);
    }
}
#endif

  
ON_WINDOW_RESIZE(OnWindowResize);
ON_KEY_PRESSED(OnKeyPressed);

struct thread_work_queue;
#define THREAD_QUEUE_CALLBACK(name) void name(thread_work_queue * Queue, void * Data)
typedef THREAD_QUEUE_CALLBACK(thread_queue_callback);

struct thread_work_queue_task
{
    thread_queue_callback * Task;
    void * Data;
};

struct thread_work_queue
{
    volatile uint32 TasksPending;
    volatile uint32 TasksCompleted;

    volatile uint32 NextWrite;
    volatile uint32 NextRead;

    thread_work_queue_task Tasks[256];

    HANDLE Semaphore;
};

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
CreateWindowApp(const char * WindowName, win32_window_user_data * WindowUserData)
{

    HINSTANCE Instance = GetModuleHandle(0);

    WindowUserData->MinWidth                   = 100; // INT   MinWidth;
    WindowUserData->MinHeight                  = 100; // INT   MinHeight;
    WindowUserData->WindowCloseRequested       = false; // INT   WindowCloseRequested;
    WindowUserData->OnWindowResizeCallback     = OnWindowResize; // Typedef * OnWindowResizeCallback;
    WindowUserData->OnKeyPressed               = OnKeyPressed;

    HWND HWnd = 
        Win32CreateWindow(Instance, WindowName, window_mode_normal, 500, 500, WindowUserData);

    WindowUserData->HWnd = HWnd;
}

ON_WINDOW_RESIZE(OnWindowResize)
{
    OutputDebugStringA("Window resized\n");
}

global_variable int K1Pressed = false;

ON_KEY_PRESSED(OnKeyPressed)
{
    if (Key == VK_ESCAPE)
    {
        WindowUserData->WindowCloseRequested = true;
    }
    if (Key == '1')
    {
        K1Pressed = true;
    }
}

struct test_thread_data
{
    int test;
};

THREAD_QUEUE_CALLBACK(TestThreadWork)
{
    test_thread_data * TestData = (test_thread_data *)Data;
    Log("Current task data %i\n",TestData->test);
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


extern "C"
int __stdcall
#if SHOW_CONSOLE
mainCRTStartup() /* /SUBSYTEM:console */
#else
WinMainCRTStartup() /* /SUBSYTEM:windows */
#endif
{
    win32_window_user_data Window;
    CreateWindowApp("Main",&Window);

    thread_work_queue WorkQueue;
    const uint32 ThreadCount = 12;
    CreateThreadQueue(&WorkQueue,ThreadCount);

    while (!Window.WindowCloseRequested)
    {
        K1Pressed = false;
        HandleInput(Window.HWnd);

        if (K1Pressed)
        {
            test_thread_data Data[15];
            for (uint32 TaskIndex = 0;
                        TaskIndex < 15;
                        ++TaskIndex)
            {
                Data[TaskIndex].test = TaskIndex;
                AddWorkToQueue(&WorkQueue, TestThreadWork,(void *)(Data + TaskIndex));
            }
            QueueCompleteTasks(&WorkQueue);
            Log("tasks completed\n");
        }
    }

    ExitProcess(0);
}
