#ifndef RENDERER_DDRAW_H
#define RENDERER_DDRAW_H

#include <ddraw.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "safe99_common/defines.h"

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

typedef struct renderer_ddraw
{
    HWND hwnd;
    RECT window_rect;
    size_t window_width;
    size_t window_height;

    IDirectDraw* p_ddraw1;
    IDirectDraw7* p_ddraw7;
    IDirectDrawSurface7* p_primary;
    IDirectDrawSurface7* p_back;
    IDirectDrawClipper* p_clipper;

    char* p_locked_back_buffer;
    size_t locked_back_buffer_pitch;
} renderer_ddraw_t;

START_EXTERN_C

SAFE99_API bool renderer_ddraw_init(renderer_ddraw_t* p_ddraw, HWND hwnd);
SAFE99_API void renderer_ddraw_release(renderer_ddraw_t* p_ddraw);

SAFE99_API size_t renderer_ddraw_get_width(renderer_ddraw_t* p_ddraw);
SAFE99_API size_t renderer_ddraw_get_height(renderer_ddraw_t* p_ddraw);

SAFE99_API void renderer_ddraw_update_window_pos(renderer_ddraw_t* p_ddraw);
SAFE99_API void renderer_ddraw_update_window_size(renderer_ddraw_t* p_ddraw);

SAFE99_API bool renderer_ddraw_begin_draw(renderer_ddraw_t* p_ddraw);
SAFE99_API void renderer_ddraw_end_draw(renderer_ddraw_t* p_ddraw);
SAFE99_API void renderer_ddraw_on_draw(renderer_ddraw_t* p_ddraw);

SAFE99_API void renderer_ddraw_clear(renderer_ddraw_t* p_ddraw, const uint32_t argb);
SAFE99_API void renderer_ddraw_draw_pixel(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const uint32_t argb);
SAFE99_API void renderer_ddraw_draw_rectangle(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const size_t width, const size_t height, const uint32_t argb);
SAFE99_API void renderer_ddraw_draw_horizontal_line(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const int32_t length, const uint32_t argb);
SAFE99_API void renderer_ddraw_draw_vertical_line(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const int32_t length, const uint32_t argb);
SAFE99_API void renderer_ddraw_draw_bitmap(renderer_ddraw_t* p_ddraw, const int32_t dx, const int32_t dy, const int32_t sx, const int32_t sy, const size_t sw, const size_t sh, const size_t width, const size_t height, const char* p_bitmap);

END_EXTERN_C

#endif // RENDERER_DDRAW_H