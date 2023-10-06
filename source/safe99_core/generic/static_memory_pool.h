//***************************************************************************
// 
// 파일: static_memory_pool.h
// 
// 설명: 고정 사이즈 메모리 풀
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/02
// 
//***************************************************************************

#ifndef STATIC_MEMORY_POOL_H
#define STATIC_MEMORY_POOL_H

#include "safe99_common/defines.h"
#include "safe99_common/types.h"

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
SAFE99_API bool static_memory_pool_init(static_memory_pool_t* p_pool, const size_t element_size, const size_t num_elements_per_block, const size_t num_max_blocks);

SAFE99_API void static_memory_pool_release(static_memory_pool_t* p_pool);

SAFE99_API void* static_memory_pool_alloc_or_null(static_memory_pool_t* p_pool);

SAFE99_API void static_memory_pool_dealloc(static_memory_pool_t* p_pool, void* p_element_or_null);

SAFE99_API void static_memory_pool_reset(static_memory_pool_t* p_pool);

END_EXTERN_C

#endif // STATIC_MEMORY_POOL_H