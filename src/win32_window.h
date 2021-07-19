#ifndef WIN32_WINDOW_H

#include <windows.h>
#include "platform.h"

/* API EXTERNAL */
enum window_modes
{
     window_mode_normal = WS_OVERLAPPEDWINDOW|WS_VISIBLE
};

struct win32_window_user_data;

#define ON_KEY_PRESSED(name) void name(int Key, win32_window_user_data * WindowUserData)
typedef ON_KEY_PRESSED(on_key_pressed);

#define ON_WINDOW_RESIZE(name) void name(int NewWidth, int NewHeight)
typedef ON_WINDOW_RESIZE(on_window_resize);

struct win32_window_user_data
{
    int MinWidth, MinHeight;
    int WindowCloseRequested;
    on_window_resize * OnWindowResizeCallback; 
    on_key_pressed * OnKeyPressed;
    HWND HWnd;
};

API HWND
Win32CreateWindow(HINSTANCE hInstance, const char * AppName, window_modes WindowMode, int Width, int Height,win32_window_user_data * WindowUserData);

API void
HandleInput(HWND WindowHandle);


#define WIN32_WINDOW_H
#endif

