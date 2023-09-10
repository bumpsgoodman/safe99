#include <Windows.h>
#include <memory.h>

#include "safe99_renderer_ddraw/renderer_ddraw.h"
#include "safe99_renderer_ddraw_test/image_loader.h"

HINSTANCE g_hinstance;
HWND g_hwnd;

renderer_ddraw_t* gp_ddraw;

HRESULT init_window(HINSTANCE hInstance, const size_t width, const size_t height, const int nCmdShow);
LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (FAILED(init_window(hInstance, 800, 600, nCmdShow)))
        return 0;

    gp_ddraw = (renderer_ddraw_t*)malloc(sizeof(renderer_ddraw_t));
    renderer_ddraw_init(gp_ddraw, g_hwnd);

    image_t image;
    load_a8r8g8b8_dds("E:\\safe99\\project\\safe99_renderer_ddraw_test\\safe99_renderer_ddraw_test\\32x32_creatures.dds", &image);

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            renderer_ddraw_begin_draw(gp_ddraw);
            {
                renderer_ddraw_clear(gp_ddraw, 0xff654321);
                renderer_ddraw_draw_bitmap(gp_ddraw, -16, -16, 0, 0, image.width, image.height, image.pa_bitmap);
                renderer_ddraw_draw_rectangle(gp_ddraw, 100, 300, 50, 50, 0xffff0000);
                renderer_ddraw_draw_horizontal_line(gp_ddraw, 300, 200, 200, 0xff00ff00);
                renderer_ddraw_draw_vertical_line(gp_ddraw, 300, 200, -150, 0xff0000ff);
            }
            renderer_ddraw_end_draw(gp_ddraw);

            renderer_ddraw_on_draw(gp_ddraw);
        }
    }

    free(gp_ddraw);

    return (int)msg.wParam;
}

HRESULT init_window(HINSTANCE hInstance, const size_t width, const size_t height, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wnd_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = NULL;
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    g_hinstance = hInstance;
    RECT rc = { 0, 0, (LONG)width, (LONG)height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hwnd = CreateWindow(L"TutorialWindowClass", L"safe99 renderer ddraw test",
                          WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                          CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                          NULL);
    if (!g_hwnd)
        return E_FAIL;

    ShowWindow(g_hwnd, nCmdShow);

    return S_OK;
}

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_MOVE:
        if (gp_ddraw != NULL)
        {
            renderer_ddraw_update_window_pos(gp_ddraw);
        }
        break;
    case WM_SIZE:
        if (gp_ddraw != NULL)
        {
            renderer_ddraw_update_window_size(gp_ddraw);
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}