#include "renderer_ddraw.h"
#include "safe99_common/assert.h"
#include "safe99_common/safe_delete.h"
#include "safe99_math/math_util.h"

#define GET_ALPHA(argb) ((argb) & 0xff000000)

static bool create_back_buffer(renderer_ddraw_t* p_ddraw, const size_t width, const size_t height);

bool renderer_ddraw_init(renderer_ddraw_t* p_ddraw, HWND hwnd)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");

    p_ddraw->hwnd = hwnd;
    
    HRESULT hr;
    DDSURFACEDESC2 ddsd = { 0, };

    renderer_ddraw_update_window_pos(p_ddraw);

    const size_t WINDOW_WIDTH = (size_t)(p_ddraw->window_rect.right - p_ddraw->window_rect.left);
    const size_t WINDOW_HEIGHT = (size_t)(p_ddraw->window_rect.bottom - p_ddraw->window_rect.top);

    p_ddraw->window_width = WINDOW_WIDTH;
    p_ddraw->window_height = WINDOW_HEIGHT;

    hr = DirectDrawCreate(NULL, &p_ddraw->p_ddraw1, NULL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to Create DDrawObject");
        goto failed_init;
    }

    hr = p_ddraw->p_ddraw1->QueryInterface(IID_IDirectDraw7, (LPVOID*)&p_ddraw->p_ddraw7);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to Create DDraw7 Object");
        goto failed_init;
    }

    hr = p_ddraw->p_ddraw7->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to SetCooperativeLeve");
        goto failed_init;
    }

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = p_ddraw->p_ddraw7->CreateSurface(&ddsd, &p_ddraw->p_primary, NULL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to CreateSurface");
        goto failed_init;
    }

    if (!create_back_buffer(p_ddraw, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        ASSERT(false, "Failed to CreateBackBuffer");
        goto failed_init;
    }

    hr = p_ddraw->p_ddraw7->CreateClipper(0, &p_ddraw->p_clipper, NULL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to CreateClipper");
        goto failed_init;
    }
    p_ddraw->p_clipper->SetHWnd(0, hwnd);
    p_ddraw->p_primary->SetClipper(p_ddraw->p_clipper);

    return true;

failed_init:
    renderer_ddraw_release(p_ddraw);
    return false;
}

void renderer_ddraw_release(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");

    SAFE_RELEASE(p_ddraw->p_clipper);
    SAFE_RELEASE(p_ddraw->p_back);
    SAFE_RELEASE(p_ddraw->p_primary);
    SAFE_RELEASE(p_ddraw->p_ddraw7);
    SAFE_RELEASE(p_ddraw->p_ddraw1);
}

void renderer_ddraw_update_window_pos(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");

    GetClientRect(p_ddraw->hwnd, &p_ddraw->window_rect);
    ClientToScreen(p_ddraw->hwnd, (POINT*)&p_ddraw->window_rect.left);
    ClientToScreen(p_ddraw->hwnd, (POINT*)&p_ddraw->window_rect.right);
}

void renderer_ddraw_update_window_size(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");

    renderer_ddraw_update_window_pos(p_ddraw);
    p_ddraw->window_width = (size_t)(p_ddraw->window_rect.right - p_ddraw->window_rect.left);
    p_ddraw->window_height = (size_t)(p_ddraw->window_rect.bottom - p_ddraw->window_rect.top);

    create_back_buffer(p_ddraw, p_ddraw->window_width, p_ddraw->window_height);
}

bool renderer_ddraw_begin_draw(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_back != NULL, "back buffer == NULL");

    HRESULT hr = 0;
    DDSURFACEDESC2 ddsd = { 0, };

    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = p_ddraw->p_back->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
    if (FAILED(hr))
    {
        return false;
    }

    ASSERT(ddsd.dwWidth == p_ddraw->window_width, "Mismatch width");
    ASSERT(ddsd.dwHeight == p_ddraw->window_height, "Mismatch height");

    p_ddraw->p_locked_back_buffer = (char*)ddsd.lpSurface;
    p_ddraw->locked_back_buffer_pitch = ddsd.lPitch;

    return true;
}

void renderer_ddraw_end_draw(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_back != NULL, "back buffer == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    p_ddraw->p_back->Unlock(NULL);

    p_ddraw->p_locked_back_buffer = NULL;
    p_ddraw->locked_back_buffer_pitch = 0;
}

void renderer_ddraw_on_draw(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    p_ddraw->p_primary->Blt(&p_ddraw->window_rect, p_ddraw->p_back, NULL, DDBLT_WAIT, NULL);
}

void renderer_ddraw_clear(renderer_ddraw_t* p_ddraw, const uint32_t argb)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (GET_ALPHA(argb) == 0)
    {
        return;
    }

    char* dst = p_ddraw->p_locked_back_buffer;
    for (size_t y = 0; y < p_ddraw->window_height; ++y)
    {
        for (size_t x = 0; x < p_ddraw->window_width; ++x)
        {
            uint32_t* dest = (uint32_t*)(dst + y * p_ddraw->locked_back_buffer_pitch + x * 4);
            *dest = argb;
        }
    }
}

