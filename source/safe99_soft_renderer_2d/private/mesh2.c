#include "safe99_common/defines.h"

typedef struct mesh2
{
    i_mesh2_t base;
    size_t ref_count;

    i_vertex_buffer2_t* p_vertex_buffer;
    i_index_buffer2_t* p_index_buffer;
    i_texture2_t* p_texture;
} mesh2_t;

static size_t __stdcall add_ref(i_mesh2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh2_t* p_mesh = (mesh2_t*)p_this;
    return ++p_mesh->ref_count;
}

static size_t __stdcall release(i_mesh2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh2_t* p_mesh = (mesh2_t*)p_this;
    if (--p_mesh->ref_count == 0)
    {
        SAFE_RELEASE(p_mesh->p_texture);
        SAFE_RELEASE(p_mesh->p_index_buffer);
        SAFE_RELEASE(p_mesh->p_vertex_buffer);

        SAFE_FREE(p_mesh);

        return 0;
    }

    return p_mesh->ref_count;
}

static size_t __stdcall get_ref_count(const i_mesh2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh2_t* p_mesh = (mesh2_t*)p_this;
    return p_mesh->ref_count;
}

static i_vertex_buffer2_t* __stdcall get_vertex_buffer(const i_mesh2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh2_t* p_mesh = (mesh2_t*)p_this;
    return p_mesh->p_vertex_buffer;
}

static i_index_buffer2_t* __stdcall get_index_buffer(const i_mesh2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh2_t* p_mesh = (mesh2_t*)p_this;
    return p_mesh->p_index_buffer;
}

static i_texture2_t* __stdcall get_texture(const i_mesh2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    mesh2_t* p_mesh = (mesh2_t*)p_this;
    return p_mesh->p_texture;
}

void __stdcall create_mesh2_private(i_mesh2_t** pp_out_mesh)
{
    ASSERT(pp_out_mesh != NULL, "pp_out_mesh == NULL");

    mesh2_t* p_mesh = (mesh2_t*)malloc(sizeof(mesh2_t));
    if (p_mesh == NULL)
    {
        ASSERT(false, "Failed to malloc mesh");
        *pp_out_mesh = NULL;
    }

    static i_mesh2_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_vertex_buffer,
        get_index_buffer,
        get_texture
    };

    p_mesh->base.vtbl = &vtbl;
    p_mesh->ref_count = 1;

    *pp_out_mesh = (i_mesh2_t*)p_mesh;
}