//***************************************************************************
// 
// 파일: i_soft_renderer.h
// 
// 설명: GDI를 활용한 2D/3D 소프트 렌더러
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/13
// 
//***************************************************************************

#ifndef I_SOFT_RENDERER_H
#define I_SOFT_RENDERER_H

#include <Windows.h>

#include "safe99_common/defines.h"

typedef enum semantic
{
    SEMANTIC_POSITION = 0x01,
    SEMANTIC_COLOR = 0x02,
    SEMANTIC_TEXCOORD = 0x04
} semantic_t;

typedef interface i_soft_renderer i_soft_renderer_t;

typedef interface i_soft_renderer_vtbl
{
    size_t      (__stdcall*     add_ref)(i_soft_renderer_t* p_this);
    size_t      (__stdcall*     release)(i_soft_renderer_t* p_this);
    size_t      (__stdcall*     get_ref_count)(const i_soft_renderer_t* p_this);
    
    size_t      (__stdcall*     get_width)(const i_soft_renderer_t* p_this);
    size_t      (__stdcall*     get_height)(const i_soft_renderer_t* p_this);

    void        (__stdcall*     update_window_size)(i_soft_renderer_t* p_this);

    bool        (__stdcall*     initialize)(i_soft_renderer_t* p_this, HWND hwnd, const bool b_full_screen);

    void        (__stdcall*     on_draw)(const i_soft_renderer_t* p_this);

    void        (__stdcall*     clear)(i_soft_renderer_t* p_this, const color_t color);
    void        (__stdcall*     draw_pixel)(i_soft_renderer_t* p_this, const int x, const int y, const color_t color);
    void        (__stdcall*     draw_horizontal_line)(i_soft_renderer_t* p_this, const int y, const color_t color);
    void        (__stdcall*     draw_vertical_line)(i_soft_renderer_t* p_this, const int x, const color_t color);
    void        (__stdcall*     draw_line)(i_soft_renderer_t* p_this,
                                           const int start_x, const int start_y,
                                           const int end_x, const int end_y,
                                           const color_t color);
    void        (__stdcall*     draw_rectangle)(i_soft_renderer_t* p_this,
                                                const int x, const int y,
                                                const size_t width, const size_t height,
                                                const color_t color);
    void        (__stdcall*     draw_bitmap)(i_soft_renderer_t* p_this,
                                             const int x, const int y,
                                             const int sprite_x, const int sprite_y, const size_t sprite_width, const size_t sprite_height,
                                             const uint32_t* p_bitmap, const size_t width, const size_t height);
    void        (__stdcall*     draw_text)(i_soft_renderer_t* p_this, const int x, const int y,
                                           const wchar_t* text, const size_t length, const color_t color);

    bool        (__stdcall*     clip_line)(int* p_out_start_x, int* p_out_start_y,
                                           int* p_out_end_x, int* p_out_end_y,
                                           const rect_t* p_clip_window);

    bool        (__stdcall*     create_vertex_buffer)(const i_soft_renderer_t* p_this,
                                                      const size_t* p_offsets, const semantic_t* p_semantics, const size_t num_semantics,
                                                      const void* p_vertices, const size_t num_vertices,
                                                      i_vertex_buffer_t** pp_out_vertex_buffer);
    bool        (__stdcall*     create_index_buffer)(const i_soft_renderer_t* p_this,
                                                     const uint_t* p_indices, const size_t num_indices,
                                                     i_index_buffer_t** pp_out_index_buffer);
    bool        (__stdcall*     create_mesh)(const i_soft_renderer_t* p_this,
                                             i_vertex_buffer_t* p_vertex_buffer,
                                             i_index_buffer_t* p_index_buffer,
                                             i_texture_t* p_texture_or_null,
                                             const color_t wireframe_color,
                                             i_mesh_t** pp_out_mesh);

    void        (__stdcall*     draw_mesh2)(i_soft_renderer_t* p_this,
                                            const i_mesh_t* p_mesh, const matrix_t* p_transform_mat);
    void        (__stdcall*     draw_mesh3)(i_soft_renderer_t* p_this,
                                            const i_mesh_t* p_mesh, const matrix_t* p_transform_mat);

    void        (__stdcall*     toggle_wireframe)(i_soft_renderer_t* p_this);
    void        (__stdcall*     toggle_backface_culling)(i_soft_renderer_t* p_this);
} i_soft_renderer_vtbl_t;

typedef interface i_soft_renderer
{
    i_soft_renderer_vtbl_t* vtbl;
} i_soft_renderer_t;

#endif // I_SOFT_RENDERER_H