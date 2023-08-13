#include <memory.h>
#include <stdlib.h>
#include <stdint.h>

#include "generic/map.h"
#include "util/assert.h"
#include "util/hash_function.h"
#include "util/safe_delete.h"

static const size_t S_BUCKET_SIZE_LIST[] =
{
    7u, 23u, 97u, 397u, 1597u, 6421u, 25717u, 102877u,
    411527u, 879743u, 1799639u, 6584983u, 26339969u, 52679969u
};
static const size_t S_NUM_BUCKET_SIZE_LIST = sizeof(S_BUCKET_SIZE_LIST) / sizeof(size_t);

static bool expand(map_t* p_map);

bool map_init(map_t* p_map, const size_t key_size, const size_t value_size, const size_t num_max_elements)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    ASSERT(key_size > 0, "key_size == 0");
    ASSERT(value_size > 0, "value_size == 0");
    ASSERT(num_max_elements <= S_BUCKET_SIZE_LIST[S_NUM_BUCKET_SIZE_LIST - 1] / 2, "overflow");

    p_map->key_size = key_size;
    p_map->value_size = value_size;
    p_map->num_max_elements = num_max_elements;
    p_map->factor = 0.75f;
    p_map->num_elements = 0;

    // 버켓 사이즈 구하기
    for (size_t i = 0; i < S_NUM_BUCKET_SIZE_LIST; ++i)
    {
        const size_t size = S_BUCKET_SIZE_LIST[i];
        if (size > num_max_elements * 2)
        {
            p_map->bucket_size = size;
            break;
        }
    }

    // 버켓 생성
    p_map->pa_bucket = (list_t*)malloc(sizeof(list_t) * p_map->bucket_size);
    if (p_map->pa_bucket == NULL)
    {
        ASSERT(false, "Failed to malloc bucket");
        goto failed_malloc_bucket;
    }
    memset(p_map->pa_bucket, 0, sizeof(list_t) * p_map->bucket_size);

    // 키-값 저장소 생성
    p_map->pa_key_values = (key_value_t*)malloc(sizeof(key_value_t) * num_max_elements);
    if (p_map->pa_key_values == NULL)
    {
        ASSERT(false, "Failed to malloc key values");
        goto failed_malloc_key_values;
    }

    // 메모리 풀 초기화 (노드)
    if (!chunked_memory_pool_init(&p_map->node_pool, sizeof(list_node_t), num_max_elements))
    {
        ASSERT(false, "Failed to init node pool");
        goto failed_init_node_pool;
    }

    // 메모리 풀 초기화 (키)
    if (!chunked_memory_pool_init(&p_map->key_pool, key_size, num_max_elements))
    {
        ASSERT(false, "Failed to init key pool");
        goto failed_init_key_pool;
    }

    // 메모리 풀 초기화 (값)
    if (!chunked_memory_pool_init(&p_map->value_pool, value_size, num_max_elements))
    {
        ASSERT(false, "Failed to init value pool");
        goto failed_init_value_pool;
    }

    return true;

failed_init_value_pool:
    chunked_memory_pool_release(&p_map->key_pool);

failed_init_key_pool:
    chunked_memory_pool_release(&p_map->value_pool);

failed_init_node_pool:
    SAFE_FREE(p_map->pa_key_values);

failed_malloc_key_values:
    SAFE_FREE(p_map->pa_bucket);

failed_malloc_bucket:
    memset(p_map, 0, sizeof(map_t));
    return false;
}

void map_release(map_t* p_map)
{
    ASSERT(p_map != NULL, "p_map == NULL");

    SAFE_FREE(p_map->pa_bucket);
    SAFE_FREE(p_map->pa_key_values);

    chunked_memory_pool_release(&p_map->node_pool);
    chunked_memory_pool_release(&p_map->key_pool);
    chunked_memory_pool_release(&p_map->value_pool);

    memset(p_map, 0, sizeof(map_t));
}

void map_clear(map_t* p_map)
{
    ASSERT(p_map != NULL, "p_map == NULL");

    p_map->num_elements = 0;

    memset(p_map->pa_bucket, 0, sizeof(list_t) * p_map->bucket_size);

    chunked_memory_pool_reset(&p_map->node_pool);
    chunked_memory_pool_reset(&p_map->key_pool);
    chunked_memory_pool_reset(&p_map->value_pool);
}

