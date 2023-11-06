//***************************************************************************
// 
// 파일: dynamic_vector.h
// 
// 설명: 가변 사이즈 벡터
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/10
// 
//***************************************************************************

#ifndef DYNAMIC_VECTOR_H
#define DYNAMIC_VECTOR_H

#include <memory.h>

#include "safe99_common/defines.h"

typedef struct dynamic_vector
{
    char* pa_elements;

    size_t element_size;
    size_t num_elements;
    size_t num_max_elements;
} dynamic_vector_t;

START_EXTERN_C

static bool __stdcall expand(dynamic_vector_t* p_vector);

FORCEINLINE bool __stdcall dynamic_vector_initialize(dynamic_vector_t* p_vector, const size_t element_size, const size_t num_max_elements)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(element_size > 0, "element_size == 0");
    ASSERT(num_max_elements > 0, "num_max_elements == 0");

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

    return true;
}

FORCEINLINE void __stdcall dynamic_vector_release(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector == NULL)
    {
        return;
    }

    SAFE_FREE(p_vector->pa_elements);
    memset(p_vector, 0, sizeof(dynamic_vector_t));
}

FORCEINLINE void __stdcall dynamic_vector_clear(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    p_vector->num_elements = 0;
}

FORCEINLINE void __stdcall dynamic_vector_push_back(dynamic_vector_t* p_vector, const void* p_element, const size_t element_size)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");
    ASSERT(p_vector->element_size == element_size, "mismatch size");

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        expand(p_vector);
    }

    memcpy(p_vector->pa_elements + p_vector->element_size * p_vector->num_elements, p_element, element_size);
    ++p_vector->num_elements;
}

FORCEINLINE void __stdcall dynamic_vector_push_back_empty(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        expand(p_vector);
    }

    ++p_vector->num_elements;
}

FORCEINLINE void __stdcall dynamic_vector_pop_back(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements > 0, "empty");

    --p_vector->num_elements;
}

FORCEINLINE void __stdcall dynamic_vector_insert(dynamic_vector_t* p_vector, const void* p_element, const size_t element_size, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");
    ASSERT(p_vector->element_size == element_size, "mismatch size");
    ASSERT(index <= p_vector->num_elements, "invalid index");

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
}

FORCEINLINE void __stdcall dynamic_vector_insert_empty(dynamic_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(index <= p_vector->num_elements, "invalid index");

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
}

FORCEINLINE void __stdcall dynamic_vector_remove(dynamic_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements > 0, "empty");
    ASSERT(index <= p_vector->num_elements, "invalid index");

    // element 이동
    const size_t element_size = p_vector->element_size;
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + element_size * index;
    const char* src = dst + element_size;
    memmove(dst, src, element_size * len);

    --p_vector->num_elements;
}

FORCEINLINE size_t __stdcall dynamic_vector_get_element_size(const dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->element_size;
}

FORCEINLINE size_t __stdcall dynamic_vector_get_num_max_elements(const dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->num_max_elements;
}

FORCEINLINE size_t __stdcall dynamic_vector_get_num_elements(const dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->num_elements;
}

FORCEINLINE void* __stdcall dynamic_vector_get_back_or_null(const dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements > 0, "empty");
    return p_vector->pa_elements + p_vector->element_size * (p_vector->num_elements - 1);
}

FORCEINLINE void* __stdcall dynamic_vector_get_element_or_null(const dynamic_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements != 0, "empty");
    ASSERT(index < p_vector->num_elements, "invliad index");

    return p_vector->pa_elements + p_vector->element_size * index;
}

FORCEINLINE char* __stdcall dynamic_vector_get_elements_ptr_or_null(const dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->pa_elements;
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

    return true;
}

END_EXTERN_C

#endif // DYNAMIC_VECTOR_H