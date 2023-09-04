#include "precompiled.h"

bool fixed_vector_init(fixed_vector_t* p_vector, const size_t element_size, const size_t num_max_elements)
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

    p_vector->p_last_element = p_vector->pa_elements;

    return true;
}

void fixed_vector_release(fixed_vector_t* p_vector)
{
    if (p_vector == NULL)
    {
        return;
    }

    SAFE_FREE(p_vector->pa_elements);
    memset(p_vector, 0, sizeof(fixed_vector_t));
}

void fixed_vector_clear(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    p_vector->num_elements = 0;
    p_vector->p_last_element = p_vector->pa_elements;
}

bool fixed_vector_insert(fixed_vector_t* p_vector, const void* p_element, const size_t element_size, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

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

bool fixed_vector_remove(fixed_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

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

void* fixed_vector_get_element_or_null(fixed_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector->num_elements == 0
        || index >= p_vector->num_elements)
    {
        ASSERT(false, "Empty or invalid index");
        return NULL;
    }

    return p_vector->pa_elements + p_vector->element_size * index;
}