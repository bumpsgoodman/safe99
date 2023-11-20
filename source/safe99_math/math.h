//***************************************************************************
// 
// 파일: math.h
// 
// 설명: 수학 함수
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/25
// 
//***************************************************************************

#ifndef SAFE99_MATH_H
#define SAFE99_MATH_H

#include "safe99_common/defines.h"
#include "color.h"
#include "math_defines.h"
#include "math_misc.h"
#include "matrix.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) (((a) > 0) ? (a) : -(a))

#define ROUND(x) ((x) >= 0.0f ? (float)((int)((x) + 0.5f)) : (float)((int)((x) - 0.5f)))
#define FLOOR(x) ((x) >= 0.0f ? (float)((int)(x)) : (float)((int)(x)))
#define ROUND_INT(x) ((x) >= 0.0f ? (int)((x) + 0.5f) : (int)((x) - 0.5f))
#define FLOOR_INT(x) ((x) >= 0.0f ? (int)(x) : (int)(x))

#endif // SAFE99_MATH_H