#include "util/hash_function.h"
#include "util/assert.h"

uint64_t hash64_fnv1a(const char* bytes, const size_t size)
{
    ASSERT(bytes != NULL, "bytes == NULL");

    static const uint64_t FNV_PRIME = 1099511628211U;
    static const uint64_t FNV_OFFSET_BASIS = 14695981039346656037U;

    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < size; ++i)
    {
        hash = hash ^ bytes[i];
        hash = hash * FNV_PRIME;
    }

    return hash;
}