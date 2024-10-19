// 작성자: bumpsgoodman
// 작성일: 2024-08-18

#include "Precompiled.h"
#include "MeshExporter.h"

using namespace std;

// Class ID. These must be unique and randomly generated!!
// If you use this as a sample project, this is the first thing
// you should change!
#define SAFE99_MESH_EXP_CLASS_ID	Class_ID(0x100d074c, 0x448c06ff)

HINSTANCE g_hInstance;

static Point3 GetVertexNormal(Mesh* pMesh, const uint_t faceIndex, RVertex* pRVertex);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        MaxSDK::Util::UseLanguagePackLocale();
        g_hInstance = hinstDLL;
        DisableThreadLibraryCalls(g_hInstance);
    }

    return (TRUE);
}

__declspec(dllexport) const TCHAR* LibDescription()
{
    return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec(dllexport) int LibNumberClasses()
{
    return 1;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
    switch (i)
    {
    case 0: return GetUWMeshExpDesc();
    default: return 0;
    }
}

__declspec(dllexport) ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec(dllexport) ULONG CanAutoDefer()
{
    return 1;
}

class MeshExpClassDesc :public ClassDesc {
public:
    int	IsPublic() { return 1; }
    void* Create(BOOL loading = FALSE) { return new MeshExporter; }
    const TCHAR* ClassName() { return GetString(IDS_SAFE99_MESH_EXP); }
    const TCHAR* NonLocalizedClassName() { return _T("MeshExporter"); }
    SClass_ID SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
    Class_ID ClassID() { return SAFE99_MESH_EXP_CLASS_ID; }
    const TCHAR* Category() { return GetString(IDS_CATEGORY); }
};

static MeshExpClassDesc s_uwMeshExpDesc;

ClassDesc* GetUWMeshExpDesc()
{
    return &s_uwMeshExpDesc;
}

MeshExporter::~MeshExporter()
{
}

int MeshExporter::ExtCount()
{
    return 1;
}

const TCHAR* MeshExporter::Ext(int n)
{
    switch (n)
    {
    case 0:
        return _T("UWMesh");
    }

    return _T("");
}

const TCHAR* MeshExporter::LongDesc()
{
    return GetString(IDS_LONGDESC);
}

const TCHAR* MeshExporter::ShortDesc()
{
    return GetString(IDS_SHORTDESC);
}

const TCHAR* MeshExporter::AuthorName()
{
    return _T("bumpsgoodman");
}

const TCHAR* MeshExporter::CopyrightMessage()
{
    return GetString(IDS_COPYRIGHT);
}

const TCHAR* MeshExporter::OtherMessage1()
{
    return _T("");
}

const TCHAR* MeshExporter::OtherMessage2()
{
    return _T("");
}

unsigned int MeshExporter::Version()
{
    return 100;
}

static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg,
                                        WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        CenterWindow(hWnd, GetParent(hWnd));
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hWnd, 1);
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

void MeshExporter::ShowAbout(HWND hWnd)
{
    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}

