#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>

#include "chunked_memory_pool.h"
#include "list.h"
#include "safe99_common/defines.h"

typedef struct map_key_value
{
    void* p_key;
    void* p_value;

    // 제거 및 확장에 필요
    // 수정 금지
    size_t index;
} key_value_t;

typedef struct map
{
    size_t key_size;
    size_t value_size;
    size_t num_max_elements;
    float factor;

    key_value_t* pa_key_values;
    size_t num_elements;

    list_t* pa_bucket; // <key_value_t*>
    size_t bucket_size;

    chunked_memory_pool_t key_pool;
    chunked_memory_pool_t value_pool;
    chunked_memory_pool_t node_pool;
} map_t;

START_EXTERN_C

// key_size는 0보다 커야 함
// value_size는 0보다 커야 함
// num_max_elements의 크기는 [0 > num_max_elements <= 26,339,984]
// 
// 이미 초기화 된 map을 다시 초기화하지 말 것
// 해야 한다면 map_release() 호출 이후에 초기화 진행
SAFE99_API bool map_init(map_t* p_map, const size_t key_size, const size_t value_size, const size_t num_max_elements);
SAFE99_API void map_release(map_t* p_map);
SAFE99_API void map_clear(map_t* p_map);

SAFE99_API bool map_insert(map_t* p_map, const void* p_key, const size_t key_size, const void* p_value, const size_t value_size);
SAFE99_API bool map_insert_by_hash(map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size, const void* p_value, const size_t value_size);

SAFE99_API bool map_remove(map_t* p_map, const void* p_key, const size_t key_size);
SAFE99_API bool map_remove_by_hash(map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size);

SAFE99_API size_t map_get_num_elements(map_t* p_map);

SAFE99_API size_t map_get_num_max_elements(map_t* p_map);

SAFE99_API key_value_t* map_find_or_null(map_t* p_map, const void* p_key, const size_t key_size);
SAFE99_API key_value_t* map_find_by_hash_or_null(map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size);

SAFE99_API void* map_get_value_or_null(map_t* p_map, const void* p_key, const size_t key_size);
SAFE99_API void* map_get_value_by_hash_or_null(map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size);

SAFE99_API size_t map_get_count(map_t* p_map, const void* p_key, const size_t key_size);

SAFE99_API key_value_t* map_get_key_values_ptr(map_t* p_map);

END_EXTERN_C

#endif // MAP_H