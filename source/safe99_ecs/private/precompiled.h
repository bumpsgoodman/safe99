//***************************************************************************
// 
// 파일: precompiled.h
// 
// 설명: precompiled 헤더
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/05
// 
//***************************************************************************

#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <intrin.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#include "safe99_common/assert.h"
#include "safe99_common/defines.h"
#include "safe99_common/safe_delete.h"
#include "safe99_common/types.h"

#include "safe99_core/generic/i_chunked_memory_pool.h"
#include "safe99_core/generic/i_dynamic_vector.h"
#include "safe99_core/generic/i_map.h"
#include "safe99_core/generic/list.h"
#include "safe99_core/util/hash_function.h"

#include "safe99_math/math.h"

#endif // PRECOMPILED_H