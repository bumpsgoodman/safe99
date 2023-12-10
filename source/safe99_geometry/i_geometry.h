//***************************************************************************
// 
// 파일: i_geometry.h
// 
// 설명: geometry 인터페이스
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/11/17
// 
//***************************************************************************

#ifndef I_GEOMETRY_H
#define I_GEOMETRY_H

#include "i_ecs.h"
#include "transform2.h"
#include "transform3.h"
#include "camera3.h"

typedef interface i_geometry i_geometry_t;
typedef interface i_geometry_vtbl
{
    size_t      (__stdcall*     add_ref)(i_geometry_t* p_this);
    size_t      (__stdcall*     release)(i_geometry_t* p_this);
    size_t      (__stdcall*     get_ref_count)(const i_geometry_t* p_this);

    bool        (__stdcall*     initialize)(i_geometry_t* p_this);

    i_ecs_t*    (__stdcall*     get_ecs)(const i_geometry_t* p_this);
} i_geometry_vtbl_t;

typedef interface i_geometry
{
    i_geometry_vtbl_t* vtbl;
} i_geometry_t;

#endif // I_GEOMETRY_H