//***************************************************************************
// 
// 파일: soft_renderer_2d.cpp
// 
// 설명: DirectDraw7을 활용한 2D 소프트웨어 렌더러
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/31
// 
//***************************************************************************

#include <ddraw.h>
#include <stdlib.h>

#include "i_soft_renderer_2d.h"
#include "safe99_generic/util/timer.h"
#include "safe99_math/math.h"

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

#define GET_ALPHA(rgb) ((rgb) & 0xff000000)
#define GET_SEMANTIC(semantic) ((semantic) & UINT_MAX)

typedef struct vertex_buffer2
{
    i_vertex_buffer2_t base;
    size_t ref_count;

    vector2_t* pa_positions;
    color_t* pa_colors;
    vector2_t* pa_texcoords;

    size_t num_vertices;
} vertex_buffer2_t;

typedef struct index_buffer2
{
    i_index_buffer2_t base;
    size_t ref_count;

    uint_t* pa_indices;
    size_t num_indices;
} index_buffer2_t;

typedef struct mesh2
{
    i_mesh2_t base;
    size_t ref_count;

    i_vertex_buffer2_t* p_vertex_buffer;
    i_index_buffer2_t* p_index_buffer;
    i_texture2_t* p_texture;
} mesh2_t;

typedef struct soft_renderer_2d
{
    i_soft_renderer_2d_t base;
    size_t ref_count;

    HWND hwnd;
    RECT window_rect;
    size_t window_width;
    size_t window_height;

    IDirectDraw* p_ddraw;
    IDirectDraw7* p_ddraw7;
    IDirectDrawSurface7* p_primary;
    IDirectDrawSurface7* p_back;
    IDirectDrawClipper* p_clipper;

    char* p_locked_back_buffer;
    size_t locked_back_buffer_pitch;
} soft_renderer_2d_t;

enum
{
    REGION_LEFT = 0x0001,
    REGION_RIGHT = 0x0010,
    REGION_TOP = 0x1000,
    REGION_BOTTOM = 0x0100
};

extern "C" void __stdcall create_vertex_buffer2_private(i_vertex_buffer2_t** pp_out_vertex_buffer);
extern "C" void __stdcall create_index_buffer2_private(i_index_buffer2_t** pp_out_index_buffer);
extern "C" void __stdcall create_mesh2_private(i_mesh2_t * *pp_out_mesh);

static bool __stdcall create_back_buffer_private(soft_renderer_2d_t* p_renderer, const size_t width, const size_t height);

static vector2_t __stdcall to_screen_coord(const vector2_t cartesian_coord, const size_t screen_width, const size_t screen_height);
static vector2_t __stdcall to_cartesian_coord(const vector2_t screen_coord, const size_t screen_width, const size_t screen_height);
static int __stdcall get_region(const int sx, const int sy, const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y);

void __stdcall create_instance(void** pp_out_instance);

static size_t __stdcall add_ref(i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    return ++p_renderer->ref_count;
}

static size_t __stdcall release(i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    if (--p_renderer->ref_count == 0)
    {
        SAFE_RELEASE_MS(p_renderer->p_clipper);
        SAFE_RELEASE_MS(p_renderer->p_back);
        SAFE_RELEASE_MS(p_renderer->p_primary);
        SAFE_RELEASE_MS(p_renderer->p_ddraw7);
        SAFE_RELEASE_MS(p_renderer->p_ddraw);

        SAFE_FREE(p_this);

        return 0;
    }

    return p_renderer->ref_count;
}

static size_t __stdcall get_ref_count(const i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    return p_renderer->ref_count;
}

static void __stdcall update_window_pos(i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

    GetClientRect(p_renderer->hwnd, &p_renderer->window_rect);
    ClientToScreen(p_renderer->hwnd, (POINT*)&p_renderer->window_rect.left);
    ClientToScreen(p_renderer->hwnd, (POINT*)&p_renderer->window_rect.right);
}

static void __stdcall update_window_size(i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

    update_window_pos(p_this);

    const size_t window_width = (size_t)p_renderer->window_rect.right - p_renderer->window_rect.left;
    const size_t window_height = (size_t)p_renderer->window_rect.bottom - p_renderer->window_rect.top;
    if (window_width == 0 || window_height == 0)
    {
        return;
    }

    p_renderer->window_width = window_width;
    p_renderer->window_height = window_height;

    create_back_buffer_private(p_renderer, p_renderer->window_width, p_renderer->window_height);
}

