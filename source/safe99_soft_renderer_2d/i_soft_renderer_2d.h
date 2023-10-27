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

#include <Windows.h>

#include "safe99_common/defines.h"

typedef enum semantic
{
    SEMANTIC_POSITION = 0x01,
    SEMANTIC_COLOR = 0x02,
    SEMANTIC_TEXCOORD = 0x04
} semantic_t;

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

    bool        (__stdcall*     initialize)(i_soft_renderer_2d_t* p_this, HWND hwnd);

    size_t      (__stdcall*     get_width)(const i_soft_renderer_2d_t* p_this);
    size_t      (__stdcall*     get_height)(const i_soft_renderer_2d_t* p_this);

    void        (__stdcall*     update_window_pos)(i_soft_renderer_2d_t* p_this);
    void        (__stdcall*     update_window_size)(i_soft_renderer_2d_t* p_this);

    bool        (__stdcall*     begin_draw)(i_soft_renderer_2d_t* p_this);
    void        (__stdcall*     end_draw)(i_soft_renderer_2d_t* p_this);
    void        (__stdcall*     on_draw)(const i_soft_renderer_2d_t* p_this);

    bool        (__stdcall*     begin_gdi)(i_soft_renderer_2d_t* p_this, HDC* p_out_hdc);
    void        (__stdcall*     end_gdi)(i_soft_renderer_2d_t* p_this, const HDC hdc);
    void        (__stdcall*     print_text)(i_soft_renderer_2d_t* p_this, const HDC hdc,
                                            const wchar_t* text, const int dx, const int dy, const size_t length, const uint32_t rgb);

    void        (__stdcall*     clear)(i_soft_renderer_2d_t* p_this, const uint32_t rgb);
    void        (__stdcall*     draw_pixel)(i_soft_renderer_2d_t* p_this, const int dx, const int dy, const uint32_t rgb);
    void        (__stdcall*     draw_rectangle)(i_soft_renderer_2d_t* p_this,
                                                const int dx, const int dy,
                                                const size_t width, const size_t height, const uint32_t rgb);
    void        (__stdcall*     draw_line)(i_soft_renderer_2d_t* p_this,
                                           const int sx, const int sy,
                                           const int dx, const int dy,
                                           const uint32_t rgb);
    void        (__stdcall*     draw_bitmap)(i_soft_renderer_2d_t* p_this,
                                             const int dx, const int dy,
                                             const int sx, const int sy, const size_t sw, const size_t sh,
                                             const size_t width, const size_t height, const char* p_bitmap);

    bool        (__stdcall*     clip_line)(int* p_out_sx, int* p_out_sy,
                                           int* p_out_dx, int* p_out_dy,
                                           const int left_top_x, const int left_top_y, const int right_bottom_x, const int right_bottom_y);

    bool        (__stdcall*     create_vertex_buffer)(const i_soft_renderer_2d_t* p_this,
                                                      const semantic_t* p_semantics, const size_t* p_offsets, const size_t num_semantics,
                                                      const void* p_vertices, const size_t num_vertices,
                                                      i_vertex_buffer2_t** pp_out_vertex_buffer);
    bool        (__stdcall*     create_index_buffer)(const i_soft_renderer_2d_t* p_this,
                                                     const uint_t* p_indices, const size_t num_indices,
                                                     i_index_buffer2_t** pp_out_index_buffer);
    bool        (__stdcall*     create_mesh)(const i_soft_renderer_2d_t* p_this,
                                             i_vertex_buffer2_t* p_vertex_buffer,
                                             i_index_buffer2_t* p_index_buffer,
                                             i_texture2_t* p_texture,
                                             i_mesh2_t** pp_out_mesh);

    void        (__stdcall*     draw_mesh)(i_soft_renderer_2d_t* p_this,
                                           const i_mesh2_t* p_mesh, const matrix_t* p_transform_mat, const bool b_wireframe);
} i_soft_renderer_2d_vtbl_t;

typedef interface i_soft_renderer_2d
{
    i_soft_renderer_2d_vtbl_t* vtbl;
} i_soft_renderer_2d_t;

#endif // I_SOFT_RENDERER_2D_H