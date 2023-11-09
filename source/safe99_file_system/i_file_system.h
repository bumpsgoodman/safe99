//***************************************************************************
// 
// 파일: i_file_system.h
// 
// 설명: 파일 시스템 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/10/15
// 
//***************************************************************************

#ifndef I_FILE_SYSTEM_H
#define I_FILE_SYSTEM_H

#include "safe99_common/defines.h"

typedef interface i_file_system i_file_system_t;
typedef interface i_file_system_vtbl
{
    size_t      (__stdcall*     add_ref)(i_file_system_t* p_this);
    size_t      (__stdcall*     release)(i_file_system_t* p_this);
    size_t      (__stdcall*     get_ref_count)(const i_file_system_t* p_this);

    bool        (__stdcall*     initialize)(i_file_system_t* p_this);

    bool        (__stdcall*     load_a8r8g8b8_dds)(i_file_system_t* p_this, const char* filename, i_texture_t** pp_out_texture);
    bool        (__stdcall*     load_safe3d)(i_file_system_t* p_this, const char* filename, i_safe3d_t** pp_out_safe3d);
} i_file_system_vtbl_t;

typedef interface i_file_system
{
    i_file_system_vtbl_t* vtbl;
} i_file_system_t;

#endif // I_FILE_SYSTEM_H