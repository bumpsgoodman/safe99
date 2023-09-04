#include "precompiled.h"

bool dynamic_vector_init(dynamic_vector_t* p_vector, const size_t element_size, const size_t num_max_elements)
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

    p_vector->p_last_element = p_vector->pa_elements;

    return true;
}

void dynamic_vector_release(dynamic_vector_t* p_vector)
{
    if (p_vector == NULL)
    {
        return;
    }

    SAFE_FREE(p_vector->pa_elements);
    memset(p_vector, 0, sizeof(dynamic_vector_t));
}

void dynamic_vector_clear(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    p_vector->num_elements = 0;
    p_vector->p_last_element = p_vector->pa_elements;
}

bool dynamic_vector_insert(dynamic_vector_t* p_vector, const void* p_element, const size_t element_size, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

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
        dynamic_vector_expand(p_vector);
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

bool dynamic_vector_insert_empty(dynamic_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (index > p_vector->num_elements)
    {
        ASSERT(false, "invalid index");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        dynamic_vector_expand(p_vector);
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

bool dynamic_vector_remove(dynamic_vector_t* p_vector, const size_t index)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

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

void* dynamic_vector_get_element_or_null(dynamic_vector_t* p_vector, const size_t index)
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

bool dynamic_vector_expand(dynamic_vector_t* p_vector)
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