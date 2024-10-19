// 작성자: bumpsgoodman
// 작성일: 2024-09-18

#include "Precompiled.h"
#include "safe99_Common/Common.h"
#include "safe99_Common/Interface/IFileSystem.h"

typedef struct FileSystem
{
    IFileSystem     Vtbl;
    size_t          RefCount;
} FileSystem;

static size_t       __stdcall   AddRef(IFileSystem* pThis);
static size_t       __stdcall   Release(IFileSystem* pThis);
static size_t       __stdcall   GetRefCount(const IFileSystem* pThis);

static bool         __stdcall   Init(IFileSystem* pThis);

static bool         __stdcall   LoadDDS_A8R8G8B8(IFileSystem* pThis, const wchar_t* pFilename, TEXTURE* pOutTexture);

static const IFileSystem s_vtbl =
{
    AddRef,
    Release,
    GetRefCount,

    Init,

    LoadDDS_A8R8G8B8,
};

size_t __stdcall AddRef(IFileSystem* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    FileSystem* pRenderer = (FileSystem*)pThis;
    return ++pRenderer->RefCount;
}

size_t __stdcall Release(IFileSystem* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    FileSystem* pFileSystem = (FileSystem*)pThis;
    if (--pFileSystem->RefCount == 0)
    {
        SAFE_FREE(pFileSystem);
        return 0;
    }

    return pFileSystem->RefCount;
}

size_t __stdcall GetRefCount(const IFileSystem* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    FileSystem* pRenderer = (FileSystem*)pThis;
    return pRenderer->RefCount;
}

bool __stdcall Init(IFileSystem* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    FileSystem* pRenderer = (FileSystem*)pThis;

    return true;
}

bool __stdcall LoadDDS_A8R8G8B8(IFileSystem* pThis, const wchar_t* pFilename, TEXTURE* pOutTexture)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    ASSERT(pFilename != NULL, "pFilename is NULL");
    ASSERT(pOutTexture != NULL, "pOutTexture is NULL");

    FileSystem* pRenderer = (FileSystem*)pThis;

    bool bResult = false;

    FILE* pFile = _wfopen(pFilename, L"rb");
    if (pFile == NULL)
    {
        ASSERT(false, "Failed to open file");
        goto lb_return;
    }

    uint32_t magic;
    fread(&magic, sizeof(uint32_t), 1, pFile);

    const char* pMagic = (const char*)&magic;
    if (pMagic[0] != 'D'
        || pMagic[1] != 'D'
        || pMagic[2] != 'S'
        || pMagic[3] != ' ')
    {
        ASSERT(false, "mismatch header");
        goto lb_return;
    }

    char ddsHeader[124];
    fread(ddsHeader, sizeof(char), 124, pFile);

    const uint32_t height = *(uint32_t*)&ddsHeader[8];
    const uint32_t width = *(uint32_t*)&ddsHeader[12];

    char* pBitmap = (char*)malloc(width * height * sizeof(uint32_t));
    if (pBitmap == NULL)
    {
        ASSERT(false, "Failed to malloc bitmap");
        goto lb_return;
    }

    fread(pBitmap, 1, width * height * sizeof(uint32_t), pFile);

    fclose(pFile);

    pOutTexture->Width = width;
    pOutTexture->Height = height;
    pOutTexture->pBitmap = pBitmap;

    fclose(pFile);
    bResult = true;

lb_return:
    return bResult;
}

void __stdcall CreateDllInstance(void** ppOutInstance)
{
    ASSERT(ppOutInstance != NULL, "ppOutInstance is NULL");

    FileSystem* pFileSystem = (FileSystem*)malloc(sizeof(FileSystem));
    ASSERT(pFileSystem != NULL, "Failed to malloc");

    pFileSystem->Vtbl = s_vtbl;
    pFileSystem->RefCount = 1;

    *ppOutInstance = pFileSystem;
}