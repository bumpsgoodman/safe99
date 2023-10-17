//***************************************************************************
// 
// 파일: dynamic_vector.c
// 
// 설명: 가변 사이즈 벡터
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/10
// 
//***************************************************************************

#include "precompiled.h"

typedef struct dynamic_vector
{
    i_dynamic_vector_t base;

    size_t element_size;
    size_t num_elements;
    size_t num_max_elements;

    char* pa_elements;
    char* p_last_element;
} dynamic_vector_t;

static bool __stdcall expand(dynamic_vector_t* p_vector);

static bool __stdcall initialize(i_dynamic_vector_t* p_this, const size_t element_size, const size_t num_max_elements)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(element_size > 0, "element_size == 0");
    ASSERT(num_max_elements > 0, "num_max_elements == 0");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    p_vector->element_size = element_size;
    p_vector->num_max_elements = num_max_elements;
    p_vector->num_elements = 0;

    p_vector->pa_elements = (char*)malloc(element_size * num_max_elements);
    if (p_vector->pa_elements == NULL)
    {
        ASSERT(false, "Failed to malloc elements");
        memset(p_vector, 0, sizeof(dynamic_vector_t));
        return false;
    }

    p_vector->p_last_element = p_vector->pa_elements;

    return true;
}

static void __stdcall release(i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector == NULL)
    {
        return;
    }

    SAFE_FREE(p_vector->pa_elements);
    memset(p_vector, 0, sizeof(dynamic_vector_t));
}

static void __stdcall clear(i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    p_vector->num_elements = 0;
    p_vector->p_last_element = p_vector->pa_elements;
}

static bool __stdcall push_back(i_dynamic_vector_t* p_this, const void* p_element, const size_t element_size)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector->element_size != element_size)
    {
        ASSERT(false, "Mismatch size");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        expand(p_vector);
    }

    memcpy(p_vector->p_last_element, p_element, element_size);

    ++p_vector->num_elements;
    p_vector->p_last_element += element_size;

    return true;
}

static bool __stdcall push_back_empty(i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        expand(p_vector);
    }

    ++p_vector->num_elements;
    p_vector->p_last_element += p_vector->element_size;

    return true;
}

static bool __stdcall pop_back(i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector->num_elements == 0)
    {
        ASSERT(false, "Empty");
        return false;
    }

    --p_vector->num_elements;
    p_vector->p_last_element -= p_vector->element_size;

    return true;
}

static bool __stdcall insert(i_dynamic_vector_t* p_this, const void* p_element, const size_t element_size, const size_t index)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector->element_size != element_size)
    {
        ASSERT(false, "Mismatch size");
        return false;
    }

    if (index > p_vector->num_elements)
    {
        ASSERT(false, "invalid index");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        expand(p_vector);
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

static bool __stdcall insert_empty(i_dynamic_vector_t* p_this, const size_t index)
{
    ASSERT(p_this != NULL, "p_vector == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (index > p_vector->num_elements)
    {
        ASSERT(false, "invalid index");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        expand(p_vector);
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

static bool __stdcall remove(i_dynamic_vector_t* p_this, const size_t index)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector->num_elements == 0
        || index >= p_vector->num_elements)
    {
        ASSERT(false, "Empty or invalid index");
        return false;
    }

    // element 이동
    const size_t element_size = p_vector->element_size;
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + element_size * index;
    const char* src = dst + element_size;
    memmove(dst, src, element_size * len);

    --p_vector->num_elements;
    p_vector->p_last_element -= element_size;

    return true;
}

static size_t __stdcall get_element_size(const i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;
    return p_vector->element_size;
}

static size_t __stdcall get_num_max_elements(const i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;
    return p_vector->num_max_elements;
}

static size_t __stdcall get_num_elements(const i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;
    return p_vector->num_elements;
}

static void* __stdcall get_back_or_null(const i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector->num_elements == 0)
    {
        ASSERT(false, "Empty");
        return NULL;
    }

    return p_vector->p_last_element - p_vector->element_size;
}

static void* __stdcall get_element_or_null(const i_dynamic_vector_t* p_this, const size_t index)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;

    if (p_vector->num_elements == 0
        || index >= p_vector->num_elements)
    {
        ASSERT(false, "Empty or invalid index");
        return NULL;
    }

    return p_vector->pa_elements + p_vector->element_size * index;
}

static char* __stdcall get_elements_ptr_or_null(const i_dynamic_vector_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const dynamic_vector_t* p_vector = (dynamic_vector_t*)p_this;
    return p_vector->pa_elements;
}

static void __stdcall create_dynamic_vector(i_dynamic_vector_t** pp_out_dynamic_vector)
{
    ASSERT(pp_out_dynamic_vector != NULL, "pp_out_dynamic_vector == NULL");

    static i_dynamic_vector_vtbl_t vtbl =
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

    dynamic_vector_t* pa_vector = (dynamic_vector_t*)malloc(sizeof(dynamic_vector_t));
    if (pa_vector == NULL)
    {
        ASSERT(false, "Failed to malloc dynamic vector");
        *pp_out_dynamic_vector = NULL;
    }

    pa_vector->base.vtbl = &vtbl;

    *pp_out_dynamic_vector = (i_dynamic_vector_t*)pa_vector;
}

static void __stdcall destroy_dynamic_vector(i_dynamic_vector_t* p_dynamic_vector)
{
    ASSERT(p_dynamic_vector != NULL, "p_dynamic_vector == NULL");

    release(p_dynamic_vector);
    SAFE_FREE(p_dynamic_vector);
}

static bool __stdcall expand(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    char* pa_new_space = (char*)malloc(p_vector->element_size * p_vector->num_max_elements * 2);
    if (pa_new_space == NULL)
    {
        ASSERT(false, "Failed malloc new space");
        return false;
    }

    memcpy(pa_new_space, p_vector->pa_elements, p_vector->element_size * p_vector->num_max_elements);

    free(p_vector->pa_elements);
    p_vector->pa_elements = pa_new_space;
    p_vector->num_max_elements *= 2;
    p_vector->p_last_element = pa_new_space + p_vector->element_size * p_vector->num_elements;

    return true;
}