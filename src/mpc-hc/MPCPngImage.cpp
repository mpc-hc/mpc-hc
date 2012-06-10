#include "stdafx.h"
#include "MPCPngImage.h"


//////////////////////////////////////////////////////////////////////
// CPngImage

CImage* CMPCPngImage::m_pImage;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPCPngImage::CMPCPngImage()
{
}

CMPCPngImage::~CMPCPngImage()
{
}

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

BOOL CMPCPngImage::Load(UINT uiResID, HINSTANCE hinstRes)
{
	return Load(MAKEINTRESOURCE(uiResID), hinstRes);
}

BOOL CMPCPngImage::Load(LPCTSTR lpszResourceName, HINSTANCE hinstRes)
{
	if (hinstRes == NULL)
	{
		hinstRes = AfxFindResourceHandle(lpszResourceName, _T("PNG"));
	}

	HRSRC hRsrc = ::FindResource(hinstRes, lpszResourceName, _T("PNG"));
	if (hRsrc == NULL)
	{
		return FALSE;
	}

	HGLOBAL hGlobal = LoadResource(hinstRes, hRsrc);
	if (hGlobal == NULL)
	{
		return FALSE;
	}

	LPVOID lpBuffer = ::LockResource(hGlobal);
	if (lpBuffer == NULL)
	{
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

	if (m_pImage == NULL)
	{
		m_pImage = new CImage;
		ENSURE(m_pImage != NULL);
	}

	if (m_pImage->Load(lpszPath) == S_OK)
	{
		bRes = Attach(m_pImage->Detach());
	}

	return bRes;
}
//*******************************************************************************
BOOL CMPCPngImage::LoadFromBuffer(LPBYTE lpBuffer, UINT uiSize)
{
	ASSERT(lpBuffer != NULL);

	HGLOBAL hRes = ::GlobalAlloc(GMEM_MOVEABLE, uiSize);
	if (hRes == NULL)
	{
		return FALSE;
	}

	IStream* pStream = NULL;
	LPVOID lpResBuffer = ::GlobalLock(hRes);
	ASSERT (lpResBuffer != NULL);

	memcpy(lpResBuffer, lpBuffer, uiSize);

	HRESULT hResult = ::CreateStreamOnHGlobal(hRes, FALSE, &pStream);

	if (hResult != S_OK)
	{
		return FALSE;
	}

	if (m_pImage == NULL)
	{
		m_pImage = new CImage;
		ENSURE(m_pImage != NULL);
	}

	m_pImage->Load(pStream);
	pStream->Release();

	BOOL bRes = Attach(m_pImage->Detach());

	return bRes;
}
