#include <windows.h>
#include "win32_window.h"
#include "win32_log.hpp"
#include "threading.h"

ON_WINDOW_RESIZE(OnWindowResize);
ON_KEY_PRESSED(OnKeyPressed);

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

struct test_thread_data
{
    int test;
};

THREAD_QUEUE_CALLBACK(TestThreadWork)
{
    test_thread_data * TestData = (test_thread_data *)Data;
    Log("Current task data %i\n",TestData->test);
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
#define ABSURD_NUMBER 2000
            test_thread_data Data[ABSURD_NUMBER];
            for (uint32 TaskIndex = 0;
                    TaskIndex < ABSURD_NUMBER;
                    ++TaskIndex)
            {
                Data[TaskIndex].test = TaskIndex;
                AddWorkToQueue(&WorkQueue, TestThreadWork,(void *)(Data + TaskIndex));
            }
            //QueueCompleteTasks(&WorkQueue);
            //Log("tasks completed\n");
        }
    }

    ExitProcess(0);
}