static bool __stdcall initialize(i_soft_renderer_2d_t* p_this, HWND hwnd)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

    p_renderer->hwnd = hwnd;

    HRESULT hr;
    DDSURFACEDESC2 ddsd = { 0, };

    update_window_pos(p_this);

    const size_t WINDOW_WIDTH = (size_t)p_renderer->window_rect.right - p_renderer->window_rect.left;
    const size_t WINDOW_HEIGHT = (size_t)p_renderer->window_rect.bottom - p_renderer->window_rect.top;

    p_renderer->window_width = WINDOW_WIDTH;
    p_renderer->window_height = WINDOW_HEIGHT;

    hr = DirectDrawCreate(NULL, &p_renderer->p_ddraw, NULL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to Create DDrawObject");
        goto failed_init;
    }

    hr = p_renderer->p_ddraw->QueryInterface(IID_IDirectDraw7, (LPVOID*)&p_renderer->p_ddraw7);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to Create DDraw7 Object");
        goto failed_init;
    }

    hr = p_renderer->p_ddraw7->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to SetCooperativeLeve");
        goto failed_init;
    }

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = p_renderer->p_ddraw7->CreateSurface(&ddsd, &p_renderer->p_primary, NULL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to CreateSurface");
        goto failed_init;
    }

    if (!create_back_buffer_private(p_renderer, WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        ASSERT(false, "Failed to CreateBackBuffer");
        goto failed_init;
    }

    hr = p_renderer->p_ddraw7->CreateClipper(0, &p_renderer->p_clipper, NULL);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to CreateClipper");
        goto failed_init;
    }
    p_renderer->p_clipper->SetHWnd(0, hwnd);
    p_renderer->p_primary->SetClipper(p_renderer->p_clipper);

    return true;

failed_init:
    release(p_this);
    memset(p_renderer, 0, sizeof(soft_renderer_2d_t));
    return false;
}

static size_t __stdcall get_width(const i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    return p_renderer->window_width;
}

static size_t __stdcall get_height(const i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    return p_renderer->window_height;
}

static bool __stdcall begin_draw(i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    ASSERT(p_renderer->p_back != NULL, "back buffer == NULL");

    HRESULT hr = 0;
    DDSURFACEDESC2 ddsd = { 0, };

    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = p_renderer->p_back->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
    if (FAILED(hr))
    {
        return false;
    }

    ASSERT(ddsd.dwWidth == p_renderer->window_width, "Mismatch width");
    ASSERT(ddsd.dwHeight == p_renderer->window_height, "Mismatch height");

    p_renderer->p_locked_back_buffer = (char*)ddsd.lpSurface;
    p_renderer->locked_back_buffer_pitch = ddsd.lPitch;

    return true;
}

static void __stdcall end_draw(i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    ASSERT(p_renderer->p_back != NULL, "back buffer == NULL");
    ASSERT(p_renderer->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    p_renderer->p_back->Unlock(NULL);

    p_renderer->p_locked_back_buffer = NULL;
    p_renderer->locked_back_buffer_pitch = 0;
}

static void __stdcall on_draw(i_soft_renderer_2d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    p_renderer->p_primary->Blt(&p_renderer->window_rect, p_renderer->p_back, NULL, DDBLT_WAIT, NULL);
}

bool __stdcall begin_gdi(const i_soft_renderer_2d_t* p_this, HDC* p_out_hdc)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

    HRESULT hr;
    HDC hdc;

    hr = p_renderer->p_back->GetDC(&hdc);
    if (FAILED(hr))
    {
        ASSERT(false, "Failed to GetDC");
        return false;
    }

    *p_out_hdc = hdc;

    return true;
}

void __stdcall end_gdi(const i_soft_renderer_2d_t* p_this, const HDC hdc)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    p_renderer->p_back->ReleaseDC(hdc);
}

void __stdcall print_text(const i_soft_renderer_2d_t* p_this, const HDC hdc,
                          const wchar_t* text, const int dx, const int dy, const size_t length, const uint32_t rgb)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

    const uint32_t r = (rgb & 0x00ff0000) >> 16;
    const uint32_t g = (rgb & 0x0000ff00) >> 8;
    const uint32_t b = (rgb & 0x000000ff);

    SetTextColor(hdc, RGB(r, g, b));
    SetBkMode(hdc, TRANSPARENT);

    TextOut(hdc, dx, dy, text, (int)length);
}

