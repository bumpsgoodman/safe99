//***************************************************************************
// 
// 파일: i_soft_renderer_2d.h
// 
// 설명: 2D 소프트웨어 렌더러 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/11
// 
//***************************************************************************

#ifndef I_SOFT_RENDERER_2D_H
#define I_SOFT_RENDERER_2D_H

#include <ddraw.h>

#include "safe99_common/defines.h"
#include "safe99_common/types.h"
#include "safe99_math/math.h"

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

typedef enum semantic
{
    SEMANTIC_POSITION = 0x01,
    SEMANTIC_COLOR = 0x02,
    SEMANTIC_TEXCOORD = 0x04
} semantic_t;

typedef struct vertex_buffer
{
    vector2_t* pa_positions;
    color_t* pa_colors;
    vector2_t* pa_texture_coords;

    size_t num_vertices;
} vertex_buffer_t;

typedef struct index_buffer
{
    uint_t* pa_indices;
    size_t num_indices;
} index_buffer_t;

typedef struct vertex_shader
{
    const matrix_t* p_transform_matrix;
} vertex_shader_t;

typedef interface i_soft_renderer_2d i_soft_renderer_2d_t;

typedef interface i_soft_renderer_2d_vtbl
{
    size_t      (__stdcall*     add_ref)(i_soft_renderer_2d_t* p_this);
    size_t      (__stdcall*     release)(i_soft_renderer_2d_t* p_this);
    size_t      (__stdcall*     get_ref_count)(const i_soft_renderer_2d_t* p_this);

    bool        (__stdcall*     init)(i_soft_renderer_2d_t* p_this, HWND hwnd, const size_t num_max_objects);

    size_t      (__stdcall*     get_width)(const i_soft_renderer_2d_t* p_this);
    size_t      (__stdcall*     get_height)(const i_soft_renderer_2d_t* p_this);

    void        (__stdcall*     update_window_pos)(i_soft_renderer_2d_t* p_this);
    void        (__stdcall*     update_window_size)(i_soft_renderer_2d_t* p_this);

    bool        (__stdcall*     begin_draw)(i_soft_renderer_2d_t* p_this);
    void        (__stdcall*     end_draw)(i_soft_renderer_2d_t* p_this);
    void        (__stdcall*     on_draw)(i_soft_renderer_2d_t* p_this);

    void        (__stdcall*     clear)(i_soft_renderer_2d_t* p_this, const uint32_t argb);
    void        (__stdcall*     draw_pixel)(i_soft_renderer_2d_t* p_this, const int dx, const int dy, const uint32_t argb);
    void        (__stdcall*     draw_rectangle)(i_soft_renderer_2d_t* p_this,
                                                const int dx, const int dy,
                                                const size_t width, const size_t height, const uint32_t argb);
    void        (__stdcall*     draw_line)(i_soft_renderer_2d_t* p_this,
                                           const int sx, const int sy,
                                           const int dx, const int dy,
                                           const uint32_t argb);
    void        (__stdcall*     draw_bitmap)(i_soft_renderer_2d_t* p_this,
                                             const int dx, const int dy,
                                             const int sx, const int sy, const size_t sw, const size_t sh,
                                             const size_t width, const size_t height, const char* p_bitmap);

    bool        (__stdcall*     clip_line)(int* p_out_sx, int* p_out_sy,
                                           int* p_out_dx, int* p_out_dy,
                                           const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y);

    bool        (__stdcall*     create_vertex_buffer)(const semantic_t* p_semantics, const size_t num_semantics,
                                                      const void* p_vertices, const size_t* p_offsets,
                                                      const size_t num_vertices, vertex_buffer_t* p_out_vertex_buffer);
    bool        (__stdcall*     create_index_buffer)(const uint_t* p_indices, const size_t num_indices, index_buffer_t* p_out_index_buffer);
    bool        (__stdcall*     create_vertex_shader)(const matrix_t* p_transform_matrix, vertex_shader_t* p_out_vertex_shader);

    void        (__stdcall*     set_vertex_buffer)(i_soft_renderer_2d_t* p_this, const vertex_buffer_t* p_vertex_buffer);
    void        (__stdcall*     set_index_buffer)(i_soft_renderer_2d_t* p_this, const index_buffer_t* p_index_buffer);
    void        (__stdcall*     set_vertex_shader)(i_soft_renderer_2d_t* p_this, vertex_shader_t* p_vertex_shader);

    bool        (__stdcall*     draw)(i_soft_renderer_2d_t* p_this);
} i_soft_renderer_2d_vtbl_t;

typedef interface i_soft_renderer_2d
{
    i_soft_renderer_2d_vtbl_t* vtbl;
} i_soft_renderer_2d_t;

#endif // I_SOFT_RENDERER_2D_H