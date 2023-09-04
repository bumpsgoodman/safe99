#ifndef PRECOMPILED_H
#define PRECOMPILED_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <Windows.h>

#include "safe99_common/assert.h"
#include "safe99_common/safe_delete.h"
#include "safe99_common/defines.h"

#include "util/hash_function.h"
#include "util/timer.h"

#include "generic/chunked_memory_pool.h"
#include "generic/dynamic_vector.h"
#include "generic/fixed_vector.h"
#include "generic/list.h"
#include "generic/map.h"
#include "generic/static_memory_pool.h"

#endif // PRECOMPILED_H