static void __stdcall clear(const i_soft_renderer_2d_t* p_this, const uint32_t rgb)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    ASSERT(p_renderer->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (GET_ALPHA(rgb) == 0x00)
    {
        return;
    }

    char* dst = p_renderer->p_locked_back_buffer;
    for (size_t y = 0; y < p_renderer->window_height; ++y)
    {
        for (size_t x = 0; x < p_renderer->window_width; ++x)
        {
            uint32_t* dest = (uint32_t*)(dst + y * p_renderer->locked_back_buffer_pitch + x * 4);
            *dest = rgb;
        }
    }
}

static void __stdcall draw_pixel(const i_soft_renderer_2d_t* p_this, const int dx, const int dy, const uint32_t rgb)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    ASSERT(p_renderer->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if ((size_t)dx >= p_renderer->window_width
        || (size_t)dy >= p_renderer->window_height)
    {
        return;
    }

    if (GET_ALPHA(rgb) == 0x00)
    {
        return;
    }

    char* dst = p_renderer->p_locked_back_buffer + dy * p_renderer->locked_back_buffer_pitch + (size_t)dx * 4;
    *(uint32_t*)dst = rgb;
}

static void __stdcall draw_rectangle(const i_soft_renderer_2d_t* p_this, const int dx, const int dy, const size_t width, const size_t height, const uint32_t rgb)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    ASSERT(p_renderer->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    if (GET_ALPHA(rgb) == 0x00)
    {
        return;
    }

    const size_t start_x = MAX(dx, 0);
    const size_t start_y = MAX(dy, 0);
    const size_t end_x = MIN(dx + width, p_renderer->window_width);
    const size_t end_y = MIN(dy + height, p_renderer->window_height);

    const size_t dst_width = end_x - start_x;
    const size_t dst_height = end_y - start_y;

    char* dst = p_renderer->p_locked_back_buffer + start_y * p_renderer->locked_back_buffer_pitch + start_x * 4;
    for (size_t y = start_y; y < end_y; ++y)
    {
        for (size_t x = start_x; x < end_x; ++x)
        {
            *(uint32_t*)dst = rgb;
            dst += 4;
        }

        dst -= dst_width * 4;
        dst += p_renderer->locked_back_buffer_pitch;
    }
}

static bool __stdcall clip_line(int* p_out_sx, int* p_out_sy, int* p_out_dx, int* p_out_dy, const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y)
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
            *p_out_sx = ROUND_INT(x);
            *p_out_sy = ROUND_INT(y);
        }
        else
        {
            *p_out_dx = ROUND_INT(x);
            *p_out_dy = ROUND_INT(y);
        }
    }
}

static void __stdcall draw_line(const i_soft_renderer_2d_t* p_this, const int sx, const int sy, const int dx, const int dy, const uint32_t rgb)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    if (GET_ALPHA(rgb) == 0x00)
    {
        return;
    }

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

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
    if (!clip_line(&start_x, &start_y, &end_x, &end_y, 0, 0, (int)get_width(p_this) - 1, (int)get_height(p_this) - 1))
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
            draw_pixel(p_this, x, y, rgb);

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
            draw_pixel(p_this, x, y, rgb);

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

    draw_pixel(p_this, x, y, rgb);
}

static void __stdcall draw_bitmap(const i_soft_renderer_2d_t* p_this, const int dx, const int dy, const int sx, const int sy, const size_t sw, const size_t sh, const size_t width, const size_t height, const char* p_bitmap)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_bitmap != NULL, "bitmap == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    ASSERT(p_renderer->p_locked_back_buffer != NULL, "locked back buffer == NULL");

    const size_t start_x = MAX(dx, 0);
    const size_t start_y = MAX(dy, 0);
    const size_t end_x = MIN(dx + sw, p_renderer->window_width);
    const size_t end_y = MIN(dy + sh, p_renderer->window_height);

    const size_t dst_width = end_x - start_x;
    const size_t dst_height = end_y - start_y;

    const char* src = p_bitmap + (sy * width + sx) * 4;
    char* dst = p_renderer->p_locked_back_buffer + start_y * p_renderer->locked_back_buffer_pitch + start_x * 4;
    for (size_t y = 0; y < dst_height; ++y)
    {
        for (size_t x = 0; x < dst_width; ++x)
        {
            if (GET_ALPHA(*(uint32_t*)src) != 0x00)
            {
                *(uint32_t*)dst = *(uint32_t*)src;
            }

            src += 4;
            dst += 4;
        }

        src -= dst_width * 4;
        src += width * 4;
        dst -= dst_width * 4;
        dst += p_renderer->locked_back_buffer_pitch;
    }
}

