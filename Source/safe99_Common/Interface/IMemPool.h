// 작성자: bumpsgoodman
// 작성일: 2024-08-18

#ifndef SAFE99_I_MEM_POOL_H
#define SAFE99_I_MEM_POOL_H

typedef SAFE99_INTERFACE IStaticMemPool IStaticMemPool;
SAFE99_INTERFACE IStaticMemPool
{
    bool    (__stdcall  *Init)(IStaticMemPool * pThis, const size_t numElementsPerBlock, const size_t numMaxBlocks, const size_t elementSize);
    void    (__stdcall  *Release)(IStaticMemPool* pThis);
    
    void*   (__stdcall  *Alloc)(IStaticMemPool* pThis);
    void    (__stdcall  *Free)(IStaticMemPool* pThis, void* pMem);
    void    (__stdcall  *Clear)(IStaticMemPool* pThis);
    
    size_t  (__stdcall  *GetNumElementsPerBlock)(const IStaticMemPool* pThis);
    size_t  (__stdcall  *GetNumMaxBlocks)(const IStaticMemPool* pThis);
    size_t  (__stdcall  *GetElementSize)(const IStaticMemPool* pThis);
    size_t  (__stdcall  *GetNumElements)(const IStaticMemPool* pThis);
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

SAFE99_GLOBAL_FUNC void __stdcall CreateStaticMemPool(IStaticMemPool** ppOutPool);
SAFE99_GLOBAL_FUNC void __stdcall DestroyStaticMemPool(IStaticMemPool* pPool);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SAFE99_I_MEM_POOL_H