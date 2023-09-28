//***************************************************************************
// 
// 파일: math.h
// 
// 설명: safe99 전용 수학 헤더 파일
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/25
// 
//***************************************************************************

#ifndef SAFE99_MATH_H
#define SAFE99_MATH_H

#include <intrin.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "safe99_common/assert.h"
#include "safe99_common/defines.h"

#include "math_defines.h"
#include "math_misc.h"
#include "matrix.h"

// 매크로 함수
// -----------------------------------------------------------------

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) (((a) > 0) ? (a) : -(a))
#define ROUND_INT(a) ((int)((a) + 0.5f))

// -----------------------------------------------------------------

START_EXTERN_C

SAFE99_API bool log2int64(uint32_t* p_out_index, const uint64_t num);

END_EXTERN_C

#endif // SAFE99_MATH_H