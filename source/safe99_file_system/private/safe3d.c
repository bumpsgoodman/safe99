//***************************************************************************
// 
// 파일: safe3d.c
// 
// 설명: safe99 전용 3d 모델
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/09
// 
//***************************************************************************

#include "precompiled.h"

typedef struct safe3d
{
    i_safe3d_t base;
    size_t ref_count;

    char magic[8]; // "safe3d"

    vector3_t* pa_vertices;
    uint_t* pa_indices;

    size_t num_vertices;
    size_t num_indices;
    color_t wireframe_color;
} safe3d_t;

static size_t __stdcall add_ref(i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    safe3d_t* p_safe3d = (safe3d_t*)p_this;
    return ++p_safe3d->ref_count;
}

static size_t __stdcall release(i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    safe3d_t* p_safe3d = (safe3d_t*)p_this;
    if (--p_safe3d->ref_count == 0)
    {
        SAFE_FREE(p_safe3d->pa_vertices);
        SAFE_FREE(p_safe3d->pa_indices);
        SAFE_FREE(p_this);

        return 0;
    }

    return p_safe3d->ref_count;
}

static size_t __stdcall get_ref_count(const i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const safe3d_t* p_safe3d = (safe3d_t*)p_this;
    return p_safe3d->ref_count;
}

static const vector3_t* __stdcall get_vertices(const i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const safe3d_t* p_safe3d = (safe3d_t*)p_this;
    return p_safe3d->pa_vertices;
}

static const uint_t* __stdcall get_indices(const i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const safe3d_t* p_safe3d = (safe3d_t*)p_this;
    return p_safe3d->pa_indices;
}

static size_t __stdcall get_num_vertices(const i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const safe3d_t* p_safe3d = (safe3d_t*)p_this;
    return p_safe3d->num_vertices;
}

static size_t __stdcall get_num_indices(const i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const safe3d_t* p_safe3d = (safe3d_t*)p_this;
    return p_safe3d->num_indices;
}

static color_t __stdcall get_wireframe_color(const i_safe3d_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const safe3d_t* p_safe3d = (safe3d_t*)p_this;
    return p_safe3d->wireframe_color;
}

bool create_safe3d_private(i_safe3d_t** pp_out_safe3d)
{
    ASSERT(pp_out_safe3d != NULL, "pp_out_safe3d == NULL");

    static i_safe3d_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_vertices,
        get_indices,

        get_num_vertices,
        get_num_indices,
        get_wireframe_color
    };

    safe3d_t* pa_safe3d = (safe3d_t*)malloc(sizeof(safe3d_t));
    if (pa_safe3d == NULL)
    {
        ASSERT(false, "Failed to malloc safe3d");
        *pp_out_safe3d = NULL;
        return false;
    }

    pa_safe3d->base.vtbl = &vtbl;
    pa_safe3d->ref_count = 1;

    *pp_out_safe3d = (i_safe3d_t*)pa_safe3d;

    return true;
}

bool initialize_safe3d_private(i_safe3d_t* p_this,
                               vector3_t* p_vertices, const size_t num_vertices,
                               uint_t* p_indices, const size_t num_indices,
                               const color_t wireframe_color)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_vertices != NULL, "p_vertices == NULL");
    ASSERT(p_indices != NULL, "p_indices == NULL");

    safe3d_t* p_safe3d = (safe3d_t*)p_this;

    strcpy(p_safe3d->magic, "safe3d");
    p_safe3d->pa_vertices = p_vertices;
    p_safe3d->pa_indices = p_indices;
    p_safe3d->num_vertices = num_vertices;
    p_safe3d->num_indices = num_indices;
    p_safe3d->wireframe_color = wireframe_color;

    return true;
}