static INT_PTR CALLBACK ExportDlgProc(HWND hWnd, UINT msg,
    WPARAM wParam, LPARAM lParam)
{
    Interval animRange;
    ISpinnerControl* spin;

    MeshExporter* exp = (MeshExporter*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    switch (msg) {
    case WM_INITDIALOG:
        exp = (MeshExporter*)lParam;
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, lParam);
        CenterWindow(hWnd, GetParent(hWnd));
        //CheckDlgButton(hWnd, IDC_MESHDATA, exp->GetIncludeMesh());
        //CheckDlgButton(hWnd, IDC_ANIMKEYS, exp->GetIncludeAnim());
        //CheckDlgButton(hWnd, IDC_MATERIAL, exp->GetIncludeMtl());
        //CheckDlgButton(hWnd, IDC_MESHANIM, exp->GetIncludeMeshAnim());
        //CheckDlgButton(hWnd, IDC_CAMLIGHTANIM, exp->GetIncludeCamLightAnim());
        //CheckDlgButton(hWnd, IDC_IKJOINTS, exp->GetIncludeIKJoints());
        CheckDlgButton(hWnd, IDC_SKIN, exp->GetIncludeSkin());
        CheckDlgButton(hWnd, IDC_TEXCOORDS, exp->GetIncludeTextureCoords());
        CheckDlgButton(hWnd, IDC_VERTEX_COLORS, exp->GetIncludeVertexColors());
        //CheckDlgButton(hWnd, IDC_OBJ_GEOM, exp->GetIncludeObjGeom());
        //CheckDlgButton(hWnd, IDC_OBJ_SHAPE, exp->GetIncludeObjShape());
        //CheckDlgButton(hWnd, IDC_OBJ_CAMERA, exp->GetIncludeObjCamera());
        //CheckDlgButton(hWnd, IDC_OBJ_LIGHT, exp->GetIncludeObjLight());
        //CheckDlgButton(hWnd, IDC_OBJ_HELPER, exp->GetIncludeObjHelper());

        //CheckRadioButton(hWnd, IDC_RADIO_USEKEYS, IDC_RADIO_SAMPLE,
        //    exp->GetAlwaysSample() ? IDC_RADIO_SAMPLE : IDC_RADIO_USEKEYS);

        // Setup the spinner controls for the controller key sample rate 
        //spin = GetISpinner(GetDlgItem(hWnd, IDC_CONT_STEP_SPIN));
        //spin->LinkToEdit(GetDlgItem(hWnd, IDC_CONT_STEP), EDITTYPE_INT);
        //spin->SetLimits(1, 100, TRUE);
        //spin->SetScale(1.0f);
        //spin->SetValue(exp->GetKeyFrameStep(), FALSE);
        //ReleaseISpinner(spin);

        // Setup the spinner controls for the mesh definition sample rate 
        //spin = GetISpinner(GetDlgItem(hWnd, IDC_MESH_STEP_SPIN));
        //spin->LinkToEdit(GetDlgItem(hWnd, IDC_MESH_STEP), EDITTYPE_INT);
        //spin->SetLimits(1, 100, TRUE);
        //spin->SetScale(1.0f);
        //spin->SetValue(exp->GetMeshFrameStep(), FALSE);
        //ReleaseISpinner(spin);

        // Setup the spinner controls for the floating point precision 
        //spin = GetISpinner(GetDlgItem(hWnd, IDC_PREC_SPIN));
        //spin->LinkToEdit(GetDlgItem(hWnd, IDC_PREC), EDITTYPE_INT);
        //spin->SetLimits(1, 10, TRUE);
        //spin->SetScale(1.0f);
        //spin->SetValue(exp->GetPrecision(), FALSE);
        //ReleaseISpinner(spin);

        // Setup the spinner control for the static frame#
        // We take the frame 0 as the default value
        //animRange = exp->GetInterface()->GetAnimRange();
        //spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN));
        //spin->LinkToEdit(GetDlgItem(hWnd, IDC_STATIC_FRAME), EDITTYPE_INT);
        //spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE);
        //spin->SetScale(1.0f);
        //spin->SetValue(0, FALSE);
        //ReleaseISpinner(spin);

        // Enable / disable mesh options
        EnableWindow(GetDlgItem(hWnd, IDC_SKIN), true);
        EnableWindow(GetDlgItem(hWnd, IDC_TEXCOORDS), true);
        EnableWindow(GetDlgItem(hWnd, IDC_VERTEX_COLORS), true);
        break;

    case CC_SPINNER_CHANGE:
        spin = (ISpinnerControl*)lParam;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_MESHDATA:
            // Enable / disable mesh options
            EnableWindow(GetDlgItem(hWnd, IDC_SKIN), IsDlgButtonChecked(hWnd,
                         IDC_MESHDATA));
            EnableWindow(GetDlgItem(hWnd, IDC_TEXCOORDS), IsDlgButtonChecked(hWnd,
                         IDC_MESHDATA));
            EnableWindow(GetDlgItem(hWnd, IDC_VERTEX_COLORS), IsDlgButtonChecked(hWnd,
                         IDC_MESHDATA));
            break;
        case IDOK:
            //exp->SetIncludeMesh(IsDlgButtonChecked(hWnd, IDC_MESHDATA));
            //exp->SetIncludeAnim(IsDlgButtonChecked(hWnd, IDC_ANIMKEYS));
            //exp->SetIncludeMtl(IsDlgButtonChecked(hWnd, IDC_MATERIAL));
            //exp->SetIncludeMeshAnim(IsDlgButtonChecked(hWnd, IDC_MESHANIM));
            //exp->SetIncludeCamLightAnim(IsDlgButtonChecked(hWnd, IDC_CAMLIGHTANIM));
            //exp->SetIncludeIKJoints(IsDlgButtonChecked(hWnd, IDC_IKJOINTS));
            exp->SetIncludeSkin(IsDlgButtonChecked(hWnd, IDC_SKIN));
            exp->SetIncludeTextureCoords(IsDlgButtonChecked(hWnd, IDC_TEXCOORDS));
            exp->SetIncludeVertexColors(IsDlgButtonChecked(hWnd, IDC_VERTEX_COLORS));
            //exp->SetIncludeObjGeom(IsDlgButtonChecked(hWnd, IDC_OBJ_GEOM));
            //exp->SetIncludeObjShape(IsDlgButtonChecked(hWnd, IDC_OBJ_SHAPE));
            //exp->SetIncludeObjCamera(IsDlgButtonChecked(hWnd, IDC_OBJ_CAMERA));
            //exp->SetIncludeObjLight(IsDlgButtonChecked(hWnd, IDC_OBJ_LIGHT));
            //exp->SetIncludeObjHelper(IsDlgButtonChecked(hWnd, IDC_OBJ_HELPER));
            //exp->SetAlwaysSample(IsDlgButtonChecked(hWnd, IDC_RADIO_SAMPLE));

            //spin = GetISpinner(GetDlgItem(hWnd, IDC_CONT_STEP_SPIN));
            //exp->SetKeyFrameStep(spin->GetIVal());
            //ReleaseISpinner(spin);

            //spin = GetISpinner(GetDlgItem(hWnd, IDC_MESH_STEP_SPIN));
            //exp->SetMeshFrameStep(spin->GetIVal());
            //ReleaseISpinner(spin);

            //spin = GetISpinner(GetDlgItem(hWnd, IDC_PREC_SPIN));
            //exp->SetPrecision(spin->GetIVal());
            //ReleaseISpinner(spin);

            //spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN));
            //exp->SetStaticFrame(spin->GetIVal() * GetTicksPerFrame());
            //ReleaseISpinner(spin);

            EndDialog(hWnd, 1);
            break;
        case IDCANCEL:
            EndDialog(hWnd, 0);
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

int	MeshExporter::DoExport(const TCHAR* pName, ExpInterface* pExpInterface, Interface* pInterface, BOOL bSuppressPrompts, DWORD options)
{
    m_bExportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;

    m_pInterface = pInterface;

    if (!DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SAFE99_MESH_EXPORT_DLG),
        pInterface->GetMAXHWnd(), ExportDlgProc, (LPARAM)this)) {
        return IMPEXP_CANCEL;
    }

    m_pFile = _wfopen(pName, L"wb");
    if (m_pFile == nullptr)
    {
        return IMPEXP_FAIL;
    }

    // 진행 표시줄 시작
    m_pInterface->ProgressStart(GetString(IDS_PROGRESS_MSG), TRUE, nullptr, nullptr);

    m_numCurNode = 0;
    m_numTotalNode = 0;

    // 총 노드 수 계산
    const uint_t numChildren = m_pInterface->GetRootNode()->NumberOfChildren();
    for (uint_t idx = 0; idx < numChildren; idx++)
    {
        if (pInterface->GetCancel())
        {
            break;
        }

        preProcessRecursion(m_pInterface->GetRootNode()->GetChildNode(idx));
    }


    // 오브젝트 저장
    for (uint_t idx = 0; idx < numChildren; idx++)
    {
        if (m_pInterface->GetCancel())
        {
            break;
        }

        exportNodeRecursion(m_pInterface->GetRootNode()->GetChildNode(idx));
    }

    // 진행 표시줄 마침
    m_pInterface->ProgressEnd();

    fclose(m_pFile);

    return IMPEXP_SUCCESS;
}

