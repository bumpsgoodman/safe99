// 작성자: bumpsgoodman
// 작성일: 2024-09-18

#ifndef SAFE99_I_FILE_SYSTEM_H
#define SAFE99_I_FILE_SYSTEM_H

typedef struct TEXTURE
{
    uint32_t    Width;
    uint32_t    Height;
    char*       pBitmap;
} TEXTURE;

typedef SAFE99_INTERFACE IFileSystem IFileSystem;
SAFE99_INTERFACE IFileSystem
{
    size_t      (__stdcall *AddRef)(IFileSystem* pThis);
    size_t      (__stdcall *Release)(IFileSystem* pThis);
    size_t      (__stdcall *GetRefCount)(const IFileSystem* pThis);

    bool        (__stdcall *Init)(IFileSystem* pThis);

    bool        (__stdcall *LoadDDS_A8R8G8B8)(IFileSystem* pThis, const wchar_t* pFilename, TEXTURE* pOutTexture);
};

#endif // SAFE99_I_FILE_SYSTEM_H