static bool __stdcall create_vertex_buffer(const i_soft_renderer_2d_t* p_this,
                                           const semantic_t* p_semantics, const size_t* p_offsets, const size_t num_semantics,
                                           const void* p_vertices, const size_t num_vertices,
                                           i_vertex_buffer2_t** pp_out_vertex_buffer)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_semantics != NULL, "p_semantics == NULL");
    ASSERT(p_offsets != NULL, "p_offsets == NULL");
    ASSERT(num_semantics != 0, "num_semantics == 0");
    ASSERT(p_vertices != NULL, "p_vertices == NULL");
    ASSERT(num_vertices != 0, "num_vertices == 0");
    ASSERT(pp_out_vertex_buffer != NULL, "pp_out_vertex_buffer == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;
    const char* p_vertex = (char*)p_vertices;

    i_vertex_buffer2_t* p_vertex_buffer;
    create_vertex_buffer2_private(&p_vertex_buffer);

    vertex_buffer2_t* p_buffer = (vertex_buffer2_t*)p_vertex_buffer;
    p_buffer->pa_texcoords = NULL;
    p_buffer->pa_colors = NULL;
    p_buffer->pa_positions = NULL;

    // 버텍스 크기 구하기 및 할당
    size_t vertex_size = 0;
    for (size_t i = 0; i < num_semantics; ++i)
    {
        switch (p_semantics[i])
        {
        case SEMANTIC_POSITION:
            p_buffer->pa_positions = (vector2_t*)malloc(sizeof(vector2_t) * num_vertices);
            if (p_buffer->pa_positions == NULL)
            {
                ASSERT(false, "Failed to malloc positions");
                goto failed_malloc_positions;
            }

            vertex_size += sizeof(vector2_t);
            break;
        case SEMANTIC_COLOR:
            p_buffer->pa_colors = (color_t*)malloc(sizeof(color_t) * num_vertices);
            if (p_buffer->pa_colors == NULL)
            {
                ASSERT(false, "Failed to malloc colors");
                goto failed_malloc_colors;
            }

            vertex_size += sizeof(color_t);
            break;
        case SEMANTIC_TEXCOORD:
            p_buffer->pa_texcoords = (vector2_t*)malloc(sizeof(vector2_t) * num_vertices);
            if (p_buffer->pa_texcoords == NULL)
            {
                ASSERT(false, "Failed to malloc texcoords");
                goto failed_malloc_texcoord;
            }

            vertex_size += sizeof(vector2_t);
            break;
        default:
            ASSERT(false, "Invalid semantic");
            goto invalid_semantics;
        }
    }

    for (size_t i = 0; i < num_vertices; ++i)
    {
        for (size_t j = 0; j < num_semantics; ++j)
        {
            switch (p_semantics[j])
            {
            case SEMANTIC_POSITION:
                memcpy(&p_buffer->pa_positions[i], p_vertex, sizeof(vector2_t));
                p_vertex += sizeof(vector2_t);
                break;
            case SEMANTIC_COLOR:
                memcpy(&p_buffer->pa_colors[i], p_vertex, sizeof(color_t));
                p_vertex += sizeof(color_t);
                break;
            case SEMANTIC_TEXCOORD:
                memcpy(&p_buffer->pa_texcoords[i], p_vertex, sizeof(vector2_t));
                p_vertex += sizeof(vector2_t);
                break;
            default:
                ASSERT(false, "Invalid semantic");
                goto invalid_semantics;
            }
        }
    }

    p_buffer->num_vertices = num_vertices;
    *pp_out_vertex_buffer = (i_vertex_buffer2_t*)p_buffer;

    return true;

invalid_semantics:
    SAFE_FREE(p_buffer->pa_texcoords);

failed_malloc_texcoord:
    SAFE_FREE(p_buffer->pa_colors);

failed_malloc_colors:
    SAFE_FREE(p_buffer->pa_positions);

failed_malloc_positions:
    p_vertex_buffer->vtbl->release(p_vertex_buffer);
    *pp_out_vertex_buffer = NULL;
    return false;
}

