#include <memory.h>

#include "safe99_common/defines.h"

typedef struct index_buffer2
{
    i_index_buffer2_t base;
    size_t ref_count;

    uint_t* pa_indices;
    size_t num_indices;
} index_buffer2_t;

static size_t __stdcall add_ref(i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;
    return ++p_buffer->ref_count;
}

static size_t __stdcall release(i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;
    if (--p_buffer->ref_count == 0)
    {
        SAFE_FREE(p_buffer->pa_indices);

        return 0;
    }

    return p_buffer->ref_count;
}

static size_t __stdcall get_ref_count(i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;
    return p_buffer->ref_count;
}

static const uint_t* __stdcall get_indices(i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;
    return p_buffer->pa_indices;
}

static size_t __stdcall get_num_indices(i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;

    return p_buffer->num_indices;
}

void  __stdcall initialize_index_buffer2_private(index_buffer2_t* p_buffer)
{
    ASSERT(p_buffer != NULL, "p_buffer == NULL");

    static i_index_buffer2_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_indices,
        get_num_indices
    };

    memset(p_buffer, 0, sizeof(index_buffer2_t));

    p_buffer->base.vtbl = &vtbl;
    p_buffer->ref_count = 1;
}