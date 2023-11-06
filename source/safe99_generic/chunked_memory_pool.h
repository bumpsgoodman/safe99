//***************************************************************************
// 
// 파일: chunked_memory_pool.h
// 
// 설명: 가변 사이즈 메모리 풀
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/05
// 
//***************************************************************************

#ifndef CHUNKED_MEMORY_POOL_H
#define CHUNKED_MEMORY_POOL_H

#include "safe99_common/defines.h"
#include "list.h"

typedef struct chunked_memory_pool
{
    list_node_t* p_head;
    list_node_t* p_tail;

    size_t element_size;
    size_t num_elements_per_chunk;
    size_t element_size_with_header;
    size_t num_alloc_elements;
} chunked_memory_pool_t;

START_EXTERN_C

SAFE99_API bool __stdcall chunked_memory_pool_initialize(chunked_memory_pool_t* p_pool, const size_t element_size, const size_t num_elements_per_chunk);
SAFE99_API void __stdcall chunked_memory_pool_release(chunked_memory_pool_t* p_pool);
SAFE99_API void __stdcall chunked_memory_pool_reset(chunked_memory_pool_t* p_pool);

SAFE99_API void* __stdcall chunked_memory_pool_alloc_or_null(chunked_memory_pool_t* p_pool);
SAFE99_API void __stdcall chunked_memory_pool_dealloc(chunked_memory_pool_t* p_pool, void* p_element_or_null);

SAFE99_API size_t __stdcall chunked_memory_pool_get_element_size(const chunked_memory_pool_t* p_pool);
SAFE99_API size_t __stdcall chunked_memory_pool_get_num_elements_per_chunk(const chunked_memory_pool_t* p_pool);
SAFE99_API size_t __stdcall chunked_memory_pool_get_num_alloc_elements(const chunked_memory_pool_t* p_pool);

END_EXTERN_C

#endif // CHUNKED_MEMORY_POOL_H