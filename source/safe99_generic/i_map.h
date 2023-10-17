//***************************************************************************
// 
// 파일: i_map.h
// 
// 설명: 해시맵 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/18
// 
//***************************************************************************

#ifndef I_MAP_H
#define I_MAP_H

#include "safe99_common/defines.h"

typedef struct map_key_value
{
    void* p_key;
    void* p_value;

    // 제거 및 확장에 필요
    // 수정 금지
    size_t index;
} key_value_t;

typedef interface i_map i_map_t;

typedef interface i_map_vtbl
{
    bool            (__stdcall*     initialize)(i_map_t* p_this, const size_t key_size, const size_t value_size, const size_t num_max_elements);
    void            (__stdcall*     release)(i_map_t* p_this);
    void            (__stdcall*     clear)(i_map_t* p_this);

    bool            (__stdcall*     insert)(i_map_t* p_this, const void* p_key, const size_t key_size, const void* p_value, const size_t value_size);
    bool            (__stdcall*     insert_by_hash)(i_map_t* p_this, const uint64_t hash, const void* p_key, const size_t key_size, const void* p_value, const size_t value_size);

    bool            (__stdcall*     remove)(i_map_t* p_this, const void* p_key, const size_t key_size);
    bool            (__stdcall*     remove_by_hash)(i_map_t* p_this, const uint64_t hash, const void* p_key, const size_t key_size);

    key_value_t*    (__stdcall*     find_or_null)(const i_map_t* p_this, const void* p_key, const size_t key_size);
    key_value_t*    (__stdcall*     find_by_hash_or_null)(const i_map_t* p_this, const uint64_t hash, const void* p_key, const size_t key_size);
    
    size_t          (__stdcall*     get_count)(const i_map_t* p_this, const void* p_key, const size_t key_size);

    size_t          (__stdcall*     get_num_elements)(const i_map_t* p_this);
    size_t          (__stdcall*     get_num_max_elements)(const i_map_t* p_this);

    void*           (__stdcall*     get_value_or_null)(const i_map_t* p_this, const void* p_key, const size_t key_size);
    void*           (__stdcall*     get_value_by_hash_or_null)(const i_map_t* p_this, const uint64_t hash, const void* p_key, const size_t key_size);
    key_value_t*    (__stdcall*     get_key_values_ptr)(const i_map_t* p_this);
} i_map_vtbl_t;

typedef interface i_map
{
    i_map_vtbl_t* vtbl;
} i_map_t;

START_EXTERN_C

SAFE99_API void __stdcall create_map(i_map_t** pp_out_map);
SAFE99_API void __stdcall destroy_map(i_map_t* p_map);

END_EXTERN_C

#endif // I_MAP_H