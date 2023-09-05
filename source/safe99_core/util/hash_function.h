#ifndef HASH_FUNCTION_H
#define HASH_FUNCTION_H

#include <stddef.h>
#include <stdint.h>

#include "safe99_common/defines.h"

START_EXTERN_C

SAFE99_API uint64_t hash64_fnv1a(const char* bytes, const size_t size);

END_EXTERN_C

#endif // HASH_FUNCTION_H