static bool __stdcall create_index_buffer(const i_soft_renderer_2d_t* p_this,
                                          const uint_t* p_indices, const size_t num_indices,
                                          i_index_buffer2_t** pp_out_index_buffer)
{
    ASSERT(p_indices != NULL, "p_indices == NULL");
    ASSERT(pp_out_index_buffer != NULL, "pp_out_index_buffer == NULL");
    ASSERT(num_indices != 0, "num_indices == 0");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

    i_index_buffer2_t* p_index_buffer;
    create_index_buffer2_private(&p_index_buffer);

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_index_buffer;

    p_buffer->pa_indices = (uint_t*)malloc(sizeof(uint_t) * num_indices);
    if (p_buffer->pa_indices == NULL)
    {
        ASSERT(false, "Failed to malloc indices");
        return false;
    }
    memcpy(p_buffer->pa_indices, p_indices, sizeof(uint_t) * num_indices);

    p_buffer->num_indices = num_indices;
    *pp_out_index_buffer = (i_index_buffer2_t*)p_buffer;

    return true;
}

bool __stdcall create_mesh (const i_soft_renderer_2d_t* p_this,
                            i_vertex_buffer2_t* p_vertex_buffer,
                            i_index_buffer2_t* p_index_buffer,
                            i_texture2_t* p_texture,
                            i_mesh2_t** pp_out_mesh)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_vertex_buffer != NULL, "p_vertex_buffer == NULL");
    ASSERT(p_index_buffer != NULL, "p_index_buffer == NULL");
    ASSERT(p_texture != NULL, "p_texture == NULL");
    ASSERT(pp_out_mesh != NULL, "pp_out_mesh == NULL");

    mesh2_t* p_mesh;
    create_mesh2_private((i_mesh2_t**)&p_mesh);

    if (p_mesh == NULL)
    {
        ASSERT(false, "Failed to create mesh");
        *pp_out_mesh = NULL;
        return false;
    }

    p_mesh->p_vertex_buffer = p_vertex_buffer;
    p_mesh->p_index_buffer = p_index_buffer;
    p_mesh->p_texture = p_texture;

    p_vertex_buffer->vtbl->add_ref(p_vertex_buffer);
    p_index_buffer->vtbl->add_ref(p_index_buffer);
    p_texture->vtbl->add_ref(p_texture);

    *pp_out_mesh = (i_mesh2_t*)p_mesh;

    return true;
}