void renderer_ddraw_draw_pixel(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const uint32_t argb)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (dx < 0 || dx >= p_ddraw->window_width
        || dy < 0 || dy >= p_ddraw->window_height)
    {
        return;
    }

    if (GET_ALPHA(argb) == 0)
    {
        return;
    }

    char* dst = p_ddraw->p_locked_back_buffer + dy * p_ddraw->locked_back_buffer_pitch + dx * 4;
    *(uint32_t*)dst = argb;
}

void renderer_ddraw_draw_rectangle(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const size_t width, const size_t height, const uint32_t argb)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (GET_ALPHA(argb) == 0)
    {
        return;
    }

    const size_t start_x = MAX(dx, 0);
    const size_t start_y = MAX(dy, 0);
    const size_t end_x = MIN(dx + width, p_ddraw->window_width);
    const size_t end_y = MIN(dy + height, p_ddraw->window_height);

    const size_t dst_width = end_x - start_x;
    const size_t dst_height = end_y - start_y;

    char* dst = p_ddraw->p_locked_back_buffer + start_y * p_ddraw->locked_back_buffer_pitch + start_x * 4;
    for (size_t y = start_y; y < end_y; ++y)
    {
        for (size_t x = start_x; x < end_x; ++x)
        {
            *(uint32_t*)dst = argb;
            dst += 4;
        }

        dst -= dst_width * 4;
        dst += p_ddraw->locked_back_buffer_pitch;
    }
}

void renderer_ddraw_draw_horizontal_line(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const int32_t length, const uint32_t argb)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (GET_ALPHA(argb) == 0)
    {
        return;
    }

    size_t start_x;
    size_t end_x;

    if (length > 0)
    {
        start_x = MAX(dx, 0);
        end_x = MIN(dx + length, p_ddraw->window_width);
    }
    else
    {
        start_x = MIN(dx + length, p_ddraw->window_width);
        end_x = MAX(dx, 0);
    }
    

    char* dst = p_ddraw->p_locked_back_buffer + dy * p_ddraw->locked_back_buffer_pitch + start_x * 4;
    for (size_t x = start_x; x < end_x; ++x)
    {
        *(uint32_t*)dst = argb;
        dst += 4;
    }
}

void renderer_ddraw_draw_vertical_line(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const int32_t length, const uint32_t argb)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (GET_ALPHA(argb) == 0)
    {
        return;
    }

    size_t start_y;
    size_t end_y;

    if (length > 0)
    {
        start_y = MAX(dy, 0);
        end_y = MIN(dy + length, p_ddraw->window_height);
    } else
    {
        start_y = MIN(dy + length, p_ddraw->window_height);
        end_y = MAX(dy, 0);
    }

    char* dst = p_ddraw->p_locked_back_buffer + start_y * p_ddraw->locked_back_buffer_pitch + dx * 4;
    for (size_t y = start_y; y < end_y; ++y)
    {
        *(uint32_t*)dst = argb;
        dst += p_ddraw->locked_back_buffer_pitch;
    }
}

void renderer_ddraw_draw_bitmap(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const int32_t sx, const int32_t sy, const size_t width, const size_t height, const char* p_bitmap)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");
    ASSERT(p_bitmap != NULL, "bitmap == NULL");

    const size_t start_x = MAX(dx, 0);
    const size_t start_y = MAX(dy, 0);
    const size_t end_x = MIN(dx + width, p_ddraw->window_width);
    const size_t end_y = MIN(dy + height, p_ddraw->window_height);

    const size_t dst_width = end_x - start_x;
    const size_t dst_height = end_y - start_y;

    const char* src = p_bitmap + sy * width + sx * 4;
    char* dst = p_ddraw->p_locked_back_buffer + start_y * p_ddraw->locked_back_buffer_pitch + start_x * 4;
    for (size_t y = 0; y < dst_height; ++y)
    {
        for (size_t x = 0; x < dst_width; ++x)
        {
            if (GET_ALPHA(*(uint32_t*)src) != 0)
            {
                *(uint32_t*)dst = *(uint32_t*)src;
            }

            src += 4;
            dst += 4;
        }

        src -= dst_width * 4;
        src += width * 4;
        dst -= dst_width * 4;
        dst += p_ddraw->locked_back_buffer_pitch;
    }
}

static bool create_back_buffer(renderer_ddraw_t* p_ddraw, const size_t width, const size_t height)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_ddraw7 != NULL, "ddraw7 object == NULL");

    HRESULT hr;
    DDSURFACEDESC2 ddsd = { 0, };

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    ddsd.dwHeight = (DWORD)height;
    ddsd.dwWidth = (DWORD)width;

    hr = p_ddraw->p_ddraw7->CreateSurface(&ddsd, &p_ddraw->p_back, NULL);
    if (FAILED(hr))
    {
        return false;
    }

    p_ddraw->window_width = width;
    p_ddraw->window_height = height;

    return true;
}