//***************************************************************************
// 
// 파일: texture.c
// 
// 설명: 텍스쳐
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/15
// 
//***************************************************************************

#include "precompiled.h"

#include "i_file_system.h"

typedef struct texture
{
    i_texture_t base;
    size_t ref_count;

    size_t width;
    size_t height;
    char* pa_bitmap;
} texture_t;

static size_t __stdcall add_ref(i_texture_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    texture_t* p_texture = (texture_t*)p_this;
    return ++p_texture->ref_count;
}

static size_t __stdcall release(i_texture_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    texture_t* p_texture = (texture_t*)p_this;
    if (--p_texture->ref_count == 0)
    {
        SAFE_FREE(p_texture->pa_bitmap);
        SAFE_FREE(p_this);

        return 0;
    }

    return p_texture->ref_count;
}

static size_t __stdcall get_ref_count(const i_texture_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const texture_t* p_texture = (texture_t*)p_this;
    return p_texture->ref_count;
}

static size_t __stdcall get_width(const i_texture_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const texture_t* p_texture = (texture_t*)p_this;
    return p_texture->width;
}

static size_t __stdcall get_height(const i_texture_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const texture_t* p_texture = (texture_t*)p_this;
    return p_texture->height;
}

static const char* __stdcall get_bitmap(const i_texture_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const texture_t* p_texture = (texture_t*)p_this;
    return p_texture->pa_bitmap;
}

bool create_texture_private(i_texture_t** pp_out_texture)
{
    ASSERT(pp_out_texture != NULL, "pp_out_texture == NULL");

    static i_texture_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_width,
        get_height,
        get_bitmap
    };

    texture_t* pa_texture = (texture_t*)malloc(sizeof(texture_t));
    if (pa_texture == NULL)
    {
        ASSERT(false, "Failed to malloc texture");
        *pp_out_texture = NULL;
        return false;
    }
    
    pa_texture->base.vtbl = &vtbl;
    pa_texture->ref_count = 1;

    *pp_out_texture = (i_texture_t*)pa_texture;

    return true;
}

bool initialize_texture_private(i_texture_t* p_this, const size_t width, const size_t height, char* p_bitmap)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_bitmap != NULL, "p_bitmap == NULL");

    texture_t* p_texture = (texture_t*)p_this;

    p_texture->width = width;
    p_texture->height = height;
    p_texture->pa_bitmap = p_bitmap;

    return true;
}