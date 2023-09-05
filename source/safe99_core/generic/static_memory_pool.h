#ifndef STATIC_MEMORY_POOL_H
#define STATIC_MEMORY_POOL_H

#include <stdbool.h>
#include <stddef.h>

#include "safe99_common/defines.h"

typedef struct static_memory_pool
{
    size_t element_size;
    size_t num_elements_per_block;
    size_t num_max_blocks;
    size_t num_cur_blocks;
    size_t element_size_with_header;

    char** ppa_blocks;
    char*** pppa_index_tables;
    char*** pppa_index_table_ptrs;
} static_memory_pool_t;

START_EXTERN_C

// element_size는 0보다 커야 함
// num_elements_per_block은 0보다 커야 함
// num_blocks는 0보다 커야 함
// 
// 이미 초기화한 메모리 풀을 다시 초기화하지 말 것
// 해야 한다면 static_memory_pool_release() 함수 호출 이후 재호출
SAFE99_API bool static_memory_pool_init(static_memory_pool_t* p_pool, const size_t element_size, const size_t num_elements_per_block, const size_t num_max_blocks);

SAFE99_API void static_memory_pool_release(static_memory_pool_t* p_pool);

SAFE99_API void* static_memory_pool_alloc_or_null(static_memory_pool_t* p_pool);

SAFE99_API void static_memory_pool_dealloc(static_memory_pool_t* p_pool, void* p_element_or_null);

SAFE99_API void static_memory_pool_reset(static_memory_pool_t* p_pool);

END_EXTERN_C

#endif // STATIC_MEMORY_POOL_H