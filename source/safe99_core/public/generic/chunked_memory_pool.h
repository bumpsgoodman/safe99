#ifndef CHUNKED_MEMORY_POOL_H
#define CHUNKED_MEMORY_POOL_H

#include <stdbool.h>
#include <stddef.h>

#include "safe99_common/defines.h"

typedef struct list_node list_node_t;

typedef struct chunked_memory_pool
{
    size_t element_size;
    size_t num_elements_per_chunk;
    size_t element_size_with_header;

    list_node_t* p_head;
    list_node_t* p_tail;
} chunked_memory_pool_t;

START_EXTERN_C

// element_size는 0보다 커야 함
// num_elements_per_chunk은 0보다 커야 함
// 
// 이미 초기화한 메모리 풀을 다시 초기화하지 말 것
// 해야 한다면 chunked_memory_pool_release() 함수 호출 이후 재호출
SAFE99_API bool chunked_memory_pool_init(chunked_memory_pool_t* p_pool, const size_t element_size, const size_t num_elements_per_chunk);

SAFE99_API void chunked_memory_pool_release(chunked_memory_pool_t* p_pool);

SAFE99_API void* chunked_memory_pool_alloc_or_null(chunked_memory_pool_t* p_pool);

SAFE99_API void chunked_memory_pool_dealloc(chunked_memory_pool_t* p_pool, void* p_element_or_null);

SAFE99_API void chunked_memory_pool_reset(chunked_memory_pool_t* p_pool);

END_EXTERN_C

#endif // CHUNKED_MEMORY_POOL_H