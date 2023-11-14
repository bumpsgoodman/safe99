//***************************************************************************
// 
// 파일: i_soft_renderer.c
// 
// 설명: GDI를 활용한 2D/3D 소프트 렌더러
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/13
// 
//***************************************************************************

#include "precompiled.h"
#include "i_soft_renderer.h"

#define ALIGNED_BYTE sizeof(uint32_t)
#define GET_SEMANTIC(semantic) ((semantic) & UINT_MAX)

typedef struct vertex_buffer
{
    i_vertex_buffer_t base;
    size_t ref_count;

    vector3_t* pa_positions;
    color_t* pa_colors;
    vector2_t* pa_texcoords;

    size_t num_vertices;
} vertex_buffer_t;

typedef struct index_buffer
{
    i_index_buffer_t base;
    size_t ref_count;

    uint_t* pa_indices;
    size_t num_indices;
} index_buffer_t;

typedef struct mesh
{
    i_mesh_t base;
    size_t ref_count;

    i_vertex_buffer_t* p_vertex_buffer;
    i_index_buffer_t* p_index_buffer;
    i_texture_t* p_texture;
    color_t wireframe_color;
} mesh_t;

typedef struct buffer
{
    uint32_t* pa_surface;
    size_t width;
    size_t height;
    size_t pitch;
} buffer_t;

typedef struct soft_renderer
{
    i_soft_renderer_t base;
    size_t ref_count;

    // 윈도우
    HWND hwnd;
    size_t window_width;
    size_t window_height;

    HDC hdc;
    HDC hmem_dc;

    bool b_full_screen;

    // 버퍼
    buffer_t* pa_buffer;

    HBITMAP hdi_bitmap;
    HBITMAP hdefault_bitmap;
} soft_renderer_t;

enum
{
    REGION_LEFT = 0x0001,
    REGION_RIGHT = 0x0010,
    REGION_TOP = 0x1000,
    REGION_BOTTOM = 0x0100
};

extern void __stdcall create_vertex_buffer_private(i_vertex_buffer_t** pp_out_vertex_buffer);
extern void __stdcall create_index_buffer_private(i_index_buffer_t** pp_out_index_buffer);
extern void __stdcall create_mesh_private(i_mesh_t** pp_out_mesh);

static bool update_buffer(soft_renderer_t* p_soft_renderer);

static int get_region(const int x, const int y, const rect_t* p_clip_window);
static vector3_t to_screen_coord(const vector3_t cartesian_coord, const size_t screen_width, const size_t screen_height);
static vector3_t to_cartesian_coord(const vector3_t screen_coord, const size_t screen_width, const size_t screen_height);

void __stdcall create_instance(void** pp_out_instance);

static size_t __stdcall add_ref(i_soft_renderer_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    return ++p_renderer->ref_count;
}

static size_t __stdcall release(i_soft_renderer_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    if (--p_renderer->ref_count == 0)
    {
        SAFE_FREE(p_renderer->pa_buffer->pa_surface);
        SAFE_FREE(p_renderer->pa_buffer);

        DeleteObject(p_renderer->hdi_bitmap);
        DeleteObject(p_renderer->hdefault_bitmap);
        ReleaseDC(p_renderer->hwnd, p_renderer->hmem_dc);
        ReleaseDC(p_renderer->hwnd, p_renderer->hdc);

        SAFE_FREE(p_renderer);

        return 0;
    }

    return p_renderer->ref_count;
}

static size_t __stdcall get_ref_count(const i_soft_renderer_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    return p_renderer->ref_count;
}

static size_t __stdcall get_width(const i_soft_renderer_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    return p_renderer->window_width;
}

static size_t __stdcall get_height(const i_soft_renderer_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    return p_renderer->window_height;
}

static void __stdcall update_window_size(i_soft_renderer_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    rect_t window_rect = { 0, };
    GetClientRect(p_renderer->hwnd, (LPRECT)&window_rect);

    const size_t window_width = (size_t)window_rect.right - window_rect.left;
    const size_t window_height = (size_t)window_rect.bottom - window_rect.top;
    if (window_width == 0 || window_height == 0)
    {
        ASSERT(false, "");
        return;
    }

    p_renderer->window_width = window_width;
    p_renderer->window_height = window_height;

    update_buffer(p_renderer);
}

