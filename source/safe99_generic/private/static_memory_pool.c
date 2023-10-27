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
#include "static_memory_pool.h"

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

bool __stdcall static_memory_pool_initialize(static_memory_pool_t* p_pool, const size_t element_size, const size_t num_elements_per_block, const size_t num_max_blocks)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");
    ASSERT(element_size > 0, "element_size == 0");
    ASSERT(num_elements_per_block > 0, "num_elements_per_block == 0");
    ASSERT(num_max_blocks > 0, "num_max_blocks == 0");

    p_pool->element_size = element_size;
    p_pool->num_elements_per_block = num_elements_per_block;
    p_pool->num_max_blocks = num_max_blocks;
    p_pool->num_cur_blocks = 0;
    p_pool->element_size_with_header = sizeof(header_t) + element_size;
    p_pool->num_alloc_elements = 0;

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
        ASSERT(false, "Failed to create new block");
        goto failed_create_new_block;
    }

    return true;

failed_create_new_block:
    SAFE_FREE(p_pool->pppa_index_table_ptrs);

failed_to_index_table_ptrs:
    SAFE_FREE(p_pool->pppa_index_tables)

failed_to_index_tables:
    SAFE_FREE(p_pool->ppa_blocks);

failed_to_malloc_blocks:
    memset(p_pool, 0, sizeof(static_memory_pool_t));
    return false;
}

void __stdcall static_memory_pool_release(static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");

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

void __stdcall static_memory_pool_reset(static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");

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

void* __stdcall static_memory_pool_alloc_or_null(static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");

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
            ASSERT(false, "Failed to create new block");
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

void __stdcall static_memory_pool_dealloc(static_memory_pool_t* p_pool, void* p_element_or_null)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");

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

size_t __stdcall static_memory_pool_get_element_size(const static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");
    return p_pool->element_size;
}

size_t __stdcall static_memory_pool_get_num_elements_per_block(const static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");
    return p_pool->num_elements_per_block;
}

size_t __stdcall static_memory_pool_get_num_alloc_elements(const static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");
    return p_pool->num_alloc_elements;
}

static bool create_new_block(static_memory_pool_t* p_pool)
{
    ASSERT(p_pool != NULL, "p_pool == NULL");

    char* pa_new_block = NULL;
    char** ppa_new_index_table = NULL;

    if (p_pool->num_cur_blocks == p_pool->num_max_blocks)
    {
        ASSERT(false, L"saturate");
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