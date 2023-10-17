//***************************************************************************
// 
// 파일: fixed_vector.c
// 
// 설명: 고정 사이즈 벡터
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/10
// 
//***************************************************************************

#include "precompiled.h"

typedef struct fixed_vector
{
    i_fixed_vector_t base;

    size_t element_size;
    size_t num_elements;
    size_t num_max_elements;

    char* pa_elements;
    char* p_last_element;
} fixed_vector_t;

static bool __stdcall initialize(i_fixed_vector_t* p_this, const size_t element_size, const size_t num_max_elements)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(element_size > 0, "element_size == 0");
    ASSERT(num_max_elements > 0, "num_max_elements == 0");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    p_vector->element_size = element_size;
    p_vector->num_max_elements = num_max_elements;
    p_vector->num_elements = 0;

    p_vector->pa_elements = (char*)malloc(element_size * num_max_elements);
    if (p_vector->pa_elements == NULL)
    {
        ASSERT(false, "Failed to malloc elements");
        memset(p_vector, 0, sizeof(fixed_vector_t));
        return false;
    }

    p_vector->p_last_element = p_vector->pa_elements;

    return true;
}

static void __stdcall release(i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector == NULL)
    {
        return;
    }

    SAFE_FREE(p_vector->pa_elements);
    memset(p_vector, 0, sizeof(fixed_vector_t));
}

static void __stdcall clear(i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;
    p_vector->num_elements = 0;
    p_vector->p_last_element = p_vector->pa_elements;
}

static bool __stdcall push_back(i_fixed_vector_t* p_this, const void* p_element, const size_t element_size)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector->element_size != element_size)
    {
        ASSERT(false, "Mismatch size");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        ASSERT(false, "Saturate");
        return false;
    }

    memcpy(p_vector->p_last_element, p_element, element_size);
    ++p_vector->num_elements;

    p_vector->p_last_element += element_size;

    return true;
}

static bool __stdcall push_back_empty(i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        ASSERT(false, "Full");
        return false;
    }

    ++p_vector->num_elements;
    p_vector->p_last_element += p_vector->element_size;

    return true;
}

static bool __stdcall pop_back(i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector->num_elements == 0)
    {
        ASSERT(false, "Empty");
        return false;
    }

    --p_vector->num_elements;
    p_vector->p_last_element -= p_vector->element_size;

    return true;
}

static bool __stdcall insert(i_fixed_vector_t* p_this, const void* p_element, const size_t element_size, const size_t index)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector->element_size != element_size)
    {
        ASSERT(false, "Mismatch size");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements
        || index > p_vector->num_elements)
    {
        ASSERT(false, "Saturate or invalid index");
        return false;
    }

    // element 이동
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + element_size * (index + 1);
    char* src = dst - element_size;
    memmove(dst, src, element_size * len);

    memcpy(src, p_element, element_size);

    ++p_vector->num_elements;
    p_vector->p_last_element += element_size;

    return true;
}

static bool __stdcall insert_empty(i_fixed_vector_t* p_this, const size_t index)
{
    ASSERT(p_this != NULL, "p_vector == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (index > p_vector->num_elements)
    {
        ASSERT(false, "invalid index");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        ASSERT(false, "Full");
        return false;
    }

    // element 이동
    const size_t element_size = p_vector->element_size;
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + element_size * (index + 1);
    char* src = dst - element_size;
    memmove(dst, src, element_size * len);

    ++p_vector->num_elements;
    p_vector->p_last_element += element_size;

    return true;
}

static bool __stdcall remove(i_fixed_vector_t* p_this, const size_t index)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector->num_elements == 0
        || index >= p_vector->num_elements)
    {
        ASSERT(false, "Empty or invalid index");
        return false;
    }

    // element 이동
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + p_vector->element_size * index;
    const char* src = dst + p_vector->element_size;
    memmove(dst, src, p_vector->element_size * len);

    --p_vector->num_elements;
    p_vector->p_last_element -= p_vector->element_size;

    return true;
}

static size_t __stdcall get_element_size(const i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const fixed_vector_t* p_vector = (fixed_vector_t*)p_this;
    return p_vector->element_size;
}

static size_t __stdcall get_num_max_elements(const i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const fixed_vector_t* p_vector = (fixed_vector_t*)p_this;
    return p_vector->num_max_elements;
}

static size_t __stdcall get_num_elements(const i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;
    return p_vector->num_elements;
}

static void* __stdcall get_back_or_null(const i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector->num_elements == 0)
    {
        ASSERT(false, "Empty");
        return NULL;
    }

    return p_vector->p_last_element;
}

static void* __stdcall get_element_or_null(const i_fixed_vector_t* p_this, const size_t index)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const fixed_vector_t* p_vector = (fixed_vector_t*)p_this;

    if (p_vector->num_elements == 0
        || index >= p_vector->num_elements)
    {
        ASSERT(false, "Empty or invalid index");
        return NULL;
    }

    return p_vector->pa_elements + p_vector->element_size * index;
}

static char* __stdcall get_elements_ptr_or_null(const i_fixed_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    fixed_vector_t* p_vector = (fixed_vector_t*)p_this;
    return p_vector->pa_elements;
}

static void __stdcall create_fixed_vector(i_fixed_vector_t** pp_out_fixed_vector)
{
    ASSERT(pp_out_fixed_vector != NULL, "pp_out_fixed_vector == NULL");

    static i_fixed_vector_vtbl_t vtbl =
    {
        initialize,
        release,
        clear,

        push_back,
        push_back_empty,
        pop_back,

        insert,
        insert_empty,
        remove,

        get_element_size,
        get_num_max_elements,
        get_num_elements,

        get_back_or_null,
        get_element_or_null,
        get_elements_ptr_or_null
    };

    fixed_vector_t* pa_vector = (fixed_vector_t*)malloc(sizeof(fixed_vector_t));
    if (pa_vector == NULL)
    {
        ASSERT(false, "Failed to malloc fixed vector");
        *pp_out_fixed_vector = NULL;
    }

    pa_vector->base.vtbl = &vtbl;

    *pp_out_fixed_vector = (i_fixed_vector_t*)pa_vector;
}

static void __stdcall destroy_fixed_vector(i_fixed_vector_t* p_fixed_vector)
{
    ASSERT(p_fixed_vector != NULL, "p_fixed_vector == NULL");

    release(p_fixed_vector);
    SAFE_FREE(p_fixed_vector);
}