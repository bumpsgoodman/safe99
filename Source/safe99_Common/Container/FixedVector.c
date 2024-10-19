// 작성자: bumpsgoodman
// 작성일: 2024-08-17

#include "Precompiled.h"
#include "FixedVector.h"
#include "../Common.h"

bool __stdcall FixedVectorInit(FixedVector* pFixedVector, size_t numMaxElements, size_t elementSize)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    ASSERT(numMaxElements > 0, "numMaxElements is 0");
    ASSERT(elementSize > 0, "elementSize is 0");

    pFixedVector->NumMaxElements = numMaxElements;
    pFixedVector->ElementSize = elementSize;
    pFixedVector->NumElements = 0;

    pFixedVector->pElements = (char*)malloc(elementSize * numMaxElements);
    ASSERT(pFixedVector->pElements != NULL, "Failed to malloc");

    pFixedVector->pLastElement = pFixedVector->pElements;

    return true;
}

void __stdcall FixedVectorRelease(FixedVector* pFixedVector)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");

    SAFE_FREE(pFixedVector->pElements);
}

bool __stdcall FixedVectorPushBack(FixedVector* pFixedVector, const void* pElement, size_t elementSize)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    ASSERT(pElement != NULL, "pElement is NULL");
    ASSERT(elementSize <= pFixedVector->ElementSize, "Mismatch elementSize");

    if (pFixedVector->NumElements >= pFixedVector->NumMaxElements)
    {
        ASSERT(false, "Full");
        safe99_SetLastError(SAFE99_ERROR_CODE_CONTAINER_FULL);
        return false;
    }

    void* pDst = pFixedVector->pLastElement++;
    memcpy(pDst, pElement, elementSize);

    ++pFixedVector->NumElements;

    return true;
}

bool __stdcall FixedVectorPopBack(FixedVector* pFixedVector)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");

    if (pFixedVector->NumElements == 0)
    {
        ASSERT(false, "Empty");
        safe99_SetLastError(SAFE99_ERROR_CODE_CONTAINER_EMPTY);
        return false;
    }

    ++pFixedVector->NumElements;
    --pFixedVector->pLastElement;

    return true;
}

bool __stdcall FixedVectorInsert(FixedVector* pFixedVector, const void* pElement, size_t elementSize, size_t index)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    ASSERT(pElement != NULL, "pElement is NULL");
    ASSERT(elementSize <= pFixedVector->ElementSize, "Mismatch elementSize");

    if (index > pFixedVector->NumElements)
    {
        ASSERT(false, "Invalid index");
        safe99_SetLastError(SAFE99_ERROR_CODE_CONTAINER_INVALID_INDEX);
        return false;
    }

    // element 한 칸씩 밀기
    const size_t len = pFixedVector->NumElements - index;
    char* pDst = pFixedVector->pElements + pFixedVector->ElementSize * (index + 1);
    char* pSrc = pDst - pFixedVector->ElementSize;
    memmove(pDst, pSrc, pFixedVector->ElementSize * len);

    memcpy(pSrc, pElement, elementSize);

    ++pFixedVector->NumElements;
    ++pFixedVector->pLastElement;

    return true;
}

bool __stdcall FixedVectorRemove(FixedVector* pFixedVector, size_t index)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");

    if (index >= pFixedVector->NumElements)
    {
        ASSERT(false, "Invalid index");
        safe99_SetLastError(SAFE99_ERROR_CODE_CONTAINER_INVALID_INDEX);
        return false;
    }

    // element 한 칸씩 당기기
    const size_t len = pFixedVector->NumElements - index;
    char* pDst = pFixedVector->pElements + pFixedVector->ElementSize * index;
    const char* pSrc = pDst + pFixedVector->ElementSize;
    memmove(pDst, pSrc, pFixedVector->ElementSize * len);

    --pFixedVector->NumElements;
    --pFixedVector->pLastElement;

    return true;
}

size_t __stdcall FixedVectorGetNumMaxElements(const FixedVector* pFixedVector)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    return pFixedVector->NumMaxElements;
}

size_t __stdcall FixedVectorGetElementSize(const FixedVector* pFixedVector)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    return pFixedVector->ElementSize;
}

size_t __stdcall FixedVectorGetNumElements(const FixedVector* pFixedVector)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    return pFixedVector->NumElements;
}

const void* __stdcall FixedVectorGetBack(const FixedVector* pFixedVector)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    return pFixedVector->pLastElement;
}

const void* __stdcall FixedVectorGetElement(const FixedVector* pFixedVector, size_t index)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    if (index >= pFixedVector->NumElements)
    {
        ASSERT(false, "Invalid index");
        safe99_SetLastError(SAFE99_ERROR_CODE_CONTAINER_INVALID_INDEX);
        return false;
    }

    return pFixedVector->pElements + pFixedVector->ElementSize * index;
}

const void* __stdcall FixedVectorGetElementsPtr(const FixedVector* pFixedVector)
{
    ASSERT(pFixedVector != NULL, "pFixedVector is NULL");
    return pFixedVector->pElements;
}