//***************************************************************************
// 
// 파일: chunked_memory_pool.c
// 
// 설명: 타이머
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/07
// 
//***************************************************************************

#include "precompiled.h"

typedef struct list_node node_t;

typedef struct chunked_memory_pool
{
    i_chunked_memory_pool_t base;

    size_t element_size;
    size_t num_elements_per_chunk;
    size_t element_size_with_header;
    size_t num_alloc_elements;

    list_node_t* p_head;
    list_node_t* p_tail;
} chunked_memory_pool_t;

typedef struct chunk
{
    char* pa_elements;
    char** ppa_index_table;
    char** pp_index_table_ptr;
} chunk_t;

typedef enum state
{
    STATE_ALLOC = 95,
    STATE_DEALLOC = 85,
} state_t;

typedef struct header
{
    list_node_t* p_chunk_node;
    state_t state;
} header_t;

static bool create_new_chunk(chunked_memory_pool_t* p_pool);

static bool __stdcall initialize(i_chunked_memory_pool_t* p_this, const size_t element_size, const size_t num_elements_per_chunk)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(element_size > 0, "element_size == 0");
    ASSERT(num_elements_per_chunk > 0, "num_elements_per_chunk == 0");

    chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;

    p_pool->element_size = element_size;
    p_pool->num_elements_per_chunk = num_elements_per_chunk;
    p_pool->element_size_with_header = sizeof(header_t) + element_size;
    p_pool->num_alloc_elements = 0;

    p_pool->p_head = NULL;
    p_pool->p_tail = NULL;

    if (!create_new_chunk(p_pool))
    {
        goto failed_create_new_chunk;
    }

    return true;

failed_create_new_chunk:
    memset(p_pool, 0, sizeof(chunked_memory_pool_t));
    return false;
}

static void __stdcall release(i_chunked_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;

    node_t* p_node = p_pool->p_head;
    while (p_node != NULL)
    {
        node_t* p_temp = p_node;

        chunk_t* p_chunk = p_node->p_element;
        SAFE_FREE(p_chunk->pa_elements);
        SAFE_FREE(p_chunk->ppa_index_table);
        SAFE_FREE(p_chunk);

        p_node = p_node->p_next;
        SAFE_FREE(p_temp);
    }

    memset(p_pool, 0, sizeof(chunked_memory_pool_t));
}

static void __stdcall reset(i_chunked_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;

    node_t* p_node = p_pool->p_head;
    while (p_node != NULL)
    {
        // 비할당 상태로 리셋
        char* p_element = (char*)p_node->p_element;
        for (size_t i = 0; i < p_pool->num_elements_per_chunk; ++i)
        {
            header_t* p_header = (header_t*)(p_element + i * p_pool->element_size_with_header);
            p_header->state = STATE_DEALLOC;
        }

        p_node = p_node->p_next;
    }

    p_pool->num_alloc_elements = 0;
}

static void* __stdcall alloc_or_null(i_chunked_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;

    // 할당 가능한 노드 찾기
    node_t* p_node = p_pool->p_head;
    while (p_node != NULL)
    {
        chunk_t* p_chunk = (chunk_t*)p_node->p_element;
        if ((size_t)(p_chunk->pp_index_table_ptr - p_chunk->ppa_index_table) < p_pool->num_elements_per_chunk)
        {
            break;
        }
        p_node = p_node->p_next;
    }

    // 할당 가능한 노드가 없으면 생성
    if (p_node == NULL)
    {
        if (!create_new_chunk(p_pool))
        {
            return NULL;
        }

        p_node = p_pool->p_tail;
    }

    chunk_t* p_chunk = (chunk_t*)p_node->p_element;

    // 헤더 변경
    header_t* p_header = (header_t*)*p_chunk->pp_index_table_ptr;
    p_header->state = STATE_ALLOC;

    ++p_chunk->pp_index_table_ptr;

    ++p_pool->num_alloc_elements;

    return (void*)(p_header + 1);
}

static void __stdcall dealloc(i_chunked_memory_pool_t* p_this, void* p_element_or_null)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;

    if (p_element_or_null == NULL)
    {
        return;
    }

    header_t* p_header = (header_t*)p_element_or_null - 1;
    if (p_header->state != STATE_ALLOC)
    {
        ASSERT(false, L"Access violation");
        return;
    }

    // 헤더 수정
    p_header->state = STATE_DEALLOC;
    chunk_t* p_chunk = (chunk_t*)p_header->p_chunk_node->p_element;

    // 인덱스 테이블 수정
    --p_chunk->pp_index_table_ptr;
    *p_chunk->pp_index_table_ptr = (char*)p_header;

    --p_pool->num_alloc_elements;
}

