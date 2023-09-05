#ifndef DYNAMIC_VECTOR_H
#define DYNAMIC_VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <memory.h>

#include "safe99_common/assert.h"
#include "safe99_common/defines.h"

typedef struct dynamic_vector
{
    size_t element_size;
    size_t num_elements;
    size_t num_max_elements;

    char* pa_elements;
    char* p_last_element;
} dynamic_vector_t;

START_EXTERN_C

SAFE99_API bool dynamic_vector_init(dynamic_vector_t* p_vector, const size_t element_size, const size_t num_max_elements);
SAFE99_API void dynamic_vector_release(dynamic_vector_t* p_vector);
SAFE99_API void dynamic_vector_clear(dynamic_vector_t* p_vector);

SAFE99_API bool dynamic_vector_expand(dynamic_vector_t* p_vector);

SAFE99_API FORCEINLINE size_t dynamic_vector_get_num_elements(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->num_elements;
}

SAFE99_API FORCEINLINE size_t dynamic_vector_get_num_max_elements(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->num_max_elements;
}

SAFE99_API FORCEINLINE size_t dynamic_vector_get_element_size(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->element_size;
}

SAFE99_API FORCEINLINE bool dynamic_vector_push_back(dynamic_vector_t* p_vector, const void* p_element, const size_t element_size)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    ASSERT(p_element != NULL, "p_element == NULL");

    if (p_vector->element_size != element_size)
    {
        ASSERT(false, "Mismatch size");
        return false;
    }

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        dynamic_vector_expand(p_vector);
    }

    memcpy(p_vector->p_last_element, p_element, element_size);

    ++p_vector->num_elements;
    p_vector->p_last_element += element_size;

    return true;
}

SAFE99_API FORCEINLINE bool dynamic_vector_push_back_empty(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector->num_elements >= p_vector->num_max_elements)
    {
        dynamic_vector_expand(p_vector);
    }

    ++p_vector->num_elements;
    p_vector->p_last_element += p_vector->element_size;

    return true;
}

SAFE99_API FORCEINLINE bool dynamic_vector_pop_back(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector->num_elements == 0)
    {
        ASSERT(false, "Empty");
        return false;
    }

    --p_vector->num_elements;
    p_vector->p_last_element -= p_vector->element_size;

    return true;
}

SAFE99_API bool dynamic_vector_insert(dynamic_vector_t* p_vector, const void* p_element, const size_t element_size, const size_t index);
SAFE99_API bool dynamic_vector_insert_empty(dynamic_vector_t* p_vector, const size_t index);
SAFE99_API bool dynamic_vector_remove(dynamic_vector_t* p_vector, const size_t index);

SAFE99_API FORCEINLINE void* dynamic_vector_back_or_null(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector->num_elements == 0)
    {
        ASSERT(false, "Empty");
        return NULL;
    }

    return p_vector->p_last_element - p_vector->element_size;
}

SAFE99_API void* dynamic_vector_get_element_or_null(dynamic_vector_t* p_vector, const size_t index);

SAFE99_API FORCEINLINE char* dynamic_vector_get_elements_ptr_or_null(dynamic_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->pa_elements;
}

END_EXTERN_C

#endif // DYNAMIC_VECTOR_H