static void __stdcall clear(i_soft_renderer_t* p_this, const color_t color)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    buffer_t* p_buffer = p_renderer->pa_buffer;
    uint32_t* p_surface = p_buffer->pa_surface;

    const uint32_t argb = color_to_argb(color);
    for (size_t y = 0; y < p_buffer->height; ++y)
    {
        for (size_t x = 0; x < p_buffer->width; ++x)
        {
            p_surface[y * p_buffer->pitch + x] = argb;
        }
    }
}

static bool __stdcall initialize(i_soft_renderer_t* p_this, HWND hwnd, const bool b_full_screen)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(hwnd != NULL, "hwnd == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    p_renderer->hwnd = hwnd;
    p_renderer->hdc = GetDC(hwnd);
    p_renderer->hmem_dc = CreateCompatibleDC(p_renderer->hdc);

    // 버퍼 생성
    {
        p_renderer->pa_buffer = (buffer_t*)malloc(sizeof(buffer_t));
        if (p_renderer->pa_buffer == NULL)
        {
            ASSERT(false, "Failed to malloc buffer");
            goto failed_malloc_buffer;
        }

        memset(p_renderer->pa_buffer, 0, sizeof(buffer_t));
    }

    update_window_size(p_this);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = (LONG)(p_renderer->pa_buffer->pitch);
    bmi.bmiHeader.biHeight = -(LONG)(p_renderer->pa_buffer->height);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    p_renderer->hdi_bitmap = CreateDIBSection(p_renderer->hdc, &bmi, DIB_RGB_COLORS, (void**)&p_renderer->pa_buffer->pa_surface, NULL, 0);
    if (p_renderer->hdi_bitmap == NULL)
    {
        ASSERT(false, "Failed to create di_bitmap");
        goto failed_create_di_bitmap;
    }

    p_renderer->hdefault_bitmap = (HBITMAP)SelectObject(p_renderer->hmem_dc, p_renderer->hdi_bitmap);

    const size_t window_width = p_renderer->window_width;
    const size_t window_height = p_renderer->window_height;

    const size_t pitch = window_width + window_width % ALIGNED_BYTE;

    p_renderer->b_full_screen = b_full_screen;

    clear(p_this, color_set(1.0f, 1.0f, 1.0f, 1.0f));

    return true;

failed_create_di_bitmap:
    SAFE_FREE(p_renderer->pa_buffer->pa_surface);
    SAFE_FREE(p_renderer->pa_buffer);

failed_malloc_buffer:
    memset(p_renderer, 0, sizeof(soft_renderer_t));
    return false;
}

static void __stdcall on_draw(const i_soft_renderer_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    ASSERT(p_renderer->hdc != NULL, "hdc == NULL");
    ASSERT(p_renderer->hmem_dc != NULL, "hmem_dc == NULL");

    buffer_t* p_buffer = p_renderer->pa_buffer;
    BitBlt(p_renderer->hdc, 0, 0, (int)p_buffer->pitch, (int)p_buffer->height, p_renderer->hmem_dc, 0, 0, SRCCOPY);
}

static void __stdcall draw_pixel(i_soft_renderer_t* p_this, const int x, const int y, const color_t color)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    if (GET_ALPHA(color) == 0)
    {
        return;
    }

    if ((size_t)x >= p_renderer->window_width
        || (size_t)y >= p_renderer->window_height)
    {
        return;
    }

    buffer_t* p_buffer = p_renderer->pa_buffer;
    uint32_t* p_surface = p_buffer->pa_surface;
    p_surface[y * p_buffer->pitch + x] = color_to_argb(color);
}

