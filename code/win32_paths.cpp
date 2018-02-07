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
    i32 Width;
    i32 Height;
    i32 Pitch;
    i32 TileOffset;
    i32 BytesPerPixel;
};

struct game_tile
{
    b32 IsFree;
    i32 PreviousX;
    i32 PreviousY;
    i32 NextX;
    i32 NextY;
};

struct game_room
{
    i32 Width;
    i32 Height;
    game_tile *Tiles;
};

struct game_state
{
    b32 Running;
    i32 Seed;
    i32 RoomsCleared;
    i32 X;
    i32 Y;
    game_room Room;
    win32_offscreen_buffer Buffer;
};

global_variable game_state *GlobalGameState;

internal game_tile*
GetTile(game_state *GameState, int X, int Y)
{
    game_tile *Tile = (GameState->Room.Tiles + X + Y * GameState->Room.Width);
    return(Tile);
}

internal void
RedrawRoom(game_state *GameState)
{
    win32_offscreen_buffer *Buffer = &GameState->Buffer;
    
    u8 *PixelRow = (u8 *)Buffer->Memory;
    int TileY = 0;
    int SubY = 0;
    for(int Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)PixelRow;
        int TileX = 0;
        int SubX = 0;
        for(int X = 0;
            X < Buffer->Width;
            ++X)
        {
            game_tile *Tile = GetTile(GameState, TileX, TileY);
            if(SubX == 0 || SubY == 0)
            {
                *Pixel++ = 0x00000000;
            }
            else if(TileX == GameState->X && TileY == GameState->Y)
            {
                if((X & 1) == (Y & 1))
                {
                    *Pixel++ = 0x006F6F6F;
                }
                else
                {
                    *Pixel++ = 0x008F8F8F;
                }
            }
            else if(Tile->IsFree)
            {
                *Pixel++ = 0x00FFFFFF;
            }
            else
            {
                *Pixel++ = 0x000F0F0F;
            }
            
            if(++SubX == Buffer->TileOffset)
            {
                SubX = 0;
                ++TileX;
            }
        }
        if(++SubY == Buffer->TileOffset)
        {
            SubY = 0;
            ++TileY;
        }
        PixelRow += Buffer->Pitch;
    }
}

internal void
ResetRoom(game_state *GameState)
{
    int Height = 7;
    int Width = 9;
    GameState->X = Width / 2;
    GameState->Y = 0;
    
    game_room *Room = &GameState->Room;
    Room->Height = Height;
    Room->Width = Width;
    Room->Tiles = (game_tile *)(GameState + 1);
    
    game_tile *TileRow = Room->Tiles;
    for(int Y = 0;
        Y < Height;
        ++Y)
    {
        game_tile *Tile = TileRow;
        for(int X = 0;
            X < Width;
            ++X)
        {
            if(X == GameState->X && Y != GameState->Y)
            {
                Tile->IsFree = true;
                Tile->PreviousX = X;
                Tile->PreviousY = Y - 1;
                Tile->NextX = X;
                Tile->NextY = Y + 1;
            }
            else
            {
                Tile->IsFree = false;
            }
            ++Tile;
        }
        
        TileRow += Width;
    }
    
    win32_offscreen_buffer *Buffer = &GameState->Buffer;
    int BytesPerPixel = 4;
    int TileWidth = 5;
    int TileSpace = 1;
    int TileOffset = TileWidth + TileSpace;
    Buffer->Width = Width * TileOffset + TileSpace;
    Buffer->Height = Height * TileOffset + TileSpace;
    Buffer->BytesPerPixel = BytesPerPixel;
    Buffer->Pitch = (Buffer->Width * BytesPerPixel + 15) & ~15;
    Buffer->TileOffset = TileOffset;
    Buffer->Memory = (void *)(Room->Tiles + Height * Width);
    
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Pitch / BytesPerPixel;
    Buffer->Info.bmiHeader.biHeight = Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    RedrawRoom(GameState);
}

internal void
PlayerMoveUp(game_state *GameState)
{
    int X = GameState->X;
    int Y = GameState->Y;
    
    ++Y;
    
    GameState->X = X;
    GameState->Y = Y;
    
    RedrawRoom(GameState);
}

internal void
PlayerMoveDown(game_state *GameState)
{
    int X = GameState->X;
    int Y = GameState->Y;
    
    --Y;
    
    GameState->X = X;
    GameState->Y = Y;
    
    RedrawRoom(GameState);
}

internal void
PlayerMoveLeft(game_state *GameState)
{
    int X = GameState->X;
    int Y = GameState->Y;
    
    --X;
    
    GameState->X = X;
    GameState->Y = Y;
    
    RedrawRoom(GameState);
}

internal void
PlayerMoveRight(game_state *GameState)
{
    int X = GameState->X;
    int Y = GameState->Y;
    
    ++X;
    
    GameState->X = X;
    GameState->Y = Y;
    
    RedrawRoom(GameState);
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
            GlobalGameState->Running = false;
        } break;
        
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = (u32)WParam;// TODO(Zyonji): Should I switch to scan codes instead?
            b32 WasDown = ((LParam & (1 << 30)) != 0);
            b32 IsDown = ((LParam & (1 << 31)) == 0);
            if(IsDown && !WasDown)
            {
                if(VKCode == 'W')
                {
                    PlayerMoveUp(GlobalGameState);
                }
                else if(VKCode == 'A')
                {
                    PlayerMoveLeft(GlobalGameState);
                }
                else if(VKCode == 'S')
                {
                    PlayerMoveDown(GlobalGameState);
                }
                else if(VKCode == 'D')
                {
                    PlayerMoveRight(GlobalGameState);
                }
                else if(VKCode == VK_UP)
                {
                    PlayerMoveUp(GlobalGameState);
                }
                else if(VKCode == VK_LEFT)
                {
                    PlayerMoveLeft(GlobalGameState);
                }
                else if(VKCode == VK_DOWN)
                {
                    PlayerMoveDown(GlobalGameState);
                }
                else if(VKCode == VK_RIGHT)
                {
                    PlayerMoveRight(GlobalGameState);
                }
                
                RedrawWindow(Window, 0, 0, RDW_INVALIDATE);
            }
            
            b32 AltKeyWasDown = (LParam & (1 << 29));
            if((VKCode == VK_F4) && AltKeyWasDown)
            {
                GlobalGameState->Running = false;
            }
        } break;
        
        case WM_PAINT:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            u32 WindowWidth = ClientRect.right - ClientRect.left;
            u32 WindowHeight = ClientRect.bottom - ClientRect.top;
            
            win32_offscreen_buffer *Buffer = &GlobalGameState->Buffer;
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            StretchDIBits(DeviceContext,
                          0, 0, WindowWidth, WindowHeight,
                          0, 0, Buffer->Width, Buffer->Height,
                          Buffer->Memory,
                          &Buffer->Info,
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
    void *Memory = VirtualAlloc(0, 1000000, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    GlobalGameState = (game_state *)Memory;
    GlobalGameState->Running = true;
    GlobalGameState->Seed = 420023;
    GlobalGameState->RoomsCleared = 0;
    
    //game_state *test = GlobalGameState + 1;
    
    ResetRoom(GlobalGameState);
    
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
            MSG Message;
            
            while(GlobalGameState->Running && (MessageResult = GetMessageA(&Message, 0, 0, 0)))
            {
                if(MessageResult > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                else
                {
                    GlobalGameState->Running= false;
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