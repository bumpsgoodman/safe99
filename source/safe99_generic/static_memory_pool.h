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

typedef struct static_memory_pool
{
    char** ppa_blocks;
    char*** pppa_index_tables;
    char*** pppa_index_table_ptrs;

    size_t element_size;
    size_t num_elements_per_block;
    size_t num_max_blocks;
    size_t num_cur_blocks;
    size_t element_size_with_header;
    size_t num_alloc_elements;
} static_memory_pool_t;

START_EXTERN_C

SAFE99_API bool __stdcall static_memory_pool_initialize(static_memory_pool_t* p_pool, const size_t element_size, const size_t num_elements_per_block, const size_t num_max_blocks);
SAFE99_API void __stdcall static_memory_pool_release(static_memory_pool_t* p_pool);
SAFE99_API void __stdcall static_memory_pool_reset(static_memory_pool_t* p_pool);

SAFE99_API void* __stdcall static_memory_pool_alloc_or_null(static_memory_pool_t* p_pool);
SAFE99_API void __stdcall static_memory_pool_dealloc(static_memory_pool_t* p_pool, void* p_element_or_null);

SAFE99_API size_t __stdcall static_memory_pool_get_element_size(const static_memory_pool_t* p_pool);
SAFE99_API size_t __stdcall static_memory_pool_get_num_elements_per_block(const static_memory_pool_t* p_pool);
SAFE99_API size_t __stdcall static_memory_pool_get_num_alloc_elements(const static_memory_pool_t* p_pool);

END_EXTERN_C

#endif // STATIC_MEMORY_POOL_H