static void __stdcall draw_horizontal_line(i_soft_renderer_t* p_this, const int y, const color_t color)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    if (GET_ALPHA(color) == 0)
    {
        return;
    }

    if ((size_t)y >= p_renderer->window_height)
    {
        return;
    }

    buffer_t* p_buffer = p_renderer->pa_buffer;
    uint32_t* p_surface = p_buffer->pa_surface;
    for (size_t x = 0; x < p_buffer->width; ++x)
    {
        p_surface[y * p_buffer->pitch + x] = color_to_argb(color);
    }
}

static void __stdcall draw_vertical_line(i_soft_renderer_t* p_this, const int x, const color_t color)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    if (GET_ALPHA(color) == 0)
    {
        return;
    }

    if ((size_t)x >= p_renderer->window_width)
    {
        return;
    }

    buffer_t* p_buffer = p_renderer->pa_buffer;
    uint32_t* p_surface = p_buffer->pa_surface;
    for (size_t y = 0; y < p_buffer->height; ++y)
    {
        p_surface[y * p_buffer->pitch + x] = color_to_argb(color);
    }
}

static bool __stdcall clip_line(int* p_out_start_x, int* p_out_start_y, int* p_out_end_x, int* p_out_end_y, const rect_t* p_clip_window)
{
    ASSERT(p_out_start_x != NULL, "p_out_start_x == NULL");
    ASSERT(p_out_start_y != NULL, "p_out_start_y == NULL");
    ASSERT(p_out_end_x != NULL, "p_out_end_x == NULL");
    ASSERT(p_out_end_y != NULL, "p_out_end_y == NULL");

    while (true)
    {
        const int start_region = get_region(*p_out_start_x, *p_out_start_y, p_clip_window);
        const int end_region = get_region(*p_out_end_x, *p_out_end_y, p_clip_window);

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
        const int width = *p_out_end_x - *p_out_start_x;
        const int height = *p_out_end_y - *p_out_start_y;

        const float slope = (width == 0) ? (float)height : (float)height / width;

        float x;
        float y;

        const int clipped_region = (start_region != 0) ? start_region : end_region;
        if (clipped_region & REGION_LEFT)
        {
            // 좌측 영역인 경우 수직 경계선과 교점 계산
            x = (float)p_clip_window->left;
            y = *p_out_start_y + slope * (x - *p_out_start_x);
        }
        else if (clipped_region & REGION_RIGHT)
        {
            // 우측 영역인 경우 수직 경계선과 교점 계산
            x = (float)p_clip_window->right;
            y = *p_out_start_y + slope * (x - *p_out_start_x);
        }
        else if (clipped_region & REGION_TOP)
        {
            // 위 영역인 경우 평 경계선과 교점 계산
            y = (float)p_clip_window->top;
            x = *p_out_start_x + (y - *p_out_start_y) / slope;
        }
        else
        {
            // 아래 영역인 경우 평 경계선과 교점 계산
            y = (float)p_clip_window->bottom;
            x = *p_out_start_x + (y - *p_out_start_y) / slope;
        }

        if (clipped_region == start_region)
        {
            *p_out_start_x = ROUND_INT(x);
            *p_out_start_y = ROUND_INT(y);
        }
        else
        {
            *p_out_end_x = ROUND_INT(x);
            *p_out_end_y = ROUND_INT(y);
        }
    }
}