bool map_insert(map_t* p_map, const void* p_key, const size_t key_size, const void* p_value, const size_t value_size)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    ASSERT(p_key != NULL, "p_key == NULL");
    ASSERT(p_value != NULL, "p_value == NULL");

    if (p_map->key_size != key_size)
    {
        ASSERT(false, "mismatch key size");
        return false;
    }

    if (p_map->value_size != value_size)
    {
        ASSERT(false, "mismatch value size");
        return false;
    }

    if (p_map->num_elements >= p_map->num_max_elements)
    {
        expand(p_map);
    }

    const uint64_t hash = hash64_fnv1a((const char*)p_key, key_size);
    const size_t index = hash % p_map->bucket_size;

    list_t* p_list = p_map->pa_bucket + index;

    // 이미 같은 키가 있는지 검사
    // 같은 키가 있다면 값 바꾸기
    {
        list_node_t* p_node = p_list->p_head;
        while (p_node != NULL && memcmp(((key_value_t*)p_node->p_element)->p_key, p_key, key_size) != 0)
        {
            p_node = p_node->p_next;
        }

        if (p_node != NULL)
        {
            key_value_t* key_value = (key_value_t*)p_node->p_element;
            memcpy(key_value->p_value, p_value, value_size);

            return true;
        }
    }

    list_node_t* p_node = (list_node_t*)chunked_memory_pool_alloc_or_null(&p_map->node_pool);
    key_value_t* key_value = p_map->pa_key_values + p_map->num_elements;

    // 키-값 복사
    key_value->p_key = chunked_memory_pool_alloc_or_null(&p_map->key_pool);
    key_value->p_value = chunked_memory_pool_alloc_or_null(&p_map->value_pool);
    memcpy(key_value->p_key, p_key, key_size);
    memcpy(key_value->p_value, p_value, value_size);

    key_value->index = index; // 제거할 때 필요

    p_node->p_element = key_value;
    list_add_tail(&p_list->p_head, &p_list->p_tail, p_node);

    ++p_map->num_elements;

    return true;
}

bool map_remove(map_t* p_map, const void* p_key, const size_t key_size)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    ASSERT(p_key != NULL, "p_key == NULL");

    if (p_map->key_size != key_size)
    {
        ASSERT(false, "mismatch key size");
        return false;
    }

    const uint64_t hash = hash64_fnv1a((const char*)p_key, key_size);
    const size_t index = hash % p_map->bucket_size;

    list_t* p_list = p_map->pa_bucket + index;

    // 키 찾기
    list_node_t* p_node = p_list->p_head;
    while (p_node != NULL && memcmp(((key_value_t*)p_node->p_element)->p_key, p_key, key_size) != 0)
    {
        p_node = p_node->p_next;
    }

    if (p_node == NULL)
    {
        return false;
    }

    key_value_t* p_key_value = (key_value_t*)p_node->p_element;
    key_value_t* p_last_key_value = p_map->pa_key_values + (p_map->num_elements - 1);
    void* p_deleted_key = p_key_value->p_key;
    void* p_deleted_value = p_key_value->p_value;
    *p_key_value = *p_last_key_value;

    list_t* p_last_list = p_map->pa_bucket + p_last_key_value->index;
    list_node_t* p_last_node = p_last_list->p_head;
    while (p_last_node != NULL
           && memcmp(p_last_key_value->p_key, ((key_value_t*)p_last_node->p_element)->p_key, key_size) != 0)
    {
        p_last_node = p_last_node->p_next;
    }

    // 이 시점에서 마지막 노드는 NULL이 될 수 없음
    ASSERT(p_last_node != NULL, "p_last_node == NULL");
    p_last_node->p_element = p_key_value;

    list_delete_node(&p_list->p_head, &p_list->p_tail, p_node);
    chunked_memory_pool_dealloc(&p_map->key_pool, p_key_value->p_key);
    chunked_memory_pool_dealloc(&p_map->value_pool, p_key_value->p_value);
    chunked_memory_pool_dealloc(&p_map->node_pool, p_node);

    --p_map->num_elements;

    return true;
}

size_t map_get_num_elements(map_t* p_map)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    return p_map->num_elements;
}

size_t map_get_num_max_elements(map_t* p_map)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    return p_map->num_max_elements;
}

