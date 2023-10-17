//***************************************************************************
// 
// 파일: i_dynamic_vector.h
// 
// 설명: 가변 사이즈 벡터 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/08/10
// 
//***************************************************************************

#ifndef I_DYNAMIC_VECTOR_H
#define I_DYNAMIC_VECTOR_H

#include "safe99_common/defines.h"

typedef interface i_dynamic_vector i_dynamic_vector_t;

typedef interface i_dynamic_vector_vtbl
{
    bool        (__stdcall*     initialize)(i_dynamic_vector_t* p_this, const size_t element_size, const size_t num_max_elements);
    void        (__stdcall*     release)(i_dynamic_vector_t* p_this);
    void        (__stdcall*     clear)(i_dynamic_vector_t* p_this);

    bool        (__stdcall*     push_back)(i_dynamic_vector_t* p_this, const void* p_element, const size_t element_size);
    bool        (__stdcall*     push_back_empty)(i_dynamic_vector_t* p_this);
    bool        (__stdcall*     pop_back)(i_dynamic_vector_t* p_this);

    bool        (__stdcall*     insert)(i_dynamic_vector_t* p_this, const void* p_element, const size_t element_size, const size_t index);
    bool        (__stdcall*     insert_empty)(i_dynamic_vector_t* p_this, const size_t index);
    bool        (__stdcall*     remove)(i_dynamic_vector_t* p_this, const size_t index);

    size_t      (__stdcall*     get_element_size)(const i_dynamic_vector_t* p_this);
    size_t      (__stdcall*     get_num_max_elements)(const i_dynamic_vector_t* p_this);
    size_t      (__stdcall*     get_num_elements)(const i_dynamic_vector_t* p_this);

    void*       (__stdcall*     get_back_or_null)(const i_dynamic_vector_t* p_this);
    void*       (__stdcall*     get_element_or_null)(const i_dynamic_vector_t* p_this, const size_t index);
    char*       (__stdcall*     get_elements_ptr_or_null)(const i_dynamic_vector_t* p_this);
} i_dynamic_vector_vtbl_t;

typedef interface i_dynamic_vector
{
    i_dynamic_vector_vtbl_t* vtbl;
} i_dynamic_vector_t;

START_EXTERN_C

SAFE99_API void __stdcall create_dynamic_vector(i_dynamic_vector_t** pp_out_dynamic_vector);
SAFE99_API void __stdcall destroy_dynamic_vector(i_dynamic_vector_t* p_dynamic_vector);

END_EXTERN_C

#endif // I_DYNAMIC_VECTOR_H