static void __stdcall draw_line(i_soft_renderer_t* p_this,
                                const int start_x, const int start_y,
                                const int end_x, const int end_y,
                                const color_t color)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    if (GET_ALPHA(color) == 0)
    {
        return;
    }

    const int width = end_x - start_x;
    const int height = end_y - start_y;

    const int increament_x = (width >= 0) ? 1 : -1;
    const int increament_y = (height > 0) ? 1 : -1;

    const int w = increament_x * width;
    const int h = increament_y * height;

    // 완만한지 검사
    const bool b_gradual = (ABS(width) >= ABS(height)) ? true : false;

    int sx = start_x;
    int sy = start_y;
    int dx = end_x;
    int dy = end_y;
    if (!clip_line(&sx, &sy, &dx, &dy, &(rect_t){ 0, 0, (int)p_renderer->window_width - 1, (int)p_renderer->window_height - 1 }))
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

        while (x != dx)
        {
            draw_pixel(p_this, x, y, color);

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

        while (y != dy)
        {
            draw_pixel(p_this, x, y, color);

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

    draw_pixel(p_this, x, y, color);
}

static void __stdcall draw_rectangle(i_soft_renderer_t* p_this,
                                     const int x, const int y,
                                     const size_t width, const size_t height,
                                     const color_t color)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    if (GET_ALPHA(color) == 0)
    {
        return;
    }

    const size_t start_x = MAX(x, 0);
    const size_t start_y = MAX(y, 0);
    const size_t end_x = MIN(x + width, p_renderer->window_width);
    const size_t end_y = MIN(y + height, p_renderer->window_height);

    if (end_x >= p_renderer->window_width
        || end_y >= p_renderer->window_height)
    {
        return;
    }

    buffer_t* p_buffer = p_renderer->pa_buffer;
    uint32_t* p_surface = p_buffer->pa_surface;
    for (size_t y = start_y; y <= end_y; ++y)
    {
        for (size_t x = start_x; x <= end_x; ++x)
        {
            p_surface[y * p_buffer->pitch + x] = color_to_argb(color);
        }
    }
}

static void __stdcall draw_bitmap(i_soft_renderer_t* p_this,
                                  const int x, const int y,
                                  const int sprite_x, const int sprite_y, const size_t sprite_width, const size_t sprite_height,
                                  const uint32_t* p_bitmap, const size_t width, const size_t height)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_bitmap != NULL, "bitmap == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    const size_t start_x = MAX(x, 0);
    const size_t start_y = MAX(y, 0);
    const size_t end_x = MIN(x + sprite_width, p_renderer->window_width);
    const size_t end_y = MIN(y + sprite_height, p_renderer->window_height);

    buffer_t* p_buffer = p_renderer->pa_buffer;
    uint32_t* p_surface = p_buffer->pa_surface;
    for (size_t y = start_y; y <= end_y; ++y)
    {
        for (size_t x = start_x; x <= end_x; ++x)
        {
            const uint32_t color = p_bitmap[y * width + x];

            if (GET_ALPHA_ARGB(color) == 0)
            {
                return;
            }

            p_surface[y * p_buffer->pitch + x] = color;
        }
    }
}

static void __stdcall draw_text(i_soft_renderer_t* p_this, const int x, const int y,
                                const wchar_t* text, const size_t length, const color_t color)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    ASSERT(p_renderer->hmem_dc != NULL, "hmem_dc == NULL");

    if (GET_ALPHA(color) == 0)
    {
        return;
    }

    SetTextColor(p_renderer->hmem_dc, RGB(GET_RED(color), GET_GREEN(color), GET_BLUE(color)));
    SetBkMode(p_renderer->hmem_dc, TRANSPARENT);

    TextOut(p_renderer->hmem_dc, x, y, text, (int)length);
}

static bool __stdcall create_vertex_buffer(const i_soft_renderer_t* p_this,
                                           const size_t* p_offsets, const semantic_t* p_semantics, const size_t num_semantics,
                                           const void* p_vertices, const size_t num_vertices,
                                           i_vertex_buffer_t** pp_out_vertex_buffer)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_semantics != NULL, "p_semantics == NULL");
    ASSERT(p_offsets != NULL, "p_offsets == NULL");
    ASSERT(num_semantics != 0, "num_semantics == 0");
    ASSERT(p_vertices != NULL, "p_vertices == NULL");
    ASSERT(num_vertices != 0, "num_vertices == 0");
    ASSERT(pp_out_vertex_buffer != NULL, "pp_out_vertex_buffer == NULL");

    const soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;
    const char* p_vertex = (char*)p_vertices;

    i_vertex_buffer_t* p_vertex_buffer;
    create_vertex_buffer_private(&p_vertex_buffer);

    vertex_buffer_t* p_buffer = (vertex_buffer_t*)p_vertex_buffer;
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
            p_buffer->pa_positions = (vector3_t*)malloc(sizeof(vector3_t) * num_vertices);
            if (p_buffer->pa_positions == NULL)
            {
                ASSERT(false, "Failed to malloc positions");
                goto failed_malloc_positions;
            }

            vertex_size += sizeof(vector3_t);
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
                memcpy(&p_buffer->pa_positions[i], p_vertex, sizeof(vector3_t));
                p_vertex += sizeof(vector3_t);
                break;
            case SEMANTIC_COLOR:
                memcpy(&p_buffer->pa_colors[i], p_vertex, sizeof(color_t));
                p_vertex += sizeof(color_t);
                break;
            case SEMANTIC_TEXCOORD:
                memcpy(&p_buffer->pa_texcoords[i], p_vertex, sizeof(vector3_t));
                p_vertex += sizeof(vector3_t);
                break;
            default:
                ASSERT(false, "Invalid semantic");
                goto invalid_semantics;
            }
        }
    }

    p_buffer->num_vertices = num_vertices;
    *pp_out_vertex_buffer = (i_vertex_buffer_t*)p_buffer;

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