static void __stdcall draw_mesh(const i_soft_renderer_2d_t* p_this, const i_mesh2_t* p_mesh, const matrix_t* p_transform_mat, const bool b_wireframe)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_mesh != NULL, "p_mesh == NULL");

    const soft_renderer_2d_t* p_renderer = (soft_renderer_2d_t*)p_this;

    const size_t window_width = p_renderer->window_width;
    const size_t window_height = p_renderer->window_height;

    const vertex_buffer2_t* p_vertex_buffer = (vertex_buffer2_t*)p_mesh->vtbl->get_vertex_buffer(p_mesh);

    const vector2_t* p_positions = p_vertex_buffer->pa_positions;
    const color_t* p_colors = p_vertex_buffer->pa_colors;
    const vector2_t* p_texcoords = p_vertex_buffer->pa_texcoords;

    const bool b_colors = (p_colors != NULL);
    const bool b_texcoords = (p_texcoords != NULL);

    // color, texcoord 둘 다 없으면 그릴 수 없음
    if (!b_colors && !b_texcoords)
    {
        ASSERT(false, "Invalid mesh");
        return;
    }

    const index_buffer2_t* p_index_buffer = (index_buffer2_t*)p_mesh->vtbl->get_index_buffer(p_mesh);

    const uint_t* p_indices = p_index_buffer->pa_indices;

    // texcoords가 들어왔으면 텍스처 불러오기
    i_texture2_t* p_texture = (b_texcoords) ? p_mesh->vtbl->get_texture(p_mesh) : NULL;

    const size_t num_triangles = p_index_buffer->num_indices / 3;
    for (size_t j = 0; j < num_triangles; ++j)
    {
        const size_t buffer_index = j * 3;

        // 버텍스 셰이더 적용
        const vector_t temp_v0 = matrix_mul_vector(vector2_to_vector(&p_positions[p_indices[buffer_index]]), *p_transform_mat);
        const vector_t temp_v1 = matrix_mul_vector(vector2_to_vector(&p_positions[p_indices[buffer_index + 1]]), *p_transform_mat);
        const vector_t temp_v2 = matrix_mul_vector(vector2_to_vector(&p_positions[p_indices[buffer_index + 2]]), *p_transform_mat);

        vector2_t v0 = to_screen_coord(vector_to_vector2(temp_v0), window_width, window_height);
        vector2_t v1 = to_screen_coord(vector_to_vector2(temp_v1), window_width, window_height);
        vector2_t v2 = to_screen_coord(vector_to_vector2(temp_v2), window_width, window_height);

        if ((v0.x < 0 || v0.x >= window_width || v0.y < 0 || v0.y >= window_height)
            && (v1.x < 0 || v1.x >= window_width || v1.y < 0 || v1.y >= window_height)
            && (v2.x < 0 || v2.x >= window_width || v2.y < 0 || v2.y >= window_height))
        {
            continue;
        }

        // 와이어프레임 모드
        if (b_wireframe)
        {
            const uint32_t color = 0xff0404f4;

            draw_line(p_this, (int)v0.x, (int)v0.y, (int)v1.x, (int)v1.y, color);
            draw_line(p_this, (int)v0.x, (int)v0.y, (int)v2.x, (int)v2.y, color);
            draw_line(p_this, (int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, color);
        }
        else
        {
            vector2_t uv0 = { 0.0f, };
            vector2_t uv1 = { 0.0f, };
            vector2_t uv2 = { 0.0f, };

            color_t color0 = { 0.0f, };
            color_t color1 = { 0.0f, };
            color_t color2 = { 0.0f, };

            if (b_colors)
            {
                memcpy(&color0, &p_colors[p_indices[buffer_index]], sizeof(color_t));
                memcpy(&color1, &p_colors[p_indices[buffer_index + 1]], sizeof(color_t));
                memcpy(&color2, &p_colors[p_indices[buffer_index + 2]], sizeof(color_t));
            }

            if (b_texcoords)
            {
                uv0.x = p_texcoords[p_indices[buffer_index]].x;
                uv0.y = p_texcoords[p_indices[buffer_index]].y;

                uv1.x = p_texcoords[p_indices[buffer_index + 1]].x;
                uv1.y = p_texcoords[p_indices[buffer_index + 1]].y;

                uv2.x = p_texcoords[p_indices[buffer_index + 2]].x;
                uv2.y = p_texcoords[p_indices[buffer_index + 2]].y;
            }

            v0.x = (float)((int)(v0.x));
            v0.y = (float)((int)(v0.y));

            v1.x = (float)((int)(v1.x));
            v1.y = (float)((int)(v1.y));

            v2.x = (float)((int)(v2.x));
            v2.y = (float)((int)(v2.y));

            vector2_t min;
            vector2_t max;

            min.x = MIN(MIN(v0.x, v1.x), v2.x);
            min.y = MIN(MIN(v0.y, v1.y), v2.y);
            max.x = MAX(MAX(v0.x, v1.x), v2.x);
            max.y = MAX(MAX(v0.y, v1.y), v2.y);

            const vector_t tv0 = vector2_to_vector(&v0);
            const vector_t tv1 = vector2_to_vector(&v1);
            const vector_t tv2 = vector2_to_vector(&v2);

            const float cross = vector_cross2(vector_sub(tv0, tv1), vector_sub(tv1, tv2));

            for (int y = (int)min.y; y <= (int)max.y; ++y)
            {
                for (int x = (int)min.x; x <= (int)max.x; ++x)
                {
                    const vector2_t temp_p = { (float)x, (float)y };
                    const vector_t p = vector2_to_vector(&temp_p);

                    const float area0 = vector_cross2(vector_sub(p, tv2), vector_sub(tv1, tv2));
                    const float area1 = vector_cross2(vector_sub(p, tv0), vector_sub(tv2, tv0));
                    const float area2 = vector_cross2(vector_sub(tv1, tv0), vector_sub(p, tv0));

                    if (area0 < 0.0f || area1 < 0.0f || area2 < 0.0f)
                    {
                        continue;
                    }

                    const float area_sum = area0 + area1 + area2;

                    const float w0 = area0 / area_sum;
                    const float w1 = area1 / area_sum;
                    const float w2 = 1 - w0 - w1;

                    if (b_texcoords)
                    {
                        const vector2_t target_uv = { uv0.x * w0 + uv1.x * w1 + uv2.x * w2, uv0.y * w0 + uv1.y * w1 + uv2.y * w2 };

                        const size_t texture_width = p_texture->vtbl->get_width(p_texture);
                        const size_t texture_height = p_texture->vtbl->get_height(p_texture);
                        const char* p_bitmap = p_texture->vtbl->get_bitmap(p_texture);

                        const size_t image_x = (size_t)(FLOOR(target_uv.x * texture_width)) % texture_width;
                        const size_t image_y = (size_t)(FLOOR(target_uv.y * texture_height)) % texture_height;
                        const size_t index = image_y * texture_width + image_x;

                        const uint32_t color = *(uint32_t*)(p_bitmap + index * 4);
                        draw_pixel(p_this, x, y, color);
                    }
                    else
                    {
                        vector_t cv0 = vector4_to_vector((vector4_t*)&color0);
                        vector_t cv1 = vector4_to_vector((vector4_t*)&color1);
                        vector_t cv2 = vector4_to_vector((vector4_t*)&color2);

                        cv0 = vector_mul_scalar(cv0, w0);
                        cv1 = vector_mul_scalar(cv1, w1);
                        cv2 = vector_mul_scalar(cv2, w2);

                        const vector_t temp = vector_add(vector_add(cv0, cv1), cv2);
                        const vector4_t cv = vector_to_vector4(temp);

                        const color_t color = { cv.x, cv.y, cv.z, 1.0f };

                        draw_pixel(p_this, x, y, color_to_argb(color));
                    }
                }
            }
        }
    }
}

