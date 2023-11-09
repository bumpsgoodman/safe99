//***************************************************************************
// 
// 파일: vertex_buffer3.c
// 
// 설명: 3D 버텍스 버퍼
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/09
// 
//***************************************************************************

#include <memory.h>

#include "safe99_common/defines.h"

typedef struct vertex_buffer3
{
    i_vertex_buffer3_t base;
    size_t ref_count;

    vector3_t* pa_positions;
    color_t* pa_colors;
    vector3_t* pa_texcoords;

    size_t num_vertices;
} vertex_buffer3_t;

static size_t __stdcall add_ref(i_vertex_buffer3_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    vertex_buffer3_t* p_buffer = (vertex_buffer3_t*)p_this;
    return ++p_buffer->ref_count;
}

static size_t __stdcall release(i_vertex_buffer3_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    vertex_buffer3_t* p_buffer = (vertex_buffer3_t*)p_this;
    if (--p_buffer->ref_count == 0)
    {
        SAFE_FREE(p_buffer->pa_positions);
        SAFE_FREE(p_buffer->pa_colors);
        SAFE_FREE(p_buffer->pa_texcoords);

        SAFE_FREE(p_this);

        return 0;
    }

    return p_buffer->ref_count;
}

static size_t __stdcall get_ref_count(const i_vertex_buffer3_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    vertex_buffer3_t* p_buffer = (vertex_buffer3_t*)p_this;
    return p_buffer->ref_count;
}

static const vector3_t* __stdcall get_positions(const i_vertex_buffer3_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    vertex_buffer3_t* p_buffer = (vertex_buffer3_t*)p_this;
    return p_buffer->pa_positions;
}

static const color_t* __stdcall get_colors_or_null(const i_vertex_buffer3_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    vertex_buffer3_t* p_buffer = (vertex_buffer3_t*)p_this;
    return p_buffer->pa_colors;
}

static const vector3_t* __stdcall get_tex_coord_or_null(const i_vertex_buffer3_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    vertex_buffer3_t* p_buffer = (vertex_buffer3_t*)p_this;
    return p_buffer->pa_texcoords;
}

static size_t __stdcall get_num_vertices(const i_vertex_buffer3_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    vertex_buffer3_t* p_buffer = (vertex_buffer3_t*)p_this;
    return p_buffer->num_vertices;
}

void __stdcall create_vertex_buffer3_private(i_vertex_buffer3_t** pp_out_vertex_buffer)
{
    ASSERT(pp_out_vertex_buffer != NULL, "pp_out_vertex_buffer == NULL");

    vertex_buffer3_t* p_vertex_buffer = (vertex_buffer3_t*)malloc(sizeof(vertex_buffer3_t));
    if (p_vertex_buffer == NULL)
    {
        ASSERT(false, "Failed to malloc vertex buffer");
        *pp_out_vertex_buffer = NULL;
    }

    static i_vertex_buffer3_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_positions,
        get_colors_or_null,
        get_tex_coord_or_null,

        get_num_vertices
    };

    p_vertex_buffer->base.vtbl = &vtbl;
    p_vertex_buffer->ref_count = 1;

    *pp_out_vertex_buffer = (i_vertex_buffer3_t*)p_vertex_buffer;
}