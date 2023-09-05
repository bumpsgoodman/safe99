#ifndef FIXED_VECTOR_H
#define FIXED_VECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <memory.h>

#include "safe99_common/assert.h"
#include "safe99_common/defines.h"

typedef struct fixed_vector
{
    size_t element_size;
    size_t num_elements;
    size_t num_max_elements;

    char* pa_elements;
    char* p_last_element;
} fixed_vector_t;

START_EXTERN_C

SAFE99_API bool fixed_vector_init(fixed_vector_t* p_vector, const size_t element_size, const size_t num_max_elements);
SAFE99_API void fixed_vector_release(fixed_vector_t* p_vector);
SAFE99_API void fixed_vector_clear(fixed_vector_t* p_vector);

SAFE99_API FORCEINLINE size_t fixed_vector_get_num_elements(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->num_elements;
}

SAFE99_API FORCEINLINE bool fixed_vector_push_back(fixed_vector_t* p_vector, const void* p_element, const size_t element_size)
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
        ASSERT(false, "Saturate");
        return false;
    }

    memcpy(p_vector->p_last_element, p_element, element_size);
    ++p_vector->num_elements;

    p_vector->p_last_element += element_size;

    return true;
}

SAFE99_API FORCEINLINE bool fixed_vector_pop_back(fixed_vector_t* p_vector)
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

SAFE99_API bool fixed_vector_insert(fixed_vector_t* p_vector, const void* p_element, const size_t element_size, const size_t index);
SAFE99_API bool fixed_vector_remove(fixed_vector_t* p_vector, const size_t index);

SAFE99_API FORCEINLINE void* fixed_vector_back_or_null(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");

    if (p_vector->num_elements == 0)
    {
        ASSERT(false, "Empty");
        return NULL;
    }

    return p_vector->p_last_element;
}

SAFE99_API void* fixed_vector_get_element_or_null(fixed_vector_t* p_vector, const size_t index);



SAFE99_API FORCEINLINE char* fixed_vector_get_elements_ptr_or_null(fixed_vector_t* p_vector)
{
    ASSERT(p_vector != NULL, "p_vector == NULL");
    return p_vector->pa_elements;
}

END_EXTERN_C

#endif // FIXED_VECTOR_H