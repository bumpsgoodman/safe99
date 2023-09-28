//***************************************************************************
// 
// 파일: math_misc.h
// 
// 설명: 기타 수학 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/27
// 
//***************************************************************************

#ifndef MATH_MISC_H
#define MATH_MISC_H

#include "safe99_common/defines.h"

START_EXTERN_C

SAFE99_API void get_sin_cos(const float rad, float* p_out_sin, float* p_out_cos);

END_EXTERN_C

#endif // MATH_MISC_H