BOOL MeshExporter::SupportsOptions(int ext, DWORD options)
{
    return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
}

TCHAR* GetString(int id)
{
    static TCHAR buf[256];

    if (g_hInstance)
    {
        return LoadString(g_hInstance, id, buf, _countof(buf)) ? buf : NULL;
    }

    return NULL;
}

void MeshExporter::preProcessRecursion(INode* pNode)
{
    if (m_bExportSelected && pNode->Selected() == FALSE)
    {
        return;
    }

    if (pNode->IsGroupHead())
    {
        return;
    }

    ++m_numTotalNode;

    for (int i = 0; i < pNode->NumberOfChildren(); ++i)
    {
        preProcessRecursion(pNode->GetChildNode(i));
    }
}

void MeshExporter::exportNodeRecursion(INode* pNode)
{
    ++m_numCurNode;
    m_pInterface->ProgressUpdate((int)((float)m_numCurNode / m_numTotalNode * 100.0f));

    if (m_pInterface->GetCancel())
    {
        return;
    }

    if (!m_bExportSelected || pNode->Selected())
    {
        ObjectState os = pNode->EvalWorldState(0);
        if (os.obj != nullptr)
        {
            switch (os.obj->SuperClassID())
            {
            case GEOMOBJECT_CLASS_ID:
                exportGeomObject(pNode);
                break;
            default:
                break;
            }
        }
    }

    for (int i = 0; i < pNode->NumberOfChildren(); ++i)
    {
        exportNodeRecursion(pNode->GetChildNode(i));
    }
}

