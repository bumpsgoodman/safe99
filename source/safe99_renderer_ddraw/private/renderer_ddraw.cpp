//***************************************************************************
// 
// 파일: renderer_ddraw.cpp
// 
// 설명: DirectDraw7 렌더러
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/31
// 
//***************************************************************************

#include "renderer_ddraw.h"
#include "safe99_common/assert.h"
#include "safe99_common/safe_delete.h"
#include "safe99_math/math.h"

#define GET_ALPHA(argb) ((argb) & 0xff000000)

enum
{
    REGION_LEFT = 0x0001,
    REGION_RIGHT = 0x0010,
    REGION_TOP = 0x1000,
    REGION_BOTTOM = 0x0100
};

static bool create_back_buffer(renderer_ddraw_t* p_ddraw, const size_t width, const size_t height);

static int get_region(const int sx, const int sy, const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y);

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

SAFE99_API size_t renderer_ddraw_get_width(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    return p_ddraw->window_width;
}

SAFE99_API size_t renderer_ddraw_get_height(renderer_ddraw_t* p_ddraw)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    return p_ddraw->window_height;
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

    const size_t window_width = (size_t)(p_ddraw->window_rect.right - p_ddraw->window_rect.left);
    const size_t window_height = (size_t)(p_ddraw->window_rect.bottom - p_ddraw->window_rect.top);
    if (window_width == 0 || window_height == 0)
    {
        return;
    }

    p_ddraw->window_width = window_width;
    p_ddraw->window_height = window_height;

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

void renderer_ddraw_draw_pixel(renderer_ddraw_t* p_ddraw, const int dx, const int dy, const uint32_t argb)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (dx < 0 || (size_t)dx >= p_ddraw->window_width
        || dy < 0 || (size_t)dy >= p_ddraw->window_height)
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

void renderer_ddraw_draw_rectangle(renderer_ddraw_t* p_ddraw, const int dx, const int dy, const size_t width, const size_t height, const uint32_t argb)
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

void renderer_ddraw_draw_line(renderer_ddraw_t* p_ddraw, const int sx, const int sy, const int dx, const int dy, const uint32_t argb)
{
    if (GET_ALPHA(argb) == 0)
    {
        return;
    }

    const int width = dx - sx;
    const int height = dy - sy;

    const int increament_x = (width >= 0) ? 1 : -1;
    const int increament_y = (height > 0) ? 1 : -1;

    const int w = increament_x * width;
    const int h = increament_y * height;

    // 완만한지 검사
    const bool b_gradual = (ABS(width) >= ABS(height)) ? true : false;

    int start_x = sx;
    int start_y = sy;

    int end_x = dx;
    int end_y = dy;
    if (!renderer_ddraw_clip_line(&start_x, &start_y, &end_x, &end_y, 0, 0, (int)renderer_ddraw_get_width(p_ddraw) - 1, (int)renderer_ddraw_get_height(p_ddraw) - 1))
    {
        return;
    }

    int x = start_x;
    int y = start_y;

    if (b_gradual)
    {
        int f = 2 * h - w;

        const int df1 = 2 * h;
        const int df2 = 2 * (h - w);

        while (x != end_x)
        {
            renderer_ddraw_draw_pixel(p_ddraw, x, y, argb);

            if (f < 0)
            {
                f += df1;
            }
            else
            {
                f += df2;
                y += increament_y;
            }

            x += increament_x;
        }
    }
    else
    {
        int f = 2 * w - h;

        const int df1 = 2 * w;
        const int df2 = 2 * (w - h);

        while (y != end_y)
        {
            renderer_ddraw_draw_pixel(p_ddraw, x, y, argb);

            if (f < 0)
            {
                f += df1;
            }
            else
            {
                f += df2;
                x += increament_x;
            }

            y += increament_y;
        }
    }

    renderer_ddraw_draw_pixel(p_ddraw, x, y, argb);
}

