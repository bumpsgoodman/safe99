//***************************************************************************
// 
// 파일: mesh.c
// 
// 설명: 메시
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/14
// 
//***************************************************************************

#include "precompiled.h"

typedef struct mesh
{
    i_mesh_t base;
    size_t ref_count;

    i_vertex_buffer_t* p_vertex_buffer;
    i_index_buffer_t* p_index_buffer;
    i_texture_t* p_texture;
    color_t wireframe_color;
} mesh_t;

static size_t __stdcall add_ref(i_mesh_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh_t* p_mesh = (mesh_t*)p_this;
    return ++p_mesh->ref_count;
}

static size_t __stdcall release(i_mesh_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh_t* p_mesh = (mesh_t*)p_this;
    if (--p_mesh->ref_count == 0)
    {
        SAFE_RELEASE(p_mesh->p_texture);
        SAFE_RELEASE(p_mesh->p_index_buffer);
        SAFE_RELEASE(p_mesh->p_vertex_buffer);

        SAFE_FREE(p_this);

        return 0;
    }

    return p_mesh->ref_count;
}

static size_t __stdcall get_ref_count(const i_mesh_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh_t* p_mesh = (mesh_t*)p_this;
    return p_mesh->ref_count;
}

static i_vertex_buffer_t* __stdcall get_vertex_buffer(const i_mesh_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh_t* p_mesh = (mesh_t*)p_this;
    p_mesh->p_vertex_buffer->vtbl->add_ref(p_mesh->p_vertex_buffer);
    return p_mesh->p_vertex_buffer;
}

static i_index_buffer_t* __stdcall get_index_buffer(const i_mesh_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh_t* p_mesh = (mesh_t*)p_this;
    p_mesh->p_index_buffer->vtbl->add_ref(p_mesh->p_index_buffer);
    return p_mesh->p_index_buffer;
}

static i_texture_t* __stdcall get_texture(const i_mesh_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh_t* p_mesh = (mesh_t*)p_this;
    p_mesh->p_texture->vtbl->add_ref(p_mesh->p_texture);
    return p_mesh->p_texture;
}

static color_t __stdcall get_wireframe_color(const i_mesh_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh_t* p_mesh = (mesh_t*)p_this;
    return p_mesh->wireframe_color;
}

void __stdcall create_mesh_private(i_mesh_t** pp_out_mesh)
{
    ASSERT(pp_out_mesh != NULL, "pp_out_mesh == NULL");

    mesh_t* p_mesh = (mesh_t*)malloc(sizeof(mesh_t));
    if (p_mesh == NULL)
    {
        ASSERT(false, "Failed to malloc mesh");
        *pp_out_mesh = NULL;
    }

    static i_mesh_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_vertex_buffer,
        get_index_buffer,
        get_texture,
        get_wireframe_color
    };

    p_mesh->base.vtbl = &vtbl;
    p_mesh->ref_count = 1;

    *pp_out_mesh = (i_mesh_t*)p_mesh;
}