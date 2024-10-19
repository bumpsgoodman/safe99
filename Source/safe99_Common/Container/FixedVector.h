// 작성자: bumpsgoodman
// 작성일: 2024-08-17

#ifndef SAFE99_FIXED_VECTOR_H
#define SAFE99_FIXED_VECTOR_H

typedef struct FixedVector
{
    size_t  NumMaxElements;
    size_t  ElementSize;

    size_t  NumElements;
    char*   pElements;

    char*   pLastElement;
} FixedVector;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool            __stdcall   FixedVectorInit(FixedVector* pFixedVector, const size_t numMaxElements, const size_t elementSize);
void            __stdcall   FixedVectorRelease(FixedVector* pFixedVector);

bool            __stdcall   FixedVectorPushBack(FixedVector* pFixedVector, const void* pElement, const size_t elementSize);
bool            __stdcall   FixedVectorPopBack(FixedVector* pFixedVector);

bool            __stdcall   FixedVectorInsert(FixedVector* pFixedVector, const void* pElement, const size_t elementSize, const size_t index);
bool            __stdcall   FixedVectorRemove(FixedVector* pFixedVector, const size_t index);

size_t          __stdcall   FixedVectorGetNumMaxElements(const FixedVector* pFixedVector);
size_t          __stdcall   FixedVectorGetElementSize(const FixedVector* pFixedVector);
size_t          __stdcall   FixedVectorGetNumElements(const FixedVector* pFixedVector);

const void*     __stdcall   FixedVectorGetBack(const FixedVector* pFixedVector);
const void*     __stdcall   FixedVectorGetElement(const FixedVector* pFixedVector, const size_t index);
const void*     __stdcall   FixedVectorGetElementsPtr(const FixedVector* pFixedVector);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SAFE99_FIXED_VECTOR_H