// 작성자: bumpsgoodman
// 작성일: 2024-08-17

#ifndef SAFE99_I_RENDERER_H
#define SAFE99_I_RENDERER_H

typedef SAFE99_INTERFACE IRenderer IRenderer;
SAFE99_INTERFACE IRenderer
{
    size_t      (__stdcall *AddRef)(IRenderer* pThis);
    size_t      (__stdcall *Release)(IRenderer* pThis);
    size_t      (__stdcall *GetRefCount)(const IRenderer* pThis);

    bool        (__stdcall *Init)(IRenderer* pThis, void* hWnd);

    void        (__stdcall *OnMoveWindow)(IRenderer* pThis);
    void        (__stdcall *OnResizeWindow)(IRenderer* pThis);

    uint_t      (__stdcall *GetWidth)(const IRenderer* pThis);
    uint_t      (__stdcall *GetHeight)(const IRenderer* pThis);

    void        (__stdcall *BeginRender)(IRenderer* pThis);
    void        (__stdcall *EndRender)(IRenderer* pThis);

    void        (__stdcall *Clear)(IRenderer* pThis, const uint32_t argb);
    void        (__stdcall *DrawHorizontalLine)(IRenderer* pThis, const int x, const int y, const uint_t width, const uint32_t argb);
    void        (__stdcall *DrawVerticalLine)(IRenderer* pThis, const int x, const int y, const uint_t height, const uint32_t argb);
    void        (__stdcall *DrawLine)(IRenderer* pThis, const int x0, const int y0, const int x1, const int y1, const uint_t argb);
    void        (__stdcall *DrawBitmap)(IRenderer* pThis, const int x, const int y, const uint_t width, const uint_t height, const void* pBitmap);

    void        (__stdcall *SetMaxFps)(IRenderer* pThis, const uint32_t fps);
    uint32_t    (__stdcall *GetFps)(const IRenderer* pThis);
};

#endif // SAFE99_I_RENDERER_H