static bool __stdcall create_index_buffer(const i_soft_renderer_t* p_this,
                                          const uint_t* p_indices, const size_t num_indices,
                                          i_index_buffer_t** pp_out_index_buffer)
{
    ASSERT(p_indices != NULL, "p_indices == NULL");
    ASSERT(pp_out_index_buffer != NULL, "pp_out_index_buffer == NULL");
    ASSERT(num_indices != 0, "num_indices == 0");

    const soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    i_index_buffer_t* p_index_buffer;
    create_index_buffer_private(&p_index_buffer);

    index_buffer_t* p_buffer = (index_buffer_t*)p_index_buffer;

    p_buffer->pa_indices = (uint_t*)malloc(sizeof(uint_t) * num_indices * 3);
    if (p_buffer->pa_indices == NULL)
    {
        ASSERT(false, "Failed to malloc indices");
        return false;
    }
    memcpy(p_buffer->pa_indices, p_indices, sizeof(uint_t) * num_indices * 3);

    p_buffer->num_indices = num_indices;
    *pp_out_index_buffer = (i_index_buffer_t*)p_buffer;

    return true;
}

static bool __stdcall create_mesh(const i_soft_renderer_t* p_this,
                                  i_vertex_buffer_t* p_vertex_buffer,
                                  i_index_buffer_t* p_index_buffer,
                                  i_texture_t* p_texture_or_null,
                                  const color_t wireframe_color,
                                  i_mesh_t** pp_out_mesh)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_vertex_buffer != NULL, "p_vertex_buffer == NULL");
    ASSERT(p_index_buffer != NULL, "p_index_buffer == NULL");
    ASSERT(pp_out_mesh != NULL, "pp_out_mesh == NULL");

    mesh_t* p_mesh;
    create_mesh_private((i_mesh_t**)&p_mesh);

    if (p_mesh == NULL)
    {
        ASSERT(false, "Failed to create mesh");
        *pp_out_mesh = NULL;

        return false;
    }

    p_mesh->p_vertex_buffer = p_vertex_buffer;
    p_mesh->p_index_buffer = p_index_buffer;
    p_mesh->p_texture = p_texture_or_null;
    p_mesh->wireframe_color = wireframe_color;

    p_vertex_buffer->vtbl->add_ref(p_vertex_buffer);
    p_index_buffer->vtbl->add_ref(p_index_buffer);

    if (p_texture_or_null != NULL)
    {
        p_texture_or_null->vtbl->add_ref(p_texture_or_null);
    }

    *pp_out_mesh = (i_mesh_t*)p_mesh;

    return true;
}