// 주의사항
// 1. 정점에 UV 매핑이 되어 있지 않으면 작동하지 않음 => 고쳐야 됨
void MeshExporter::exportGeomObject(INode* pNode)
{
    uint_t includeFlag = 0;

    ObjectState os;
    IDerivedObject* pDerivedObj = NULL;
    int skinIndex = -1;
    if (GetIncludeSkin() && findISkinMod(pNode->GetObjectRef(), &pDerivedObj, &skinIndex))
    {
        // We have a skin, because we export its data
        // separately we do not want to include it in the eval
        // Eval the derived object instead, starting at the skin idx
        os = pDerivedObj->Eval(0, skinIndex + 1);
    }
    else
    {
        // We have no skin, evaluate the entire stack
        os = pNode->EvalWorldState(0);
    }

    Object* pObject = os.obj;
    TriObject* pTri = nullptr;
    bool bDeleteTri = false;

    if (pObject->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
    {
        pTri = (TriObject*)pObject->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));

        // Note that the TriObject should only be deleted
        // if the pointer to it is not equal to the object
        // pointer that called ConvertToType()
        if (pObject != pTri)
        {
            bDeleteTri = true;
        }
    }

    Mesh* pMesh = &pTri->GetMesh();

    // 법선 불러오려면 꼭 호출 해야 됨
    pMesh->buildNormals();

    if (bDeleteTri)
    {
        delete pTri;
    }
}

static Point3 GetVertexNormal(Mesh* pMesh, const uint_t faceIndex, RVertex* pRVertex)
{
    Face* pFace = &pMesh->getFace(faceIndex);
    const uint_t smGroup = pFace->smGroup;
    const uint_t numNormals = pRVertex->rFlags & NORCT_MASK;
    Point3 vertexNormal;

    if (numNormals != 0 && smGroup)
    {
        // 법선이 한 개면 RVertex의 rn 멤버에 저장되어있음
        if (numNormals == 1)
        {
            vertexNormal = pRVertex->rn.getNormal();
        }
        // 법선이 여러 개라면 RVertex의 ern 멤버에 저장되어 있음
        else
        {
            // 현재 면과 같은 스무딩 그룹인 법선을 찾아서 추가
            for (uint_t k = 0; k < numNormals; ++k)
            {
                if (pRVertex->ern[k].getSmGroup() & smGroup)
                {
                    vertexNormal = pRVertex->ern[k].getNormal();
                }
            }
        }
    }
    // 면이 스무딩 그룹에 포함되어 있지 않다면 getFaceNormal()로 얻어올 수 있음
    else
    {
        vertexNormal = pMesh->getFaceNormal(faceIndex);
    }

    return vertexNormal;
}

bool MeshExporter::findISkinMod(Object* pObj, IDerivedObject** ppOutDerivedObj, int* pOutSkinIndex)
{
    if (pObj == NULL || pObj->SuperClassID() != GEN_DERIVOB_CLASS_ID)
    {
        // We have not found anything, and we have not more modifiers.
        return false;
    }

    IDerivedObject* pDerived = (IDerivedObject*)pObj;
    for (int i = 0; i < pDerived->NumModifiers(); i++)
    {
        Modifier* pMod = pDerived->GetModifier(i);
        // Does this modifier support the skin interface?
        void* pSkin = pMod->GetInterface(I_SKIN);
        if (pSkin != NULL)
        {
            // Success, we have found a skin!
            *pOutSkinIndex = i;
            *ppOutDerivedObj = pDerived;
            return true;
        }
    }

    // We have not found a skin on this iderived object,
    // however, the IDerivedObject references another object.
    // It is entirely possible for the next object to be a
    // derived object as well.  Recurse to look there as well
    Object* pNextObj = pDerived->GetObjRef();
    return findISkinMod(pNextObj, ppOutDerivedObj, pOutSkinIndex);
}