void renderer_ddraw_draw_bitmap(renderer_ddraw_t* p_ddraw, const int dx, const int dy, const int sx, const int sy, const size_t sw, const size_t sh, const size_t width, const size_t height, const char* p_bitmap)
{
    ASSERT(p_ddraw != NULL, "p_ddraw == NULL");
    ASSERT(p_ddraw->p_locked_back_buffer != NULL, "locked back buffer == NULL");
    ASSERT(p_bitmap != NULL, "bitmap == NULL");

    const size_t start_x = MAX(dx, 0);
    const size_t start_y = MAX(dy, 0);
    const size_t end_x = MIN(dx + sw, p_ddraw->window_width);
    const size_t end_y = MIN(dy + sh, p_ddraw->window_height);

    const size_t dst_width = end_x - start_x;
    const size_t dst_height = end_y - start_y;

    const char* src = p_bitmap + (sy * width + sx) * 4;
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

bool renderer_ddraw_clip_line(int* p_out_sx, int* p_out_sy, int* p_out_dx, int* p_out_dy, const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y)
{
    ASSERT(p_out_sx != NULL, "p_out_sx == NULL");
    ASSERT(p_out_sy != NULL, "p_out_sy == NULL");
    ASSERT(p_out_dx != NULL, "p_out_dx == NULL");
    ASSERT(p_out_dy != NULL, "p_out_dy == NULL");

    while (true)
    {
        const int start_region = get_region(*p_out_sx, *p_out_sy, left_top_x, left_top_y, right_bottom_x, right_bottom_y);
        const int end_region = get_region(*p_out_dx, *p_out_dy, left_top_x, left_top_y, right_bottom_x, right_bottom_y);

        // 두 영역 모두 0이면 두 영역은 모두 영역 내에 있음
        if ((start_region | end_region) == 0)
        {
            return true;
        }

        // 0보다 크면 두 영역은 모두 영역 밖에 있음
        if ((start_region & end_region) > 0)
        {
            return false;
        }

        // 클리핑
        const int width = *p_out_dx - *p_out_sx;
        const int height = *p_out_dy - *p_out_sy;

        const float slope = (width == 0) ? (float)height : (float)height / width;

        float x;
        float y;

        const int clipped_region = (start_region != 0) ? start_region : end_region;
        if (clipped_region & REGION_LEFT)
        {
            // 좌측 영역인 경우 수직 경계선과 교점 계산
            x = (float)left_top_x;
            y = *p_out_sy + slope * (x - *p_out_sx);
        }
        else if (clipped_region & REGION_RIGHT)
        {
            // 우측 영역인 경우 수직 경계선과 교점 계산
            x = (float)right_bottom_x;
            y = *p_out_sy + slope * (x - *p_out_sx);
        }
        else if (clipped_region & REGION_TOP)
        {
            // 위 영역인 경우 평 경계선과 교점 계산
            y = (float)left_top_y;
            x = *p_out_sx + (y - *p_out_sy) / slope;
        }
        else
        {
            // 아래 영역인 경우 평 경계선과 교점 계산
            y = (float)right_bottom_y;
            x = *p_out_sx + (y - *p_out_sy) / slope;
        }

        if (clipped_region == start_region)
        {
            *p_out_sx = ROUND(x);
            *p_out_sy = ROUND(y);
        }
        else
        {
            *p_out_dx = ROUND(x);
            *p_out_dy = ROUND(y);
        }
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

static int get_region(const int sx, const int sy, const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y)
{
    int result = 0;
    if (sx < left_top_x)
    {
        result |= REGION_LEFT; // 0b0001
    }
    else if (sx > right_bottom_x)
    {
        result |= REGION_RIGHT; // 0b0010
    }

    if (sy < left_top_y)
    {
        result |= REGION_TOP; // 0b1000
    }
    else if (sy > right_bottom_y)
    {
        result |= REGION_BOTTOM; // 0b0100
    }

    return result;
}