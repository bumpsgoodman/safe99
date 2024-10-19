// 작성자: bumpsgoodman
// 작성일: 2024-08-24
// 
// SSE로 작성된 소프트웨어 렌더러

#include "Precompiled.h"
#include "safe99_Common/Common.h"
#include "safe99_Common/Util/HighPerformanceTimer.h"
#include "safe99_Common/Interface/IRenderer.h"
#include "safe99_Math/safe99_Math.inl"
#include "Clipping.h"

#define NUM_MAX_BACK_BUFFERS 1

#define USE_SSE 1

typedef struct Renderer
{
    IRenderer   Vtbl;
    size_t      RefCount;

    HWND        hWnd;

    uint32_t*   pBackBuffers[NUM_MAX_BACK_BUFFERS];
    uint8_t     BackBufferIndex;
    uint_t      Pitch;
    uint_t      Width;
    uint_t      Height;

    HDC         hdc;
    HBITMAP     hBitmap;
    BITMAPINFO  Bmi;

    HIGH_PERFORMANCE_TIMER  FrameTimer;
    uint_t                  MaxFps;
    uint_t                  Fps;
    float                   TicksPerFrame;
} Renderer;

static size_t       __stdcall   AddRef(IRenderer* pThis);
static size_t       __stdcall   Release(IRenderer* pThis);
static size_t       __stdcall   GetRefCount(const IRenderer* pThis);

static bool         __stdcall   Init(IRenderer* pThis, void* hWnd);

static void         __stdcall   OnMoveWindow(IRenderer* pThis);
static void         __stdcall   OnResizeWindow(IRenderer* pThis);

static uint_t       __stdcall   GetWidth(const IRenderer* pThis);
static uint_t       __stdcall   GetHeight(const IRenderer* pThis);

static void         __stdcall   BeginRender(IRenderer* pThis);
static void         __stdcall   EndRender(IRenderer* pThis);

static void         __stdcall   Clear(IRenderer* pThis, const uint32_t argb);
static void         __stdcall   DrawHorizontalLine(IRenderer* pThis, const int x, const int y, const uint_t width, const uint32_t argb);
static void         __stdcall   DrawVerticalLine(IRenderer* pThis, const int x, const int y, const uint_t height, const uint32_t argb);
static void         __stdcall   DrawLine(IRenderer* pThis, const int x0, const int y0, const int x1, const int y1, const uint_t argb);
static void         __stdcall   DrawBitmap(IRenderer* pThis, const int x, const int y, const uint_t width, const uint_t height, const void* pBitmap);

static void         __stdcall   SetMaxFps(IRenderer* pThis, const uint_t fps);
static uint_t       __stdcall   GetFps(const IRenderer* pThis);

static const IRenderer s_vtbl =
{
    AddRef,
    Release,
    GetRefCount,

    Init,

    OnMoveWindow,
    OnResizeWindow,

    GetWidth,
    GetHeight,

    BeginRender,
    EndRender,

    Clear,
    DrawHorizontalLine,
    DrawVerticalLine,
    DrawLine,
    DrawBitmap,

    SetMaxFps,
    GetFps
};

size_t __stdcall AddRef(IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;
    return ++pRenderer->RefCount;
}

size_t __stdcall Release(IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;
    if (--pRenderer->RefCount == 0)
    {
        DeleteObject(pRenderer->hBitmap);
        ReleaseDC(pRenderer->hWnd, pRenderer->hdc);

        for (size_t i = 0; i < NUM_MAX_BACK_BUFFERS; ++i)
        {
            SAFE_FREE(pRenderer->pBackBuffers[i]);
        }

        SAFE_FREE(pRenderer);
        return 0;
    }

    return pRenderer->RefCount;
}

size_t __stdcall GetRefCount(const IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;
    return pRenderer->RefCount;
}

