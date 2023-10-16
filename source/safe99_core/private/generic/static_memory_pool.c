//***************************************************************************
// 
// 파일: static_memory_pool.c
// 
// 설명: 고정 사이즈 메모리 풀
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/02
// 
//***************************************************************************

#include "precompiled.h"

typedef struct static_memory_pool
{
    i_static_memory_pool_t base;

    size_t element_size;
    size_t num_elements_per_block;
    size_t num_max_blocks;
    size_t num_cur_blocks;
    size_t element_size_with_header;
    size_t num_alloc_elements;

    char** ppa_blocks;
    char*** pppa_index_tables;
    char*** pppa_index_table_ptrs;
} static_memory_pool_t;

typedef enum state
{
    STATE_ALLOC = 95,
    STATE_DEALLOC = 85,
} state_t;

typedef struct header
{
    size_t block_index;
    state_t state;
} header_t;

static bool create_new_block(static_memory_pool_t* p_pool);

static bool __stdcall initialize(i_static_memory_pool_t* p_this, const size_t element_size, const size_t num_elements_per_block, const size_t num_max_blocks)
{
    ASSERT(p_this != NULL, "p_this == NULL");
    ASSERT(element_size > 0, "key_size == 0");
    ASSERT(num_elements_per_block > 0, "num_elements_per_block == 0");
    ASSERT(num_max_blocks > 0, "num_blocks == 0");

    static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;

    p_pool->element_size = element_size;
    p_pool->num_elements_per_block = num_elements_per_block;
    p_pool->num_max_blocks = num_max_blocks;
    p_pool->num_cur_blocks = 0;
    p_pool->element_size_with_header = sizeof(header_t) + element_size;
    p_pool->num_alloc_elements = 0;

    char** pa_blocks = NULL;
    char*** pa_index_tables = NULL;
    char*** pa_index_table_pointers = NULL;

    // 메모리 풀 블럭 생성
    p_pool->ppa_blocks = (char**)malloc(sizeof(char*) * p_pool->num_max_blocks);
    if (p_pool->ppa_blocks == NULL)
    {
        ASSERT(false, "Failed to malloc blocks");
        goto failed_to_malloc_blocks;
    }

    // 인덱스 테이블 생성
    p_pool->pppa_index_tables = (char***)malloc(sizeof(char**) * p_pool->num_elements_per_block * p_pool->num_max_blocks);
    if (p_pool->pppa_index_tables == NULL)
    {
        ASSERT(false, L"Failed to malloc index tables");
        goto failed_to_index_tables;
    }

    // 인덱스 테이블 포인터 생성
    p_pool->pppa_index_table_ptrs = (char***)malloc(sizeof(char**) * p_pool->num_max_blocks);
    if (p_pool->pppa_index_table_ptrs == NULL)
    {
        ASSERT(false, L"Failed to malloc index table pointers");
        goto failed_to_index_table_ptrs;
    }

    // 새 블럭 생성
    if (!create_new_block(p_pool))
    {
        goto failed_create_new_block;
    }

    return true;

failed_create_new_block:
    SAFE_FREE(p_pool->pppa_index_table_ptrs);

failed_to_index_table_ptrs:
    SAFE_FREE(p_pool->pppa_index_tables)

        failed_to_index_tables :
        SAFE_FREE(p_pool->ppa_blocks);

failed_to_malloc_blocks:
    memset(p_pool, 0, sizeof(static_memory_pool_t));
    return false;
}

static void __stdcall release(i_static_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;

    for (size_t i = 0; i < p_pool->num_cur_blocks; ++i)
    {
        SAFE_FREE(p_pool->ppa_blocks[i]);
        SAFE_FREE(p_pool->pppa_index_tables[i]);
    }

    SAFE_FREE(p_pool->pppa_index_table_ptrs);
    SAFE_FREE(p_pool->pppa_index_tables);
    SAFE_FREE(p_pool->ppa_blocks);

    memset(p_pool, 0, sizeof(static_memory_pool_t));
}

static void __stdcall reset(i_static_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;

    // 헤더, 인덱스 테이블 및 인덱스 테이블 포인터 리셋
    for (size_t i = 0; i < p_pool->num_cur_blocks; ++i)
    {
        for (size_t j = 0; j < p_pool->num_elements_per_block; ++j)
        {
            header_t* header = (header_t*)(p_pool->ppa_blocks[i] + j * p_pool->element_size_with_header);
            header->state = STATE_DEALLOC;
            header->block_index = i;
            p_pool->pppa_index_tables[i][j] = (char*)header;
        }

        p_pool->pppa_index_table_ptrs[i] = p_pool->pppa_index_tables[i];
    }

    p_pool->num_cur_blocks = 0;
    p_pool->num_alloc_elements = 0;
}

