// 작성자: bumpsgoodman
// 작성일: 2024-08-18﻿

/*
* 메모리 레이아웃
* -----------------------
* 매직넘버                   => char[12], "safe99Mesh"
* 
* 메시 개수
* 메시
*   머티리얼 파일명          => wchar_t[64]
*   버텍스 개수              => uint16_t
*   인덱스 버퍼 개수         => uint16_t
* 
*   버텍스 (N개)
*     - 위치                 => float[3]
*     - 노말                 => float[3]
* 
*   인덱스 버퍼 (N개)
*     - 인덱스 개수          => uint16_t
*     - 인덱스               => uint16_t[]
*/

#ifndef MESH_EXPORTER_H
#define MESH_EXPORTER_H

#define CFGFILENAME		_T("SAFE99_MESH_EXPORTER.CFG")	// Configuration file

using namespace std;

extern ClassDesc*   GetUWMeshExpDesc();
extern TCHAR*       GetString(int id);
extern HINSTANCE    g_hInstance;

class MeshExporter final : public SceneExport
{
public:
    MeshExporter() = default;
    MeshExporter(const MeshExporter&) = delete;
    MeshExporter& operator=(const MeshExporter&) = delete;
    ~MeshExporter();

    // SceneExport 메서드
    int             ExtCount();             // 지원되는 확장자 수
    const TCHAR*    Ext(int n);             // 확장자
    const TCHAR*    LongDesc();             // 긴 설명
    const TCHAR*    ShortDesc();            // 짧은 설명
    const TCHAR*    AuthorName();           // 작성자
    const TCHAR*    CopyrightMessage();     // 저작권 메시지
    const TCHAR*    OtherMessage1();        // Other message #1
    const TCHAR*    OtherMessage2();        // Other message #2
    unsigned int    Version();              // 버전 번호
    void            ShowAbout(HWND hWnd);   // DLL About 박스
    int	            DoExport(const TCHAR* pName, ExpInterface* pExpInterface, Interface* pInterface, BOOL bSuppressPrompts = FALSE, DWORD options = 0); // Export 실행 함수
    BOOL            SupportsOptions(int ext, DWORD options);

    inline bool     GetIncludeTextureCoords() const;
    inline bool     GetIncludeSkin() const;
    inline bool     GetIncludeVertexColors() const;

    inline void     SetIncludeTextureCoords(bool bActive);
    inline void     SetIncludeSkin(bool bActive);
    inline void     SetIncludeVertexColors(bool bActive);

private:
    void            preProcessRecursion(INode* pNode);
    void            exportNodeRecursion(INode* pNode);
    void            exportGeomObject(INode* pNode);

    bool            findISkinMod(Object* pObj, IDerivedObject** ppOutDerivedObj, int* pOutSkinIndex);

private:
    size_t          m_numTotalNode = 0;
    size_t          m_numCurNode = 0;
    Interface*      m_pInterface = nullptr;

    bool            m_bExportSelected = false;

    bool            m_bIncludeTextureCoord = false;
    bool            m_bIncludeSkin = false;
    bool            m_bIncludeVertexColor = false;

    FILE*           m_pFile = nullptr;
};

inline bool MeshExporter::GetIncludeTextureCoords() const
{
    return m_bIncludeTextureCoord;
}

inline bool MeshExporter::GetIncludeSkin() const
{
    return m_bIncludeSkin;
}

inline bool MeshExporter::GetIncludeVertexColors() const
{
    return m_bIncludeVertexColor;
}

inline void MeshExporter::SetIncludeTextureCoords(bool bActive)
{
    m_bIncludeTextureCoord = bActive;
}

inline void MeshExporter::SetIncludeSkin(bool bActive)
{
    m_bIncludeSkin = bActive;
}

inline void MeshExporter::SetIncludeVertexColors(bool bActive)
{
    m_bIncludeVertexColor = bActive;
}

#endif // MESH_EXPORTER_H