static void __stdcall draw_mesh2(i_soft_renderer_t* p_this, const i_mesh_t* p_mesh, const matrix_t* p_transform_mat, const bool b_wireframe)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_mesh != NULL, "p_mesh == NULL");

    const soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    const size_t window_width = p_renderer->window_width;
    const size_t window_height = p_renderer->window_height;

    i_vertex_buffer_t* p_vertex_buffer = (i_vertex_buffer_t*)p_mesh->vtbl->get_vertex_buffer(p_mesh);
    const vector2_t* p_positions = (vector2_t*)p_vertex_buffer->vtbl->get_positions(p_vertex_buffer);
    const color_t* p_colors = p_vertex_buffer->vtbl->get_colors_or_null(p_vertex_buffer);
    const vector2_t* p_texcoords = p_vertex_buffer->vtbl->get_tex_coord_or_null(p_vertex_buffer);
    SAFE_RELEASE(p_vertex_buffer);

    const bool b_colors = (p_colors != NULL);
    const bool b_texcoords = (p_texcoords != NULL);

    // color, texcoord 둘 다 없으면 그릴 수 없음
    if (!b_wireframe && !b_colors && !b_texcoords)
    {
        ASSERT(false, "Invalid mesh");
        return;
    }

    i_index_buffer_t* p_index_buffer = (i_index_buffer_t*)p_mesh->vtbl->get_index_buffer(p_mesh);
    const uint_t* p_indices = p_index_buffer->vtbl->get_indices(p_index_buffer);
    const size_t num_triangles = p_index_buffer->vtbl->get_num_indices(p_index_buffer) / 3;
    SAFE_RELEASE(p_index_buffer);

    // texcoords가 들어왔으면 텍스처 불러오기
    i_texture_t* p_texture = (b_texcoords) ? p_mesh->vtbl->get_texture(p_mesh) : NULL;

    for (size_t j = 0; j < num_triangles; ++j)
    {
        const size_t buffer_index = j * 3;

        // 버텍스 셰이더 적용
        const vector_t temp_v0 = matrix_mul_vector(vector2_to_vector(&p_positions[p_indices[buffer_index]]), *p_transform_mat);
        const vector_t temp_v1 = matrix_mul_vector(vector2_to_vector(&p_positions[p_indices[buffer_index + 1]]), *p_transform_mat);
        const vector_t temp_v2 = matrix_mul_vector(vector2_to_vector(&p_positions[p_indices[buffer_index + 2]]), *p_transform_mat);

        vector3_t v0 = to_screen_coord(vector_to_vector3(temp_v0), window_width, window_height);
        vector3_t v1 = to_screen_coord(vector_to_vector3(temp_v1), window_width, window_height);
        vector3_t v2 = to_screen_coord(vector_to_vector3(temp_v2), window_width, window_height);

        if ((v0.x < 0 || v0.x >= window_width || v0.y < 0 || v0.y >= window_height)
            && (v1.x < 0 || v1.x >= window_width || v1.y < 0 || v1.y >= window_height)
            && (v2.x < 0 || v2.x >= window_width || v2.y < 0 || v2.y >= window_height))
        {
            continue;
        }

        // 와이어프레임 모드
        if (b_wireframe)
        {
            const color_t color = p_mesh->vtbl->get_wireframe_color(p_mesh);

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

            vector3_t min;
            vector3_t max;

            min.x = MIN(MIN(v0.x, v1.x), v2.x);
            min.y = MIN(MIN(v0.y, v1.y), v2.y);
            max.x = MAX(MAX(v0.x, v1.x), v2.x);
            max.y = MAX(MAX(v0.y, v1.y), v2.y);

            const vector_t tv0 = vector3_to_vector(&v0);
            const vector_t tv1 = vector3_to_vector(&v1);
            const vector_t tv2 = vector3_to_vector(&v2);

            const float cross = vector_cross2(vector_sub(tv0, tv1), vector_sub(tv1, tv2));

            for (int y = (int)min.y; y <= (int)max.y; ++y)
            {
                for (int x = (int)min.x; x <= (int)max.x; ++x)
                {
                    const vector3_t temp_p = { (float)x, (float)y };
                    const vector_t p = vector3_to_vector(&temp_p);

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
                        draw_pixel(p_this, x, y, argb_to_color(color));
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

                        draw_pixel(p_this, x, y, color);
                    }
                }
            }
        }
    }
}