bool __stdcall Init(IRenderer* pThis, void* hWnd)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

    bool bResult = false;

    RECT windowRect;
    GetClientRect((HWND)hWnd, &windowRect);
    const uint_t windowWidth = windowRect.right - windowRect.left;
    const uint_t windowHeight = windowRect.bottom - windowRect.top;

    const uint_t padding = DEFAULT_ALIGN - windowWidth % DEFAULT_ALIGN;
    const uint_t pitch = windowWidth + ((padding == DEFAULT_ALIGN) ? 0 : padding);

    for (size_t i = 0; i < NUM_MAX_BACK_BUFFERS; ++i)
    {
        uint32_t* pBackBuffer = (uint32_t*)malloc(4 * pitch * windowHeight);
        ASSERT(pBackBuffer != NULL, "Failed to malloc");

        memset(pBackBuffer, 0, 4 * pitch * windowHeight);
        pRenderer->pBackBuffers[i] = pBackBuffer;
    }

    pRenderer->BackBufferIndex = 0;
    pRenderer->Pitch = pitch;
    pRenderer->Width = windowWidth;
    pRenderer->Height = windowHeight;

    memset(&pRenderer->Bmi, 0, sizeof(pRenderer->Bmi));
    pRenderer->Bmi.bmiHeader.biSize = sizeof(pRenderer->Bmi);
    pRenderer->Bmi.bmiHeader.biWidth = (LONG)pitch;
    pRenderer->Bmi.bmiHeader.biHeight = -(LONG)windowHeight;
    pRenderer->Bmi.bmiHeader.biBitCount = 32;
    pRenderer->Bmi.bmiHeader.biCompression = BI_RGB;
    pRenderer->Bmi.bmiHeader.biPlanes = 1;

    pRenderer->hWnd = (HWND)hWnd;
    pRenderer->hdc = GetDC(pRenderer->hWnd);
    pRenderer->hBitmap = CreateCompatibleBitmap(pRenderer->hdc, (int)pitch, (int)windowHeight);

    SelectObject(pRenderer->hdc, pRenderer->hBitmap);

    HighPerformanceTimerInit(&pRenderer->FrameTimer);
    pRenderer->MaxFps = UINT32_MAX;
    pRenderer->TicksPerFrame = 0.0f;
    pRenderer->Fps = 0;

    bResult = true;

    return bResult;
}

void __stdcall OnMoveWindow(IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

    StretchDIBits(pRenderer->hdc,
                  0, 0, (int)pRenderer->Pitch, (int)pRenderer->Height,
                  0, 0, (int)pRenderer->Pitch, (int)pRenderer->Height,
                  pRenderer->pBackBuffers[pRenderer->BackBufferIndex], &pRenderer->Bmi, DIB_RGB_COLORS, SRCCOPY);
}

