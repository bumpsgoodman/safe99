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

        SAFE_FREE(p_this);

        return 0;
    }

    return p_buffer->ref_count;
}

static size_t __stdcall get_ref_count(const i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;
    return p_buffer->ref_count;
}

static const uint_t* __stdcall get_indices(const i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;
    return p_buffer->pa_indices;
}

static size_t __stdcall get_num_indices(const i_index_buffer2_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    index_buffer2_t* p_buffer = (index_buffer2_t*)p_this;
    return p_buffer->num_indices;
}

void __stdcall create_index_buffer2_private(i_index_buffer2_t** pp_out_index_buffer)
{
    ASSERT(pp_out_index_buffer != NULL, "pp_out_index_buffer == NULL");

    index_buffer2_t* p_index_buffer = (index_buffer2_t*)malloc(sizeof(index_buffer2_t));
    if (p_index_buffer == NULL)
    {
        ASSERT(false, "Failed to malloc index buffer");
        *pp_out_index_buffer = NULL;
    }

    static i_index_buffer2_vtbl_t vtbl =
    {
        add_ref,
        release,
        get_ref_count,

        get_indices,
        get_num_indices
    };

    p_index_buffer->base.vtbl = &vtbl;
    p_index_buffer->ref_count = 1;

    *pp_out_index_buffer = (i_index_buffer2_t*)p_index_buffer;
}