static void __stdcall draw_mesh3(i_soft_renderer_t* p_this, const i_mesh_t* p_mesh, const matrix_t* p_transform_mat, const bool b_wireframe)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_mesh != NULL, "p_mesh == NULL");

    const soft_renderer_t* p_renderer = (soft_renderer_t*)p_this;

    const size_t window_width = p_renderer->window_width;
    const size_t window_height = p_renderer->window_height;

    i_vertex_buffer_t* p_vertex_buffer = (i_vertex_buffer_t*)p_mesh->vtbl->get_vertex_buffer(p_mesh);
    const vector3_t* p_positions = p_vertex_buffer->vtbl->get_positions(p_vertex_buffer);
    const color_t* p_colors = p_vertex_buffer->vtbl->get_colors_or_null(p_vertex_buffer);
    const vector2_t* p_texcoords = p_vertex_buffer->vtbl->get_tex_coord_or_null(p_vertex_buffer);
    SAFE_RELEASE(p_vertex_buffer);

    const bool b_colors = (p_colors != NULL);
    const bool b_texcoords = (p_texcoords != NULL);

    // color, texcoord 둘 다 없으면 그릴 수 없음
    if (!b_wireframe && !b_colors && !b_texcoords)
    {
        ASSERT(false, "Invalid mesh");
        return;
    }

    i_index_buffer_t* p_index_buffer = (i_index_buffer_t*)p_mesh->vtbl->get_index_buffer(p_mesh);
    const uint_t* p_indices = p_index_buffer->vtbl->get_indices(p_index_buffer);
    const size_t num_triangles = p_index_buffer->vtbl->get_num_indices(p_index_buffer);
    SAFE_RELEASE(p_index_buffer);

    // texcoords가 들어왔으면 텍스처 불러오기
    i_texture_t* p_texture = (b_texcoords) ? p_mesh->vtbl->get_texture(p_mesh) : NULL;

    for (size_t j = 0; j < num_triangles; ++j)
    {
        const size_t buffer_index = j * 3;

        // 버텍스 셰이더 적용
        const vector_t temp_v0 = matrix_mul_vector(vector3_to_vector(&p_positions[p_indices[buffer_index]]), *p_transform_mat);
        const vector_t temp_v1 = matrix_mul_vector(vector3_to_vector(&p_positions[p_indices[buffer_index + 1]]), *p_transform_mat);
        const vector_t temp_v2 = matrix_mul_vector(vector3_to_vector(&p_positions[p_indices[buffer_index + 2]]), *p_transform_mat);

        vector3_t v0 = to_screen_coord(vector_to_vector3(temp_v0), window_width, window_height);
        vector3_t v1 = to_screen_coord(vector_to_vector3(temp_v1), window_width, window_height);
        vector3_t v2 = to_screen_coord(vector_to_vector3(temp_v2), window_width, window_height);

        if ((v0.x < 0 || v0.x >= window_width || v0.y < 0 || v0.y >= window_height)
            && (v1.x < 0 || v1.x >= window_width || v1.y < 0 || v1.y >= window_height)
            && (v2.x < 0 || v2.x >= window_width || v2.y < 0 || v2.y >= window_height))
        {
            continue;
        }

        // 와이어프레임 모드
        if (b_wireframe)
        {
            const color_t color = p_mesh->vtbl->get_wireframe_color(p_mesh);

            draw_line(p_this, (int)v0.x, (int)v0.y, (int)v1.x, (int)v1.y, color);
            draw_line(p_this, (int)v0.x, (int)v0.y, (int)v2.x, (int)v2.y, color);
            draw_line(p_this, (int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, color);
        }
        else
        {
            vector3_t uv0 = { 0.0f, };
            vector3_t uv1 = { 0.0f, };
            vector3_t uv2 = { 0.0f, };

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

            vector3_t min;
            vector3_t max;

            min.x = MIN(MIN(v0.x, v1.x), v2.x);
            min.y = MIN(MIN(v0.y, v1.y), v2.y);
            max.x = MAX(MAX(v0.x, v1.x), v2.x);
            max.y = MAX(MAX(v0.y, v1.y), v2.y);

            const vector_t tv0 = vector3_to_vector(&v0);
            const vector_t tv1 = vector3_to_vector(&v1);
            const vector_t tv2 = vector3_to_vector(&v2);

            const float cross = vector_cross2(vector_sub(tv0, tv1), vector_sub(tv1, tv2));

            for (int y = (int)min.y; y <= (int)max.y; ++y)
            {
                for (int x = (int)min.x; x <= (int)max.x; ++x)
                {
                    const vector3_t temp_p = { (float)x, (float)y };
                    const vector_t p = vector3_to_vector(&temp_p);

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
                        draw_pixel(p_this, x, y, argb_to_color(color));
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

                        draw_pixel(p_this, x, y, color);
                    }
                }
            }
        }
    }
}