// TODO: 크기에 따라 객체들의 위치도 바뀌도록 수정
void __stdcall OnResizeWindow(IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

    RECT windowRect;
    GetClientRect(pRenderer->hWnd, &windowRect);
    const uint_t windowWidth = windowRect.right - windowRect.left;
    const uint_t windowHeight = windowRect.bottom - windowRect.top;

    const uint_t padding = DEFAULT_ALIGN - windowWidth % DEFAULT_ALIGN;
    const uint_t pitch = windowWidth + ((padding == DEFAULT_ALIGN) ? 0 : padding);

    if (pitch == pRenderer->Pitch
        && windowHeight == pRenderer->Height)
    {
        return;
    }

    //HBITMAP hNewBitmap = CreateCompatibleBitmap(pRenderer->hdc, (int)pitch, (int)windowHeight);
    //HBITMAP hOldBitmap = (HBITMAP)SelectObject(pRenderer->hdc, hNewBitmap);
    //DeleteObject(hOldBitmap);

    HDC hNewDC = CreateCompatibleDC(pRenderer->hdc);
    HBITMAP hNewBitmap = CreateCompatibleBitmap(hNewDC, (int)pitch, (int)windowHeight);
    SelectObject(hNewDC, hNewBitmap);

    const uint_t minPitch = MIN(pitch, pRenderer->Pitch);
    const uint_t minHeight = MIN(windowHeight, pRenderer->Height);

    for (size_t i = 0; i < NUM_MAX_BACK_BUFFERS; ++i)
    {
        uint32_t* pBackBuffer = (uint32_t*)malloc(4 * pitch * windowHeight);
        ASSERT(pBackBuffer != NULL, "Failed to malloc");

        memset(pBackBuffer, 0, 4 * pitch * windowHeight);
        memcpy(pBackBuffer, pRenderer->pBackBuffers[i], 4 * minPitch * minHeight);

        SAFE_FREE(pRenderer->pBackBuffers[i]);
        pRenderer->pBackBuffers[i] = pBackBuffer;
    }

    pRenderer->BackBufferIndex = 0;
    pRenderer->Pitch = pitch;
    pRenderer->Width = windowWidth;
    pRenderer->Height = windowHeight;

    pRenderer->Bmi.bmiHeader.biWidth = (LONG)pitch;
    pRenderer->Bmi.bmiHeader.biHeight = -(LONG)windowHeight;

    BOOL a = BitBlt(hNewDC, 0, 0, (int)minPitch, (int)minHeight, pRenderer->hdc, 0, 0, SRCCOPY);

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(pRenderer->hdc, hNewBitmap);
    BitBlt(pRenderer->hdc, 0, 0, (int)minPitch, (int)minHeight, hNewDC, 0, 0, SRCCOPY);

#if 0
    StretchDIBits(pRenderer->hdc,
                  0, 0, (int)minPitch, (int)minHeight,
                  0, 0, (int)minPitch, (int)minHeight,
                  pRenderer->pBackBuffers[pRenderer->BackBufferIndex], &pRenderer->Bmi, DIB_RGB_COLORS, SRCCOPY);
#endif
    return;
}

uint_t __stdcall GetWidth(const IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;
    return pRenderer->Width;
}

uint_t __stdcall GetHeight(const IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;
    return pRenderer->Height;
}

void __stdcall BeginRender(IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

    HighPerformanceTimerUpdate(&pRenderer->FrameTimer, pRenderer->TicksPerFrame);
}

void __stdcall EndRender(IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

    StretchDIBits(pRenderer->hdc,
                  0, 0, (int)pRenderer->Pitch, (int)pRenderer->Height,
                  0, 0, (int)pRenderer->Pitch, (int)pRenderer->Height,
                  pRenderer->pBackBuffers[pRenderer->BackBufferIndex], &pRenderer->Bmi, DIB_RGB_COLORS, SRCCOPY);

    pRenderer->BackBufferIndex = (pRenderer->BackBufferIndex + 1) % NUM_MAX_BACK_BUFFERS;

    float deltaTime = HighPerformanceTimerGetDeltaTime(&pRenderer->FrameTimer);
    while (deltaTime < pRenderer->TicksPerFrame)
    {
        HighPerformanceTimerUpdate(&pRenderer->FrameTimer, pRenderer->TicksPerFrame);
        deltaTime = HighPerformanceTimerGetDeltaTime(&pRenderer->FrameTimer);
    }

    pRenderer->Fps = (uint_t)ROUND_INT((1.0f / deltaTime));
}

void __stdcall Clear(IRenderer* pThis, const uint32_t argb)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

#if USE_SSE
    __m128i* pStartBufferSSE = (__m128i*)(pRenderer->pBackBuffers[pRenderer->BackBufferIndex]);
    __m128i* pEndBufferSSE = (__m128i*)((uint32_t*)pStartBufferSSE + pRenderer->Height * pRenderer->Pitch);
    while (pStartBufferSSE < pEndBufferSSE)
    {
        *pStartBufferSSE++ = _mm_set1_epi32(argb);
    }

