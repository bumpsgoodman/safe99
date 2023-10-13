//***************************************************************************
// 
// 파일: precompiled.h
// 
// 설명: precompiled 헤더
// 
// 작성자: bumpsgoodman
// 
// 작성일: 2023/09/04
// 
//***************************************************************************

#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <memory.h>
#include <stdlib.h>
#include <Windows.h>

#include "safe99_common/assert.h"
#include "safe99_common/safe_delete.h"
#include "safe99_common/defines.h"
#include "safe99_common/types.h"

#include "util/hash_function.h"
#include "util/timer.h"
#include "util/stop_watch.h"

#include "generic/i_chunked_memory_pool.h"
#include "generic/i_dynamic_vector.h"
#include "generic/i_fixed_vector.h"
#include "generic/i_map.h"
#include "generic/i_static_memory_pool.h"
#include "generic/list.h"

#endif // PRECOMPILED_H