static bool update_buffer(soft_renderer_t* p_renderer)
{
    ASSERT(p_renderer != NULL, "p_renderer == NULL");

    const size_t window_width = p_renderer->window_width;
    const size_t window_height = p_renderer->window_height;

    const size_t pitch = window_width + window_width % ALIGNED_BYTE;

    // 버퍼 업데이트
    buffer_t* p_buffer = p_renderer->pa_buffer;
    uint32_t* p_surface = p_buffer->pa_surface;

    uint32_t* pa_new_surface = (uint32_t*)malloc(sizeof(uint32_t) * pitch * window_height);
    if (pa_new_surface == NULL)
    {
        ASSERT(false, "Failed to malloc new surface");
        goto failed_malloc_new_surface;
    }

    if (p_surface != NULL)
    {
        const size_t min_pitch = MIN(pitch, p_buffer->pitch);
        const size_t min_height = MIN(window_height, p_buffer->height);

        // 가장 작은 크기의 표면 복사
        memcpy(pa_new_surface, p_buffer->pa_surface, sizeof(uint32_t) * min_pitch * min_height);

        SAFE_FREE(p_buffer->pa_surface);
    }

    p_buffer->pa_surface = pa_new_surface;
    p_buffer->width = window_width;
    p_buffer->height = window_height;
    p_buffer->pitch = pitch;

    return true;

failed_malloc_new_surface:
    return false;
}

static int get_region(const int x, const int y, const rect_t* p_clip_window)
{
    int result = 0;
    if (x < p_clip_window->left)
    {
        result |= REGION_LEFT; // 0b0001
    }
    else if (x > p_clip_window->right)
    {
        result |= REGION_RIGHT; // 0b0010
    }

    if (y < p_clip_window->top)
    {
        result |= REGION_TOP; // 0b1000
    }
    else if (y > p_clip_window->bottom)
    {
        result |= REGION_BOTTOM; // 0b0100
    }

    return result;
}

static vector3_t to_screen_coord(const vector3_t cartesian_coord, const size_t screen_width, const size_t screen_height)
{
    vector3_t v = { cartesian_coord.x + (float)screen_width * 0.5f, -cartesian_coord.y + (float)screen_height * 0.5f };
    return v;
}

static vector3_t to_cartesian_coord(const vector3_t screen_coord, const size_t screen_width, const size_t screen_height)
{
    const vector3_t v = { screen_coord.x - (float)screen_width * 0.5f + 0.5f, -(screen_coord.y + 0.5f) + (float)screen_height * 0.5f };
    return v;
}

void __stdcall create_instance(void** pp_out_instance)
{
    ASSERT(pp_out_instance != NULL, "pp_out_instance == NULL");

    static i_soft_renderer_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_width,
        get_height,

        update_window_size,

        initialize,

        on_draw,

        clear,
        draw_pixel,
        draw_horizontal_line,
        draw_vertical_line,
        draw_line,
        draw_rectangle,
        draw_bitmap,
        draw_text,

        clip_line,

        create_vertex_buffer,
        create_index_buffer,
        create_mesh,

        draw_mesh2,
        draw_mesh3
    };

    soft_renderer_t* pa_renderer = (soft_renderer_t*)malloc(sizeof(soft_renderer_t));
    if (pa_renderer == NULL)
    {
        ASSERT(false, "Failed to malloc renderer");

        *pp_out_instance = NULL;
        return;
    }

    pa_renderer->base.vtbl = &vtbl;
    pa_renderer->ref_count = 1;

    *pp_out_instance = pa_renderer;
}