#else
    uint32_t* pBuffer = pRenderer->pBackBuffers[pRenderer->BackBufferIndex];
    uint32_t* pEndBuffer = pBuffer + pRenderer->Height * pRenderer->Pitch;
    while (pBuffer < pEndBuffer)
    {
        *pBuffer++ = argb;
    }
#endif // USE_SSE
}

void __stdcall DrawHorizontalLine(IRenderer* pThis, const int x, const int y, const uint_t width, const uint32_t argb)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    ASSERT(width > 0, "width is 0");

    Renderer* pRenderer = (Renderer*)pThis;

    const int startY = MIN(MAX(y, 0), (int)pRenderer->Height - 1);
    const int startX = MIN(MAX(x, 0), (int)pRenderer->Width - 1);
    const int endX = MIN(MAX(x + (int)width, 0), (int)pRenderer->Width - 1);

    uint32_t* pStartBuffer = pRenderer->pBackBuffers[pRenderer->BackBufferIndex] + startY * pRenderer->Pitch + startX;
    uint32_t* pEndBuffer = pStartBuffer + endX - startX;

#if USE_SSE
    if (endX & 1)
    {
        *pEndBuffer-- = argb;
    }

    if (endX & 2)
    {
        *pEndBuffer-- = argb;
    }

    __m128i* pStartBufferSSE = (__m128i*)pStartBuffer;
    while (pStartBufferSSE <  (__m128i*)pEndBuffer)
    {
        *pStartBufferSSE++ = _mm_set1_epi32(argb);
    }
#else
    while (pBuffer < pEndBuffer)
    {
        *pBuffer++ = argb;
    }

#endif // USE_SSE
}

void __stdcall DrawVerticalLine(IRenderer* pThis, const int x, const int y, const uint_t height, const uint32_t argb)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    ASSERT(height > 0, "height is 0");

    Renderer* pRenderer = (Renderer*)pThis;

    const int startX = MIN(MAX(x, 0), (int)pRenderer->Width - 1);
    const int startY = MIN(MAX(y, 0), (int)pRenderer->Height - 1);
    const int endY = MIN(MAX(y + (int)height, 0), (int)pRenderer->Height - 1);

    uint32_t* pStartBuffer = pRenderer->pBackBuffers[pRenderer->BackBufferIndex] + startY * pRenderer->Pitch + startX;
    uint32_t* pEndBuffer = pStartBuffer + (endY - startY) * pRenderer->Pitch;
    while (pStartBuffer < pEndBuffer)
    {
        *pStartBuffer = argb;
        pStartBuffer += pRenderer->Pitch;
    }
}

void __stdcall DrawLine(IRenderer* pThis, const int x0, const int y0, const int x1, const int y1, const uint_t argb)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

    int startX = x0;
    int startY = y0;
    int endX = x1;
    int endY = y1;
    const bool bInsideWindow = ClipLine(0, 0, pRenderer->Width - 1, pRenderer->Height - 1, &startX, &startY, &endX, &endY);
    if (!bInsideWindow)
    {
        return;
    }

    const int width = endX - startX;
    const int height = endY - startY;
    const bool bGradual = (ABS(width) >= ABS(height));

    const int dx = (width >= 0) ? 1 : -1;
    const int dy = (height > 0) ? 1 : -1;

    const int dw = dx * width;
    const int dh = dy * height;

    int discriminant =              bGradual ?  2 * dh - dw     : 2 * dw - dh;
    const int NEXT_DISCRIMINANT0 =  bGradual ?  2 * dh          : 2 * dw;
    const int NEXT_DISCRIMINANT1 =  bGradual ?  2 * (dh - dw)   : 2 * (dw - dh);

    uint32_t* pBuffer = pRenderer->pBackBuffers[pRenderer->BackBufferIndex] + startY * pRenderer->Pitch + startX;
    uint32_t* pEndBuffer = pRenderer->pBackBuffers[pRenderer->BackBufferIndex] + endY * pRenderer->Pitch + endX;

    if (bGradual)
    {
        while (pBuffer != pEndBuffer)
        {
            *pBuffer = argb;

            if (discriminant < 0)
            {
                discriminant += NEXT_DISCRIMINANT0;
            }
            else
            {
                discriminant += NEXT_DISCRIMINANT1;
                pBuffer += (int)pRenderer->Pitch * dy;
            }

            pBuffer += dx;
        }
    }
    else
    {
        while (pBuffer != pEndBuffer)
        {
            *pBuffer = argb;

            if (discriminant < 0)
            {
                discriminant += NEXT_DISCRIMINANT0;
            }
            else
            {
                discriminant += NEXT_DISCRIMINANT1;
                pBuffer += dx;
            }

            pBuffer += (int)pRenderer->Pitch * dy;
        }
    }
}

