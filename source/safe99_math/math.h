//***************************************************************************
// 
// 파일: math.h
// 
// 설명: 수학 헤더 파일
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/25
// 
//***************************************************************************

#ifndef SAFE99_MATH_H
#define SAFE99_MATH_H

#include <intrin.h>

#include "safe99_common/defines.h"

#include "color.h"
#include "math_defines.h"
#include "math_misc.h"
#include "matrix.h"

START_EXTERN_C

SAFE99_API bool __stdcall log2int64(uint32_t* p_out_index, const uint64_t num);

END_EXTERN_C

#endif // SAFE99_MATH_H