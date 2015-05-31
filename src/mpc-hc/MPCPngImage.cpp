#include "stdafx.h"
#include "MPCPngImage.h"


//////////////////////////////////////////////////////////////////////
// CPngImage

CImage* CMPCPngImage::m_pImage;

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

BOOL CMPCPngImage::Load(UINT uiResID, HINSTANCE hinstRes)
{
    return Load(MAKEINTRESOURCE(uiResID), hinstRes);
}

BOOL CMPCPngImage::Load(LPCTSTR lpszResourceName, HINSTANCE hinstRes)
{
    if (hinstRes == nullptr) {
        hinstRes = AfxFindResourceHandle(lpszResourceName, _T("PNG"));
    }

    HRSRC hRsrc = ::FindResource(hinstRes, lpszResourceName, _T("PNG"));
    if (hRsrc == nullptr) {
        // Fallback to the instance handle
        hinstRes = AfxGetInstanceHandle();
        hRsrc = ::FindResource(hinstRes, lpszResourceName, _T("PNG"));
        if (hRsrc == nullptr) {
            return FALSE;
        }
    }

    HGLOBAL hGlobal = LoadResource(hinstRes, hRsrc);
    if (hGlobal == nullptr) {
        return FALSE;
    }

    LPVOID lpBuffer = ::LockResource(hGlobal);
    if (lpBuffer == nullptr) {
        FreeResource(hGlobal);
        return FALSE;
    }

    BOOL bRes = LoadFromBuffer((LPBYTE) lpBuffer, (UINT) ::SizeofResource(hinstRes, hRsrc));

    UnlockResource(hGlobal);
    FreeResource(hGlobal);

    return bRes;
}
//*******************************************************************************
BOOL CMPCPngImage::LoadFromFile(LPCTSTR lpszPath)
{
    BOOL bRes = FALSE;

    if (m_pImage == nullptr) {
        m_pImage = DEBUG_NEW CImage;
        ENSURE(m_pImage != nullptr);
    }

    if (m_pImage->Load(lpszPath) == S_OK) {
        bRes = Attach(m_pImage->Detach());
    }

    return bRes;
}
//*******************************************************************************
BOOL CMPCPngImage::LoadFromBuffer(const LPBYTE lpBuffer, UINT uiSize)
{
    ASSERT(lpBuffer != nullptr);

    HGLOBAL hRes = ::GlobalAlloc(GMEM_MOVEABLE, uiSize);
    if (hRes == nullptr) {
        return FALSE;
    }

    IStream* pStream = nullptr;
    LPVOID lpResBuffer = ::GlobalLock(hRes);
    ASSERT(lpResBuffer != nullptr);

    memcpy(lpResBuffer, lpBuffer, uiSize);

    HRESULT hResult = ::CreateStreamOnHGlobal(hRes, FALSE, &pStream);

    if (hResult != S_OK) {
        return FALSE;
    }

    if (m_pImage == nullptr) {
        m_pImage = DEBUG_NEW CImage;
        ENSURE(m_pImage != nullptr);
    }

    m_pImage->Load(pStream);
    pStream->Release();

    BOOL bRes = Attach(m_pImage->Detach());

    return bRes;
}

CSize CMPCPngImage::GetSize()
{
    CSize size;
    BITMAP bm;
    if (GetBitmap(&bm)) {
        size.SetSize(bm.bmWidth, bm.bmHeight);
    }
    return size;
}
