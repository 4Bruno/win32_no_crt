#ifndef WIN32_WINDOW
#define WIN32_WINDOW

#include <windows.h>
#include "platform.h"

// windowsx.h
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

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

HWND
Win32CreateWindow(HINSTANCE hInstance, const char * AppName, window_modes WindowMode, int Width, int Height,win32_window_user_data * WindowUserData);


/* API INTERNAL */
LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


/*
 * WindowUserData if supplied is a pointer to memory location
 *                which must last until window is closed
 *
 */
HWND
Win32CreateWindow(HINSTANCE hInstance, 
                  const char * WindowName, 
                  window_modes WindowMode,
                  int Width, int Height, 
                  win32_window_user_data * WindowUserData)
{
    Assert(WindowUserData);

    WNDCLASS wc      = {0}; 
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = WindowName;

    if( !RegisterClass(&wc) )
       return 0;

    HWND WindowHandle = 
        CreateWindow(wc.lpszClassName, 
                    WindowName, 
                    WindowMode , 
                    0,0, Width ,Height,
                    0,0, hInstance,
                    NULL);

    SetWindowLongPtrW(WindowHandle,GWLP_USERDATA,(LONG_PTR)WindowUserData);

    return WindowHandle;
}

struct win32_window
{
    int Height, Width;
    RECT Rect;
};

win32_window
Win32GetWindowSize(HWND hWnd)
{
    win32_window Wnd = {};

    RECT Rect;
    BOOL Success = GetWindowRect(hWnd, &Rect);

    if (Success)
    {
        Wnd.Height =  Rect.bottom - Rect.top;
        Wnd.Width = Rect.right - Rect.left;

        Wnd.Rect = Rect;
    }

    return Wnd;

}

win32_window_user_data *
Win32GetWindowData(HWND hWnd)
{
    win32_window_user_data * WindowUserData = 
        (win32_window_user_data *)GetWindowLongPtr(hWnd,GWLP_USERDATA);

    // There is legitimate case where this is null ptr
    // on window creation, windows will call wndproc before
    // we had time to set user data
    return WindowUserData;
}

LRESULT CALLBACK 
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    // First time will be null ptr as we didn't have time to set user data
    win32_window_user_data * WindowUserData = Win32GetWindowData(hWnd);

    switch(message)
    {
        case WM_CLOSE: 
        case WM_QUIT:
        {
            if (WindowUserData)
            {
                WindowUserData->WindowCloseRequested = true;
                DestroyWindow(hWnd);
            }
        } break;
        case WM_SIZING:
        {
            if (WindowUserData)
            {
                win32_window Wnd = Win32GetWindowSize(hWnd);
                RECT * Rect = (RECT *)lParam;
                if ((Wnd.Height < WindowUserData->MinHeight))
                {
                    Rect->bottom = Rect->top + WindowUserData->MinHeight;
                }
                if ((Wnd.Width < WindowUserData->MinWidth))
                {
                    Rect->right = Rect->left + WindowUserData->MinWidth;
                }
            }

        } break;
        case WM_SIZE:
        {
            if (WindowUserData)
            {
                RECT Rect;
                GetWindowRect(hWnd, &Rect);
                int Height =  Rect.bottom - Rect.top;
                int Width = Rect.right - Rect.left;
                if (WindowUserData->OnWindowResizeCallback)
                {
                    WindowUserData->OnWindowResizeCallback(Width,Height);
                }
            }
        } break;
        case WM_MOVE:
        {
            // Allow resizing window
            //Win32ReleaseCursor();
        } break;
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        };
    };
    return true;
}

void
HandleInput(HWND WindowHandle)
{
    win32_window_user_data * WindowUserData = Win32GetWindowData(WindowHandle);

    MSG msg = {};

    for (;;)
    { 
        BOOL GotMessage = PeekMessage( &msg, WindowHandle, 0, 0, PM_REMOVE );
        if (!GotMessage) break;
        // https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
        switch (msg.message)
        {
            case WM_CLOSE:
            case WM_QUIT:
            {
                if (WindowUserData)
                {
                    WindowUserData->WindowCloseRequested = true;
                }
            } break;

            case WM_SYSKEYDOWN: 
            case WM_SYSKEYUP: 
            case WM_KEYDOWN: 
            case WM_KEYUP:
            {
                if (!WindowUserData) break;

                uint32 VKCode = (uint32)msg.wParam;

                bool32 IsAltPressed = (msg.lParam & (1 << 29));
                // https://docs.microsoft.com/en-us/windows/win32/inputdev/using-keyboard-input
                bool32 IsShiftPressed = (GetKeyState(VK_SHIFT) & (1 << 15));
                bool32 IsPressed = (msg.lParam & (1UL << 31)) == 0;
                bool32 WasPressed = (msg.lParam & (1 << 30)) != 0;

                if (IsPressed != WasPressed)
                {
                    if (WindowUserData->OnKeyPressed)
                    {
                        WindowUserData->OnKeyPressed((int)VKCode,WindowUserData);
                    }
                }

            } break;
#if 1
            case WM_MOUSEMOVE:
            {
                RECT Rect;
                GetWindowRect(WindowHandle, &Rect);
                int32 yHeight =  Rect.bottom - Rect.top;
                int32 xWidth = Rect.right - Rect.left;
                int32 xPos = (int32)GET_X_LPARAM(msg.lParam) + 1;
                int32 yPos = (int32)GET_Y_LPARAM(msg.lParam) + 1; 
            } break;
#endif
            case WM_INPUT:
            {
                // raw mouse offsets for high resolution input
                UINT dwSize = sizeof(RAWINPUT);
                static BYTE lpb[sizeof(RAWINPUT)];

                GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

                RAWINPUT* raw = (RAWINPUT*)lpb;

                if (raw->header.dwType == RIM_TYPEMOUSE) 
                {
#if 0
                    Controller->RelMouseX = raw->data.mouse.lLastX;
                    // Flip Y
                    Controller->RelMouseY = -raw->data.mouse.lLastY;
#endif
                    
                } 
            } break;
            case WM_RBUTTONDOWN:
            {
            }break;
            case WM_RBUTTONUP:
            {
            }break;

            default :
            {
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            } break;
        }
    } // for lock
    
    if (WindowUserData->WindowCloseRequested)
    {
        DestroyWindow(WindowHandle);
    }
}

#endif

