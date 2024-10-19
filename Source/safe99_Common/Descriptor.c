// 작성자: bumpsgoodman
// 작성일: 2024-08-20

#include "Precompiled.h"
#include "Descriptor.h"

MATERIAL_DESC* __stdcall GetMaterialDesc(const MESH_DESC* pMesh)
{
    ASSERT(pMesh != NULL, "pMesh is NULL");

    MATERIAL_DESC* pMaterial = (MATERIAL_DESC*)pMesh->pData;
    return pMaterial;
}

VERTEX_DESC* __stdcall GetVertexDesc(const MESH_DESC* pMesh)
{
    ASSERT(pMesh != NULL, "pMesh is NULL");

    MATERIAL_DESC* pMaterial = GetMaterialDesc(pMesh);
    VERTEX_DESC* pVertex = (VERTEX_DESC*)((char*)pMaterial + sizeof(wchar_t) * SAFE99_FILE_NAME_LEN * pMaterial->NumTextures);
    return pVertex;
}

INDEX_BUFFER_DESC* __stdcall GetIndexBufferDesc(const MESH_DESC* pMesh)
{
    ASSERT(pMesh != NULL, "pMesh is NULL");

    char* pVertex = (char*)GetVertexDesc(pMesh);
    INDEX_BUFFER_DESC* pIndexBuffer = (INDEX_BUFFER_DESC*)(pVertex + sizeof(VERTEX_DESC) * pMesh->NumVertices);
    return pIndexBuffer;
}