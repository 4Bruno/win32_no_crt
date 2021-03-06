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

ON_WINDOW_RESIZE(OnWindowResize)
{
    OutputDebugStringA("Window resized\n");
}

ON_KEY_PRESSED(OnKeyPressed)
{
    if (Key == VK_ESCAPE)
    {
        WindowUserData->WindowCloseRequested = true;
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

    while (!Window.WindowCloseRequested)
    {
        HandleInput(Window.HWnd);
        Log("Win32 defined\n");
    }

    ExitProcess(0);
}
