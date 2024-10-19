// 작성자: bumpsgoodman
// 작성일: 2024-08-10

#include "Precompiled.h"
#include "safe99_Common/Interface/IMemPool.h"

#define HEADER_SIZE sizeof(size_t)

typedef struct StaticMemPool
{
    IStaticMemPool  Vtbl;

    size_t          NumElementsPerBlock;
    size_t          NumMaxBlocks;
    size_t          ElementSize;
    size_t          ElementSizeWithHeader;
    size_t          NumBlocks;
    size_t          NumElements;
    size_t          NumMaxElements;

    char**          ppBlocks;
    char***         pppIndexTable;
    char***         pppIndexTablePtr;
} StaticMemPool;

static bool    __stdcall   Init(IStaticMemPool* pThis, const size_t numElementsPerBlock, const size_t numMaxBlocks, const size_t elementSize);
static void    __stdcall   Release(IStaticMemPool* pThis);

static void*   __stdcall   Alloc(IStaticMemPool* pThis);
static void    __stdcall   Free(IStaticMemPool* pThis, void* pMem);
static void    __stdcall   Clear(IStaticMemPool* pThis);

static size_t  __stdcall   GetNumElementsPerBlock(const IStaticMemPool* pThis);
static size_t  __stdcall   GetNumMaxBlocks(const IStaticMemPool* pThis);
static size_t  __stdcall   GetElementSize(const IStaticMemPool* pThis);
static size_t  __stdcall   GetNumElements(const IStaticMemPool* pThis);

static IStaticMemPool s_vtbl =
{
    Init,
    Release,

    Alloc,
    Free,
    Clear,

    GetNumElementsPerBlock,
    GetNumMaxBlocks,
    GetElementSize,
    GetNumElements,
};

bool __stdcall Init(IStaticMemPool* pThis, const size_t numElementsPerBlock, const size_t numMaxBlocks, const size_t elementSize)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    ASSERT(numElementsPerBlock > 0, "numElementsPerBlock is 0");
    ASSERT(numMaxBlocks > 0, "numMaxBlocks is 0");
    ASSERT(elementSize > 0, "elementSize is 0");

    StaticMemPool* pPool = (StaticMemPool*)pThis;

    pPool->Vtbl = s_vtbl;

    pPool->NumElementsPerBlock = numElementsPerBlock;
    pPool->NumMaxBlocks = numMaxBlocks;
    pPool->ElementSize = elementSize;
    pPool->ElementSizeWithHeader = HEADER_SIZE + elementSize;
    pPool->NumBlocks = 1;
    pPool->NumElements = 0;
    pPool->NumMaxElements = numElementsPerBlock * numMaxBlocks;

    pPool->ppBlocks = (char**)malloc(PTR_SIZE * numMaxBlocks);
    ASSERT(pPool->ppBlocks != NULL, "Failed to malloc");

    memset(pPool->ppBlocks, 0, PTR_SIZE * numMaxBlocks);

    pPool->ppBlocks[0] = (char*)malloc(pPool->ElementSizeWithHeader * numElementsPerBlock);
    ASSERT(pPool->ppBlocks[0] != NULL, "Failed to malloc");

    pPool->pppIndexTable = (char***)malloc(PTR_SIZE * numMaxBlocks);
    ASSERT(pPool->pppIndexTable != NULL, "Failed to malloc");

    char** ppIndexTable = (char**)malloc(PTR_SIZE * numElementsPerBlock);
    ASSERT(ppIndexTable != NULL, "Failed to malloc");

    pPool->pppIndexTablePtr = (char***)malloc(PTR_SIZE * numMaxBlocks);
    ASSERT(pPool->pppIndexTablePtr != NULL, "Failed to malloc");

    // 인덱스 테이블 초기화
    for (size_t i = 0; i < numElementsPerBlock; ++i)
    {
        size_t* pHeader = (size_t*)(pPool->ppBlocks[0] + i * pPool->ElementSizeWithHeader);
        *pHeader = 0;
        ppIndexTable[i] = (char*)pHeader;
    }

    pPool->pppIndexTable[0] = ppIndexTable;
    pPool->pppIndexTablePtr[0] = ppIndexTable;

    return true;
}

void __stdcall Release(IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    StaticMemPool* pPool = (StaticMemPool*)pThis;
    
    for (size_t i = 0; i < pPool->NumBlocks; ++i)
    {
        SAFE_FREE(pPool->ppBlocks[i]);
        SAFE_FREE(pPool->pppIndexTable[i]);
    }

    SAFE_FREE(pPool->ppBlocks);
    SAFE_FREE(pPool->pppIndexTable);
    SAFE_FREE(pPool->pppIndexTablePtr);

    memset(pPool, 0, sizeof(StaticMemPool));
}

