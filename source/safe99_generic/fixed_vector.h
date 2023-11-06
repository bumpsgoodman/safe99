//***************************************************************************
// 
// 파일: fixed_vector.h
// 
// 설명: 고정 사이즈 벡터
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/09
// 
//***************************************************************************

#ifndef FIXED_VECTOR_H
#define FIXED_VECTOR_H

#include <memory.h>

#include "safe99_common/defines.h"

typedef struct fixed_vector
{
    char* pa_elements;

    size_t element_size;
    size_t num_elements;
    size_t num_max_elements;
} fixed_vector_t;

START_EXTERN_C

FORCEINLINE bool __stdcall fixed_vector_initialize(fixed_vector_t* p_vector, const size_t element_size, const size_t num_max_elements)
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
        memset(p_vector, 0, sizeof(fixed_vector_t));
        return false;
    }

    return true;
}

FORCEINLINE void __stdcall fixed_vector_release(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector == NULL)
    {
        return;
    }

    SAFE_FREE(p_vector->pa_elements);
    memset(p_vector, 0, sizeof(fixed_vector_t));
}

FORCEINLINE void __stdcall fixed_vector_clear(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    p_vector->num_elements = 0;
}

FORCEINLINE void __stdcall fixed_vector_push_back(fixed_vector_t* p_vector, const void* p_element, const size_t element_size)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

    ASSERT(p_vector->element_size == element_size, "mismatch size");
    ASSERT(p_vector->num_elements < p_vector->num_max_elements, "saturate");

    memcpy(p_vector->pa_elements + p_vector->element_size * p_vector->num_elements, p_element, element_size);
    ++p_vector->num_elements;
}

FORCEINLINE void __stdcall fixed_vector_push_back_empty(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements < p_vector->num_max_elements, "saturate");

    ++p_vector->num_elements;
}

FORCEINLINE void __stdcall fixed_vector_pop_back(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements != 0, "Empty");

    --p_vector->num_elements;
}

FORCEINLINE void __stdcall fixed_vector_insert(fixed_vector_t* p_vector, const void* p_element, const size_t element_size, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");
    ASSERT(p_vector->element_size == element_size, "mismatch size");
    ASSERT(p_vector->num_elements < p_vector->num_max_elements, "saturate");
    ASSERT(index <= p_vector->num_elements, "invalid index");

    // element 이동
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + element_size * (index + 1);
    char* src = dst - element_size;
    memmove(dst, src, element_size * len);

    memcpy(src, p_element, element_size);

    ++p_vector->num_elements;
}

FORCEINLINE void __stdcall fixed_vector_insert_empty(fixed_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements < p_vector->num_max_elements, "saturate");
    ASSERT(index <= p_vector->num_elements, "invalid index");

    // element 이동
    const size_t element_size = p_vector->element_size;
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + element_size * (index + 1);
    char* src = dst - element_size;
    memmove(dst, src, element_size * len);

    ++p_vector->num_elements;
}

FORCEINLINE void __stdcall fixed_vector_remove(fixed_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements < p_vector->num_max_elements, "saturate");
    ASSERT(index <= p_vector->num_elements, "invalid index");

    // element 이동
    const size_t len = p_vector->num_elements - index;
    char* dst = p_vector->pa_elements + p_vector->element_size * index;
    const char* src = dst + p_vector->element_size;
    memmove(dst, src, p_vector->element_size * len);

    --p_vector->num_elements;
}

FORCEINLINE size_t __stdcall fixed_vector_get_element_size(const fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->element_size;
}

FORCEINLINE size_t __stdcall fixed_vector_get_num_max_elements(const fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->num_max_elements;
}

FORCEINLINE size_t __stdcall fixed_vector_get_num_elements(const fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->num_elements;
}

FORCEINLINE void* __stdcall fixed_vector_get_back_or_null(const fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements != 0, "empty");

    return p_vector->pa_elements + p_vector->element_size * (p_vector->num_elements - 1);
}

FORCEINLINE void* __stdcall fixed_vector_get_element_or_null(const fixed_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_vector->num_elements != 0, "empty");
    ASSERT(index < p_vector->num_elements, "invliad index");

    return p_vector->pa_elements + p_vector->element_size * index;
}

FORCEINLINE char* __stdcall fixed_vector_get_elements_ptr_or_null(const fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->pa_elements;
}

END_EXTERN_C

#endif // FIXED_VECTOR_H