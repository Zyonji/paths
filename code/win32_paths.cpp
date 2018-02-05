#include <windows.h>
#include <stdint.h>
#include <stddef.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t memory_index;

typedef float r32;
typedef double r64;

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

global_variable b32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;

internal void
Win32ResizeOffscreenBuffer(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    
    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;
    
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    Buffer->Pitch = (Width*BytesPerPixel + 15) & ~15;
    int BitmapMemorySize = (Buffer->Pitch*Buffer->Height);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

internal void
PlayerMoveUp()
{
    MessageBoxA(0, "No up action defined.", 0, MB_OK|MB_ICONERROR);
}

internal void
PlayerMoveDown()
{
    MessageBoxA(0, "No down action defined.", 0, MB_OK|MB_ICONERROR);
}

internal void
PlayerMoveLeft()
{
    MessageBoxA(0, "No left action defined.", 0, MB_OK|MB_ICONERROR);
}

internal void
PlayerMoveRight()
{
    MessageBoxA(0, "No right action defined.", 0, MB_OK|MB_ICONERROR);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_DESTROY:
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = (u32)WParam;
            b32 WasDown = ((LParam & (1 << 30)) != 0);
            b32 IsDown = ((LParam & (1 << 31)) == 0);
            if(IsDown && !WasDown)
            {
                if(VKCode == 'W')
                {
                    PlayerMoveUp();
                }
                else if(VKCode == 'A')
                {
                    PlayerMoveLeft();
                }
                else if(VKCode == 'S')
                {
                    PlayerMoveDown();
                }
                else if(VKCode == 'D')
                {
                    PlayerMoveRight();
                }
                else if(VKCode == VK_UP)
                {
                    PlayerMoveUp();
                }
                else if(VKCode == VK_LEFT)
                {
                    PlayerMoveLeft();
                }
                else if(VKCode == VK_DOWN)
                {
                    PlayerMoveDown();
                }
                else if(VKCode == VK_RIGHT)
                {
                    PlayerMoveRight();
                }
            }
            
            b32 AltKeyWasDown = (LParam & (1 << 29));
            if((VKCode == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }
        } break;
        
        case WM_PAINT:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            u32 WindowWidth = ClientRect.right - ClientRect.left;
            u32 WindowHeight = ClientRect.bottom - ClientRect.top;
            
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            StretchDIBits(DeviceContext,
                          0, 0, WindowWidth, WindowHeight,
                          0, 0, GlobalBackbuffer.Width, GlobalBackbuffer.Height,
                          GlobalBackbuffer.Memory,
                          &GlobalBackbuffer.Info,
                          DIB_RGB_COLORS, SRCCOPY);
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    Win32ResizeOffscreenBuffer(&GlobalBackbuffer, 1920, 1080);
    
    WNDCLASS WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    //    WindowClass.hIcon;
    WindowClass.lpszClassName = "PathsWindowClass";
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(0,
                            WindowClass.lpszClassName,
                            "Paths",
                            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            0,
                            0,
                            Instance,
                            0);
        if(Window)
        {
            UpdateWindow(Window);
            BOOL MessageResult;
            GlobalRunning = true;
            MSG Message;
            
            while(GlobalRunning && (MessageResult = GetMessageA(&Message, 0, 0, 0)))
            {
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                else
                {
                    GlobalRunning = false;
                }
            }
        }
        else
        {
            MessageBoxA(0, "A window could not be created.", 0, MB_OK|MB_ICONERROR);
        }
    }
    else
    {
        MessageBoxA(0, "A window class could not be registered.", 0, MB_OK|MB_ICONERROR);
    }
    
    return(0);
}