static bool __stdcall create_back_buffer_private(soft_renderer_2d_t* p_renderer, const size_t width, const size_t height)
{
    ASSERT(p_renderer != NULL, "p_renderer == NULL");
    ASSERT(p_renderer->p_ddraw7 != NULL, "ddraw7 object == NULL");

    HRESULT hr;
    DDSURFACEDESC2 ddsd = { 0, };

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    ddsd.dwHeight = (DWORD)height;
    ddsd.dwWidth = (DWORD)width;

    hr = p_renderer->p_ddraw7->CreateSurface(&ddsd, &p_renderer->p_back, NULL);
    if (FAILED(hr))
    {
        return false;
    }

    p_renderer->window_width = width;
    p_renderer->window_height = height;

    return true;
}

static int __stdcall get_region(const int sx, const int sy, const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y)
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

static vector2_t __stdcall to_screen_coord(const vector2_t cartesian_coord, const size_t screen_width, const size_t screen_height)
{
    vector2_t v = { cartesian_coord.x + (float)screen_width * 0.5f, -cartesian_coord.y + (float)screen_height * 0.5f };
    return v;
}

static vector2_t __stdcall to_cartesian_coord(const vector2_t screen_coord, const size_t screen_width, const size_t screen_height)
{
    const vector2_t v = { screen_coord.x - (float)screen_width * 0.5f + 0.5f, -(screen_coord.y + 0.5f) + (float)screen_height * 0.5f };
    return v;
}

void __stdcall create_instance(void** pp_out_instance)
{
    ASSERT(pp_out_instance != NULL, "pp_out_instance == NULL");

    static i_soft_renderer_2d_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        initialize,

        get_width,
        get_height,
        update_window_pos,
        update_window_size,

        begin_draw,
        end_draw,
        on_draw,

        begin_gdi,
        end_gdi,
        print_text,

        clear,
        draw_pixel,
        draw_rectangle,
        draw_line,
        draw_bitmap,

        clip_line,

        create_vertex_buffer,
        create_index_buffer,
        create_mesh,

        draw_mesh
    };

    soft_renderer_2d_t* pa_renderer = (soft_renderer_2d_t*)malloc(sizeof(soft_renderer_2d_t));
    if (pa_renderer == NULL)
    {
        ASSERT(false, "Failed to malloc renderer");
        *pp_out_instance = NULL;
    }

    pa_renderer->base.vtbl = &vtbl;
    pa_renderer->ref_count = 1;

    *pp_out_instance = pa_renderer;
}