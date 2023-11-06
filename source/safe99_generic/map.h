//***************************************************************************
// 
// 파일: map.h
// 
// 설명: 해시맵
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/18
// 
//***************************************************************************

#ifndef MAP_H
#define MAP_H

#include "safe99_common/defines.h"
#include "chunked_memory_pool.h"
#include "list.h"

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

SAFE99_API bool __stdcall map_initialize(map_t* p_map, const size_t key_size, const size_t value_size, const size_t num_max_elements);
SAFE99_API void __stdcall map_release(map_t* p_map);
SAFE99_API void __stdcall map_clear(map_t* p_map);

SAFE99_API bool __stdcall map_insert(map_t* p_map, const void* p_key, const size_t key_size, const void* p_value, const size_t value_size);
SAFE99_API bool __stdcall map_insert_by_hash(map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size, const void* p_value, const size_t value_size);

SAFE99_API bool __stdcall map_remove(map_t* p_map, const void* p_key, const size_t key_size);
SAFE99_API bool __stdcall map_remove_by_hash(map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size);

SAFE99_API key_value_t* __stdcall map_find_or_null(const map_t* p_map, const void* p_key, const size_t key_size);
SAFE99_API key_value_t* __stdcall map_find_by_hash_or_null(const map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size);

SAFE99_API size_t __stdcall map_get_count(const map_t* p_map, const void* p_key, const size_t key_size);

SAFE99_API size_t __stdcall map_get_num_elements(const map_t* p_map);
SAFE99_API size_t __stdcall map_get_num_max_elements(const map_t* p_map);

SAFE99_API void* __stdcall map_get_value_or_null(const map_t* p_map, const void* p_key, const size_t key_size);
SAFE99_API void* __stdcall map_get_value_by_hash_or_null(const map_t* p_map, const uint64_t hash, const void* p_key, const size_t key_size);
SAFE99_API key_value_t* __stdcall map_get_key_values_ptr(const map_t* p_map);

END_EXTERN_C

#endif // MAP_H