//***************************************************************************
// 
// 파일: geometry.c
// 
// 설명: geometry
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/17
// 
//***************************************************************************

#include "precompiled.h"
#include "i_geometry.h"

typedef struct geometry
{
    i_geometry_t base;
    size_t ref_count;

    i_ecs_t* p_ecs;
} geometry_t;

extern void __stdcall create_ecs_instance_private(i_ecs_t** pp_out_ecs_instance);
void __stdcall create_instance(void** pp_out_instance);

static size_t __stdcall add_ref(i_geometry_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    geometry_t* p_geometry = (geometry_t*)p_this;
    return ++p_geometry->ref_count;
}

static size_t __stdcall release(i_geometry_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    geometry_t* p_geometry = (geometry_t*)p_this;
    if (--p_geometry->ref_count == 0)
    {
        SAFE_RELEASE(p_geometry->p_ecs);

        SAFE_FREE(p_this);

        return 0;
    }

    return p_geometry->ref_count;
}

static size_t __stdcall get_ref_count(const i_geometry_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const geometry_t* p_geometry = (geometry_t*)p_this;
    return p_geometry->ref_count;
}

static bool __stdcall initialize(i_geometry_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    geometry_t* p_geometry = (geometry_t*)p_this;

    create_ecs_instance_private(&p_geometry->p_ecs);
    if (p_geometry->p_ecs == NULL)
    {
        ASSERT(false, "Failed to create ecs instance");
        return false;
    }

    return true;
}

static i_ecs_t* __stdcall get_ecs(const i_geometry_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const geometry_t* p_geometry = (geometry_t*)p_this;
    i_ecs_t* p_ecs = p_geometry->p_ecs;
    p_ecs->vtbl->add_ref(p_ecs);
    return p_ecs;
}

void __stdcall create_instance(void** pp_out_instance)
{
    ASSERT(pp_out_instance != NULL, "pp_out_instance == NULL");

    static i_geometry_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        initialize,
        get_ecs
    };

    geometry_t* pa_geometry = malloc(sizeof(geometry_t));
    if (pa_geometry == NULL)
    {
        ASSERT(false, "Failed to malloc world");

        *pp_out_instance = NULL;
        return;
    }

    pa_geometry->base.vtbl = &vtbl;
    pa_geometry->ref_count = 1;

    *pp_out_instance = pa_geometry;
}