key_value_t* map_find_or_null(map_t* p_map, const void* p_key, const size_t key_size)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    ASSERT(p_key != NULL, "p_key == NULL");

    if (p_map->key_size != key_size)
    {
        ASSERT(false, "mismatch key size");
        return NULL;
    }

    const uint64_t hash = hash64_fnv1a((const char*)p_key, key_size);
    const size_t index = hash % p_map->bucket_size;

    list_t* p_list = p_map->pa_bucket + index;
    list_node_t* p_node = p_list->p_head;
    while (p_node != NULL && memcmp(((key_value_t*)p_node->p_element)->p_key, p_key, key_size) != 0)
    {
        p_node = p_node->p_next;
    }

    if (p_node == NULL)
    {
        return NULL;
    }

    return (key_value_t*)p_node->p_element;
}

void* map_get_value_or_null(map_t* p_map, const void* p_key, const size_t key_size)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    ASSERT(p_key != NULL, "p_key == NULL");

    key_value_t* p_key_value = map_find_or_null(p_map, p_key, key_size);
    if (p_key_value == NULL)
    {
        return NULL;
    }

    return p_key_value->p_value;
}

size_t map_get_count(map_t* p_map, const void* p_key, const size_t key_size)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    ASSERT(p_key != NULL, "p_key == NULL");
    ASSERT(p_map->key_size == key_size, "mismatch key size");

    const uint64_t hash = hash64_fnv1a((const char*)p_key, key_size);
    const size_t index = hash % p_map->bucket_size;
    size_t count = 0;

    list_t* p_list = p_map->pa_bucket + index;
    list_node_t* p_node = p_list->p_head;
    while (p_node != NULL)
    {
        if (memcmp(((key_value_t*)p_node->p_element)->p_key, p_key, key_size) == 0)
        {
            ++count;
        }

        p_node = p_node->p_next;
    }

    return count;
}

key_value_t* map_get_key_values_ptr(map_t* p_map)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    ASSERT(p_map->pa_key_values != NULL, "pa_key_values == NULL");

    return p_map->pa_key_values;
}

static bool expand(map_t* p_map)
{
    ASSERT(p_map != NULL, "p_map == NULL");
    
    size_t new_bucket_size = 0;
    list_t* pa_new_bucket = NULL;
    key_value_t* pa_new_key_values = NULL;

    const size_t new_num_max_elements = p_map->num_max_elements + p_map->node_pool.num_elements_per_chunk;

    pa_new_key_values = (key_value_t*)malloc(sizeof(key_value_t) * new_num_max_elements);
    if (pa_new_key_values == NULL)
    {
        ASSERT(false, "Failed to malloc new key values");
        goto failed_malloc_new_key_values;
    }

    // 버켓 사이즈 구하기
    for (size_t i = 0; i < S_NUM_BUCKET_SIZE_LIST; ++i)
    {
        const size_t size = S_BUCKET_SIZE_LIST[i];
        if (size > new_num_max_elements * 2)
        {
            new_bucket_size = size;
            break;
        }
    }
    memcpy(pa_new_key_values, p_map->pa_key_values, sizeof(key_value_t) * p_map->num_elements);

    pa_new_bucket = (list_t*)malloc(sizeof(list_t) * new_num_max_elements);
    if (pa_new_bucket == NULL)
    {
        ASSERT(false, "Failed to malloc new bucket");
        goto failed_malloc_new_bucket;
    }
    memset(pa_new_bucket, 0, sizeof(list_t) * new_num_max_elements);

    key_value_t* p_key_values = p_map->pa_key_values;
    for (size_t i = 0; i < p_map->num_elements; ++i)
    {
        key_value_t* p_key_value = &p_key_values[i];
        key_value_t* p_new_key_value = &pa_new_key_values[i];

        const uint64_t hash = hash64_fnv1a((const char*)p_key_value->p_key, p_map->key_size);
        const size_t index = hash % new_bucket_size;

        p_new_key_value->p_key = p_key_value->p_key;
        p_new_key_value->p_value = p_key_value->p_value;
        p_new_key_value->index = index;

        pa_new_bucket[index] = p_map->pa_bucket[p_key_value->index];
        pa_new_bucket[index].p_head->p_element = p_new_key_value;
    }

    SAFE_FREE(p_map->pa_bucket);
    p_map->pa_bucket = pa_new_bucket;

    SAFE_FREE(p_map->pa_key_values);
    p_map->pa_key_values = pa_new_key_values;

    p_map->num_max_elements = new_num_max_elements;

    return true;

failed_malloc_new_bucket:
    SAFE_FREE(pa_new_key_values);

failed_malloc_new_key_values:
    return false;
}