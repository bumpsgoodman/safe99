// 작성자: bumpsgoodman
// 작성일: 2024-08-20

#ifndef SAFE99_DESCRIPTOR_H
#define SAFE99_DESCRIPTOR_H

#include "Common.h"
#include "Container/LinkedList.h"

typedef struct MATERIAL_DESC
{
    uint16_t    NumTextures;
    char        pTextureFilenames[1];   // wchar_t[n][SAFE99_FILE_NAME_LEN]
} MATERIAL_DESC;

typedef struct VERTEX_DESC
{
    float       Positions[3];
    float       Normals[3];
} VERTEX_DESC;

typedef struct INDEX_BUFFER_DESC
{
    uint16_t    NumIndices;
    char        pIndices[1]; // uint16_t[n]
} INDEX_BUFFER_DESC;

typedef struct MESH_DESC
{
    uint16_t            NumMaterials;
    uint16_t            NumVertices;
    uint16_t            NumIndexBuffers;
    uint16_t            MaterialId;
    struct MESH_DESC*   pNext;

    char                pData[1];
} MESH_DESC;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

MATERIAL_DESC*      __stdcall   GetMaterialDesc(const MESH_DESC* pMesh);
VERTEX_DESC*        __stdcall   GetVertexDesc(const MESH_DESC* pMesh);
INDEX_BUFFER_DESC*  __stdcall   GetIndexBufferDesc(const MESH_DESC* pMesh);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SAFE99_DESCRIPTOR_H