static void* __stdcall alloc_or_null(i_static_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;

    size_t blockIndex = SIZE_MAX;

    // 할당 가능한 블럭 찾기
    for (size_t i = 0; i < p_pool->num_cur_blocks; ++i)
    {
        const char** basePointer = (const char**)p_pool->pppa_index_tables[i];
        const char** pointer = (const char**)p_pool->pppa_index_table_ptrs[i];
        if ((size_t)(pointer - basePointer) < p_pool->num_elements_per_block)
        {
            blockIndex = i;
            break;
        }
    }

    // 할당 가능한 블럭 없으면 생성
    if (blockIndex == SIZE_MAX)
    {
        if (!create_new_block(p_pool))
        {
            return NULL;
        }

        blockIndex = p_pool->num_cur_blocks - 1;
    }

    header_t* header = (header_t*)(*p_pool->pppa_index_table_ptrs[blockIndex]);
    header->state = STATE_ALLOC;

    ++p_pool->pppa_index_table_ptrs[blockIndex];

    ++p_pool->num_alloc_elements;

    return (void*)(header + 1);
}

static void __stdcall dealloc(i_static_memory_pool_t* p_this, void* p_element_or_null)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;

    if (p_element_or_null == NULL)
    {
        return;
    }

    header_t* header = (header_t*)p_element_or_null - 1;
    if (header->state != STATE_ALLOC)
    {
        ASSERT(false, L"Access violation");
        return;
    }

    // 헤더 수정
    header->state = STATE_DEALLOC;

    // 인덱스 테이블 수정
    --p_pool->pppa_index_table_ptrs[header->block_index];
    *p_pool->pppa_index_table_ptrs[header->block_index] = (char*)header;

    --p_pool->num_alloc_elements;
}

static size_t __stdcall get_element_size(const i_static_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;
    return p_pool->element_size;
}

static size_t __stdcall get_num_elements_per_block(const i_static_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;
    return p_pool->num_elements_per_block;
}

static size_t __stdcall get_num_alloc_elements(const i_static_memory_pool_t* p_this)
{
    ASSERT(p_this != NULL, "p_this == NULL");

    const static_memory_pool_t* p_pool = (static_memory_pool_t*)p_this;
    return p_pool->num_alloc_elements;
}

static bool create_new_block(static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");

    char* pa_new_block = NULL;
    char** ppa_new_index_table = NULL;

    if (p_pool->num_cur_blocks == p_pool->num_max_blocks)
    {
        ASSERT(false, L"p_pool->num_cur_blocks == p_pool->num_max_blocks");
        return false;
    }

    // 블럭 생성
    pa_new_block = (char*)malloc(p_pool->element_size_with_header * p_pool->num_elements_per_block);
    if (pa_new_block == NULL)
    {
        ASSERT(false, L"Failed to malloc new block");
        goto failed_malloc_new_block;
    }

    // 인덱스 테이블 생성
    ppa_new_index_table = (char**)malloc(sizeof(char*) * p_pool->num_elements_per_block);
    if (ppa_new_index_table == NULL)
    {
        ASSERT(false, L"Failed to malloc new block");
        goto failed_malloc_new_index_table;
        return false;
    }

    // 헤더 초기화
    for (size_t i = 0; i < p_pool->num_elements_per_block; ++i)
    {
        header_t* header = (header_t*)(pa_new_block + i * p_pool->element_size_with_header);
        header->state = STATE_DEALLOC;
        header->block_index = p_pool->num_cur_blocks;

        *(ppa_new_index_table + i) = (char*)header;
    }

    p_pool->ppa_blocks[p_pool->num_cur_blocks] = pa_new_block;
    p_pool->pppa_index_tables[p_pool->num_cur_blocks] = ppa_new_index_table;
    p_pool->pppa_index_table_ptrs[p_pool->num_cur_blocks] = ppa_new_index_table;
    ++p_pool->num_cur_blocks;

    return true;

failed_malloc_new_index_table:
    SAFE_FREE(ppa_new_index_table);

failed_malloc_new_block:
    return false;
}

static void __stdcall create_static_memory_pool(i_static_memory_pool_t** pp_out_static_memory_pool)
{
    ASSERT(pp_out_static_memory_pool != NULL, "pp_out_static_memory_pool == NULL");

    static i_static_memory_pool_vtbl_t vtbl =
    {
        initialize,
        release,
        reset,

        alloc_or_null,
        dealloc,

        get_element_size,
        get_num_elements_per_block,
        get_num_alloc_elements
    };

    static_memory_pool_t* pa_pool = (static_memory_pool_t*)malloc(sizeof(static_memory_pool_t));
    if (pa_pool == NULL)
    {
        ASSERT(false, "Failed to malloc pool");
        *pp_out_static_memory_pool = NULL;
    }

    pa_pool->base.vtbl = &vtbl;

    *pp_out_static_memory_pool = (i_static_memory_pool_t*)pa_pool;
}

static void __stdcall destroy_static_memory_pool(i_static_memory_pool_t* p_static_memory_pool)
{
    ASSERT(p_static_memory_pool != NULL, "p_static_memory_pool == NULL");
    
    release(p_static_memory_pool);
    SAFE_FREE(p_static_memory_pool);
}