void __stdcall DrawBitmap(IRenderer* pThis, const int x, const int y, const uint_t width, const uint_t height, const void* pBitmap)
{
    ASSERT(pThis != NULL, "pThis is NULL");
    ASSERT(pBitmap != NULL, "pBitmap is NULL");

    Renderer* pRenderer = (Renderer*)pThis;

    const uint_t startX = MIN(MAX(x, 0), (int)pRenderer->Width - 1);
    const uint_t startY = MIN(MAX(y, 0), (int)pRenderer->Height - 1);
    uint_t clippedWidth = width - (x - startX);
    uint_t clippedHeight = height - (y - startY);

    const uint_t endX = MIN(startX + clippedWidth, pRenderer->Width - 1);
    const uint_t endY = MIN(startY + clippedHeight, pRenderer->Height - 1);
    clippedWidth = clippedWidth - ((startX + clippedWidth) - endX);
    clippedHeight = clippedHeight - ((startY + clippedHeight) - endY);
    
#if USE_SSE
    const uint32_t* pPixel = (const uint32_t*)pBitmap;

    uint32_t* pBuffer = pRenderer->pBackBuffers[pRenderer->BackBufferIndex] + startY * pRenderer->Pitch + startX;
    //const size_t mask = ((clippedWidth & 1) == 1) + ((clippedWidth & 2) == 2);
    const size_t mask = (clippedWidth & 3);
    const size_t loopCount = (clippedWidth - mask) / 4;
    for (size_t i = 0; i < clippedHeight; ++i)
    {
        for (size_t k = 0; k < mask; ++k)
        {
            *pBuffer++ = *pPixel++;
        }

        __m128i* pBufferSSE = (__m128i*)pBuffer;
        const __m128i* pPixelSSE = (const __m128i*)pPixel;
        for (size_t k = 0; k < loopCount; ++k)
        {
            *pBufferSSE++ = *pPixelSSE++;
        }

        pBuffer -= mask;
        pBuffer += pRenderer->Pitch;
        pPixel = (const uint32_t*)pPixelSSE;
    }
#endif
}

void __stdcall SetMaxFps(IRenderer* pThis, const uint_t fps)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;
    pRenderer->MaxFps = (fps == 0) ? UINT_MAX : fps;
    pRenderer->TicksPerFrame = (fps == 0) ? 0.0f : 1000.0f / (float)fps / 1000.0f;
}

uint_t __stdcall GetFps(const IRenderer* pThis)
{
    ASSERT(pThis != NULL, "pThis is NULL");

    Renderer* pRenderer = (Renderer*)pThis;
    return pRenderer->Fps;
}

void __stdcall CreateDllInstance(void** ppOutInstance)
{
    ASSERT(ppOutInstance != NULL, "ppOutInstance is NULL");

    Renderer* pRenderer = (Renderer*)malloc(sizeof(Renderer));
    ASSERT(pRenderer != NULL, "Failed to malloc");

    pRenderer->Vtbl = s_vtbl;
    pRenderer->RefCount = 1;

    *ppOutInstance = pRenderer;
}