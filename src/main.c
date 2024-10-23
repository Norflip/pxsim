#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <windows.h>

#include "surf.h"

#define SCRW 1024
#define SCRH 1024
#define SURFACE_SCALE 4

#define TICK_RATE 50
#define TICK_TIME (1.0 / TICK_RATE)

#define UMIN(a, b) ((a) < (b) ? (a) : (b))
#define UMAX(a, b) ((a) > (b) ? (a) : (b))
#define UCLAMP(x, a, b) UMIN(UMAX((x), (a)), (b))

HWND hwnd;
HDC hdc;
bool left_mouse_down, right_mouse_down;
int64_t tick_count;

static LRESULT wnd_proc(HWND, UINT, WPARAM, LPARAM);
static bool map_window_to_surface(int mapwidth, int mapheight, POINT* pOut);

// Helper function to count set bits in the processor mask.
DWORD CountSetBits(ULONG_PTR bitMask) {
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i) {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;
    }

    return bitSetCount;
}

int main() {
    printf("Hello!\n");

    WNDCLASS wc = {0};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = "pxsim-classname";
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hIcon = (HICON)LoadImage(NULL, "icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
    RegisterClass(&wc);

    DWORD dwstyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    RECT windowRect = {0, 0, (LONG)SCRW, (LONG)SCRH};
    AdjustWindowRect(&windowRect, dwstyle, false);
    int wWidth = windowRect.right - windowRect.left;
    int wHeight = windowRect.bottom - windowRect.top;
    hwnd = CreateWindow(wc.lpszClassName, "pxsim", dwstyle, 0, 0, wWidth, wHeight, 0, 0, wc.hInstance, 0);
    hdc = GetDC(hwnd);
    ShowWindow(hwnd, true);
    UpdateWindow(hwnd);

    double accumulator = 0.0;
    int64_t current_tick, last_tick;
    int64_t freq_tick;
    QueryPerformanceCounter((LARGE_INTEGER*)&current_tick);
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq_tick);
    last_tick = current_tick;


    // Get the logical processors
    // https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getlogicalprocessorinformation
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int num_cpus = sysinfo.dwNumberOfProcessors;


    int map_width = SCRW / SURFACE_SCALE;
    int map_height = SCRH / SURFACE_SCALE;
    surface_t surf = createSurface(map_width, map_height);

    printf("map size: %dx%d\n", map_width, map_height);
    printf("num threads: %d\n", num_cpus);

    MSG msg = {0};
    bool quit = false;
    while (!quit) {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                quit = true;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        surfaceUVDemo(&surf);

        // check if we have clicked
        POINT p;
        if (left_mouse_down || right_mouse_down) {
            if (map_window_to_surface(map_width, map_height, &p)) {
                if (left_mouse_down) {
                    draw_circle(&surf, p.x, p.y, 5, PIXEL_WHITE, false);
                } else {
                    draw_line(&surf, 0, 0, p.x, p.y, PIXEL_WHITE);
                }
            }
        }

        //draw_text(&surf, 50, 50, PIXEL_WHITE, "Hello, World!");


        QueryPerformanceCounter((LARGE_INTEGER*)&current_tick);
        double delta_time = (current_tick - last_tick) / (double)freq_tick;
        Sleep(1);

        bool did_tick = false;
        accumulator += delta_time;

        while (accumulator >= TICK_TIME) {
            // change the title to the fps based on the delta_time
            char title[256];
            sprintf(title, "pxsim - %f", 1.0 / delta_time);
            SetWindowText(hwnd, title);


            accumulator -= TICK_TIME;
            did_tick = true;
        }

        draw_text(&surf, 0, 0, PIXEL_WHITE, "sand\n5px");

        if (did_tick) {
            surface_blit(&surf, hdc);
            surface_clear(&surf);
            UpdateWindow(hwnd);
        }

        last_tick = current_tick;
    }

    CloseWindow(hwnd);
    return 0;
}

LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE)
                PostQuitMessage(0);
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            left_mouse_down = true;
            return 0;
        case WM_LBUTTONUP:
            left_mouse_down = false;
            return 0;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            right_mouse_down = true;
            return 0;
        case WM_RBUTTONUP:
            right_mouse_down = false;
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return 0;
}

static bool map_window_to_surface(int mapwidth, int mapheight, POINT* pOut) {
    POINT p;
    RECT rc;
    if (GetCursorPos(&p)) {
        GetClientRect(hwnd, &rc);
        ScreenToClient(hwnd, &p);
        if (PtInRect(&rc, p) == 0)
            return false;

        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        pOut->x = p.x * mapwidth / w;
        pOut->y = p.y * mapheight / h;
        pOut->x = UCLAMP(pOut->x, 0, mapwidth);
        pOut->y = UCLAMP(pOut->y, 0, mapheight);
        return true;
    }
    return false;
}