void* __stdcall Alloc(IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    StaticMemPool* pPool = (StaticMemPool*)pThis;

    if (pPool->NumElements >= pPool->NumMaxElements)
    {
        ASSERT(false, "Pool is full");
        safe99_SetLastError(SAFE99_ERROR_CODE_MEM_POOL_ALLOC_FULL);
        return NULL;
    }

    // 할당 가능한 블럭 찾기
    size_t blockIndex;
    size_t indexInBlock = 0;
    for (blockIndex = 0; blockIndex < pPool->NumBlocks; ++blockIndex)
    {
        const size_t numAllocElements = (size_t)(pPool->pppIndexTablePtr[blockIndex] - pPool->pppIndexTable[blockIndex]);
        if (numAllocElements < pPool->NumElementsPerBlock)
        {
            ++pPool->pppIndexTablePtr[blockIndex];
            indexInBlock = numAllocElements;
            break;
        }
    }

    // 할당 가능한 블럭이 없으면 블럭 생성
    if (blockIndex >= pPool->NumBlocks)
    {
        char* pBlock = (char*)malloc(pPool->ElementSizeWithHeader * pPool->NumElementsPerBlock);
        ASSERT(pBlock != NULL, "Failed to mallc");

        char** ppIndexTable = (char**)malloc(PTR_SIZE * pPool->NumElementsPerBlock);
        ASSERT(ppIndexTable != NULL, "Failed to malloc");

        // 인덱스 테이블 초기화
        for (size_t i = 0; i < pPool->NumElementsPerBlock; ++i)
        {
            size_t* pHeader = (size_t*)(pBlock + i * pPool->ElementSizeWithHeader);
            *pHeader = blockIndex;
            ppIndexTable[i] = (char*)pHeader;
        }

        pPool->ppBlocks[blockIndex] = pBlock;
        pPool->pppIndexTable[blockIndex] = ppIndexTable;
        pPool->pppIndexTablePtr[blockIndex] = ppIndexTable;
        ++pPool->NumBlocks;
    }

    char* pMem = pPool->pppIndexTable[blockIndex][indexInBlock] + HEADER_SIZE;
    ++pPool->NumElements;

    return pMem;
}

void __stdcall Free(IStaticMemPool* pThis, void* pMem)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    StaticMemPool* pPool = (StaticMemPool*)pThis;

    char* pHeader = (char*)pMem - HEADER_SIZE;
    const size_t blockIndex = *(size_t*)pHeader;
    *(--pPool->pppIndexTablePtr[blockIndex]) = pHeader;

    --pPool->NumElements;
}

void __stdcall Clear(IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    StaticMemPool* pPool = (StaticMemPool*)pThis;
    for (size_t i = 0; i < pPool->NumBlocks; ++i)
    {
        for (size_t j = 0; j < pPool->NumElementsPerBlock; ++j)
        {
            // 헤더 초기화
            size_t* pHeader = (size_t*)(pPool->ppBlocks[i] + j * pPool->ElementSizeWithHeader);
            //*pHeader = i;

            pPool->pppIndexTable[i][j] = (char*)pHeader;
        }

        pPool->pppIndexTablePtr[i] = pPool->pppIndexTable[i];
    }

    pPool->NumElements = 0;
}

size_t __stdcall GetNumElementsPerBlock(const IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    return ((StaticMemPool*)pThis)->NumElementsPerBlock;
}

size_t __stdcall GetNumMaxBlocks(const IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    return ((StaticMemPool*)pThis)->NumMaxBlocks;
}

size_t __stdcall GetElementSize(const IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    return ((StaticMemPool*)pThis)->ElementSize;
}

size_t __stdcall GetNumElements(const IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    StaticMemPool* pPool = (StaticMemPool*)pThis;
    return pPool->NumElements;
}

void __stdcall CreateStaticMemPool(IStaticMemPool** ppOutPool)
{
    StaticMemPool* pPool = (StaticMemPool*)malloc(sizeof(StaticMemPool));
    ASSERT(pPool != NULL, "Failed to malloc");

    pPool->Vtbl = s_vtbl;
    *ppOutPool = &pPool->Vtbl;
}

void __stdcall DestroyStaticMemPool(IStaticMemPool* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Release(pThis);
    SAFE_FREE(pThis);
}