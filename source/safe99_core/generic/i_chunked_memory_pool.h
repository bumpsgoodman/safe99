//***************************************************************************
// 
// 파일: i_chunked_memory_pool.h
// 
// 설명: 가변 사이즈 메모리 풀 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/07/05
// 
//***************************************************************************

#ifndef I_CHUNKED_MEMORY_POOL_H
#define I_CHUNKED_MEMORY_POOL_H

#include "safe99_common/defines.h"

typedef interface i_chunked_memory_pool i_chunked_memory_pool_t;

typedef interface i_chunked_memory_pool_vtbl
{
    bool    (__stdcall*     initialize)(i_chunked_memory_pool_t* p_this, const size_t element_size, const size_t num_elements_per_chunk);
    void    (__stdcall*     release)(i_chunked_memory_pool_t* p_this);
    void    (__stdcall*     reset)(i_chunked_memory_pool_t* p_this);

    void*   (__stdcall*     alloc_or_null)(i_chunked_memory_pool_t* p_this);
    void    (__stdcall*     dealloc)(i_chunked_memory_pool_t* p_this, void* p_element_or_null);

    size_t  (__stdcall*     get_element_size)(const i_chunked_memory_pool_t* p_this);
    size_t  (__stdcall*     get_num_elements_per_chunk)(const i_chunked_memory_pool_t* p_this);
    size_t  (__stdcall*     get_num_alloc_elements)(const i_chunked_memory_pool_t* p_this);
} i_chunked_memory_pool_vtbl_t;

typedef interface i_chunked_memory_pool
{
    i_chunked_memory_pool_vtbl_t* vtbl;
} i_chunked_memory_pool_t;

START_EXTERN_C

SAFE99_API void __stdcall create_chunked_memory_pool(i_chunked_memory_pool_t** pp_out_chunked_memory_pool);
SAFE99_API void __stdcall destroy_chunked_memory_pool(i_chunked_memory_pool_t* p_chunked_memory_pool);

END_EXTERN_C

#endif // I_CHUNKED_MEMORY_POOL_H