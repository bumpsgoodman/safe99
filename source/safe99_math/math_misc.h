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

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) (((a) > 0) ? (a) : -(a))

#define ROUND(x) ((x) >= 0.0f ? (float)((int)((x) + 0.5f)) : (float)((int)((x) - 0.5f)))
#define FLOOR(x) ((x) >= 0.0f ? (float)((int)(x)) : (float)((int)(x)))
#define ROUND_INT(x) ((x) >= 0.0f ? (int)((x) + 0.5f) : (int)((x) - 0.5f))
#define FLOOR_INT(x) ((x) >= 0.0f ? (int)(x) : (int)(x))

START_EXTERN_C

SAFE99_API void __stdcall get_sin_cos(const float rad, float* p_out_sin, float* p_out_cos);

END_EXTERN_C

#endif // MATH_MISC_H