static size_t __stdcall get_element_size(const i_chunked_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;
    return p_pool->element_size;
}

static size_t __stdcall get_num_elements_per_chunk(const i_chunked_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;
    return p_pool->num_elements_per_chunk;
}

static size_t __stdcall get_num_alloc_elements(const i_chunked_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const chunked_memory_pool_t* p_pool = (chunked_memory_pool_t*)p_this;
    return p_pool->num_alloc_elements;
}

static bool create_new_chunk(chunked_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");

    node_t* pa_new_node = NULL;
    chunk_t* pa_new_chunk = NULL;

    // 청크 노드 생성
    pa_new_node = (node_t*)malloc(sizeof(node_t));
    if (pa_new_node == NULL)
    {
        goto failed_malloc_new_node;
    }

    // 청크 생성
    pa_new_chunk = (chunk_t*)malloc(sizeof(chunk_t));
    if (pa_new_chunk == NULL)
    {
        goto failed_malloc_new_chunk;
    }

    // elements 생성
    pa_new_chunk->pa_elements = (char*)malloc(p_pool->element_size_with_header * p_pool->num_elements_per_chunk);
    if (pa_new_chunk->pa_elements == NULL)
    {
        goto failed_malloc_elements;
    }

    // index table 생성
    pa_new_chunk->ppa_index_table = (char**)malloc(sizeof(char*) * p_pool->num_elements_per_chunk);
    if (pa_new_chunk->ppa_index_table == NULL)
    {
        goto failed_malloc_index_table;
    }

    // elements, index table 초기화
    for (size_t i = 0; i < p_pool->num_elements_per_chunk; ++i)
    {
        header_t* p_header = (header_t*)(pa_new_chunk->pa_elements + i * p_pool->element_size_with_header);
        p_header->p_chunk_node = pa_new_node;
        p_header->state = STATE_DEALLOC;

        // pa_new_chunk->ppa_index_table[i] = (char*)p_header; // C6386: false positive
        *(pa_new_chunk->ppa_index_table + i) = (char*)p_header;
    }
    pa_new_chunk->pp_index_table_ptr = pa_new_chunk->ppa_index_table;

    // 링크 수정
    pa_new_node->p_element = pa_new_chunk;
    list_add_tail(&p_pool->p_head, &p_pool->p_tail, pa_new_node);

    return true;

failed_malloc_index_table:
    SAFE_FREE(pa_new_chunk->pa_elements);

failed_malloc_elements:
    SAFE_FREE(pa_new_chunk);

failed_malloc_new_chunk:
    SAFE_FREE(pa_new_node);

failed_malloc_new_node:
    return false;
}

static void __stdcall create_chunked_memory_pool(i_chunked_memory_pool_t** pp_out_chunked_memory_pool)
{
    ASSERT(pp_out_chunked_memory_pool != NULL, "pp_out_instance == NULL");

    static i_chunked_memory_pool_vtbl_t vtbl =
    {
        initialize,
        release,
        reset,

        alloc_or_null,
        dealloc,

        get_element_size,
        get_num_elements_per_chunk,
        get_num_alloc_elements
    };

    chunked_memory_pool_t* pa_pool = (chunked_memory_pool_t*)malloc(sizeof(chunked_memory_pool_t));
    if (pa_pool == NULL)
    {
        ASSERT(false, "Failed to malloc pool");
        *pp_out_chunked_memory_pool = NULL;
    }
    memset(pa_pool, 0, sizeof(chunked_memory_pool_t));

    pa_pool->base.vtbl = &vtbl;

    *pp_out_chunked_memory_pool = (i_chunked_memory_pool_t*)pa_pool;
}

static void __stdcall destroy_chunked_memory_pool(i_chunked_memory_pool_t* p_chunked_memory_pool)
{
    ASSERT(p_chunked_memory_pool != NULL, "p_chunked_memory_pool == NULL");

    release(p_chunked_memory_pool);
    SAFE_FREE(p_chunked_memory_pool);
}