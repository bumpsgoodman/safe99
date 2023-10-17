//***************************************************************************
// 
// 파일: i_static_memory_pool.h
// 
// 설명: 고정 사이즈 메모리 풀 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/02
// 
//***************************************************************************

#ifndef I_STATIC_MEMORY_POOL_H
#define I_STATIC_MEMORY_POOL_H

#include "safe99_common/defines.h"

typedef interface i_static_memory_pool i_static_memory_pool_t;

typedef interface i_static_memory_pool_vtbl
{
    bool    (__stdcall*     initialize)(i_static_memory_pool_t* p_this, const size_t element_size, const size_t num_elements_per_block, const size_t num_max_blocks);
    void    (__stdcall*     release)(i_static_memory_pool_t* p_this);
    void    (__stdcall*     reset)(i_static_memory_pool_t* p_this);

    void*   (__stdcall*     alloc_or_null)(i_static_memory_pool_t* p_this);
    void    (__stdcall*     dealloc)(i_static_memory_pool_t* p_this, void* p_element_or_null);

    size_t  (__stdcall*     get_element_size)(const i_static_memory_pool_t* p_this);
    size_t  (__stdcall*     get_num_elements_per_block)(const i_static_memory_pool_t* p_this);
    size_t  (__stdcall*     get_num_alloc_elements)(const i_static_memory_pool_t* p_this);
} i_static_memory_pool_vtbl_t;

typedef interface i_static_memory_pool
{
    i_static_memory_pool_vtbl_t* vtbl;
} i_static_memory_pool_t;

START_EXTERN_C

SAFE99_API void __stdcall create_static_memory_pool(i_static_memory_pool_t** pp_out_static_memory_pool);
SAFE99_API void __stdcall destroy_static_memory_pool(i_static_memory_pool_t* p_static_memory_pool);

END_EXTERN_C

#endif // I_STATIC_MEMORY_POOL_H