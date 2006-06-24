/* 
 *	Copyright (C) 2003-2005 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "StdAfx.h"
#include <atlbase.h>
#include "..\..\..\DSUtil\MediaTypes.h"
#include <initguid.h>
#include "qtdec.h"

using namespace QT;

static DWORD DWSWAP(DWORD dw) {return(((dw&0xff)<<24)|((dw&0xff00)<<8)|((dw&0xff0000)>>8)|((dw&0xff000000)>>24));}

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    { L"Input",             // Pins string name
      FALSE,                // Is it rendered
      FALSE,                // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      countof(sudPinTypesIn),	// Number of types
      sudPinTypesIn			// Pin information
    },
    { L"Output",            // Pins string name
      FALSE,                // Is it rendered
      TRUE,                 // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      countof(sudPinTypesOut),	// Number of types
      sudPinTypesOut		// Pin information
    }
};

const AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_QTDec,			// Filter CLSID
    L"QuickTime Decoder",	// String name
    MERIT_DO_NOT_USE/*MERIT_NORMAL*/,			// Filter merit
    countof(sudpPins),	// Number of pins
    sudpPins                // Pin information
};

CFactoryTemplate g_Templates[] =
{
    { L"QuickTime Decoder"
    , &CLSID_QTDec
    , CQTDec::CreateInstance
    , NULL
    , &sudFilter }
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    return DllEntryPoint((HINSTANCE)hModule, dwReason, 0); // "DllMain" of the dshow baseclasses;
}

//
// CQTDec
//

CUnknown* WINAPI CQTDec::CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
    CUnknown* punk = new CQTDec(lpunk, phr);
    if(punk == NULL) *phr = E_OUTOFMEMORY;
	return punk;
}

#endif

CQTDec::CQTDec(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CQTDec"), lpunk, CLSID_QTDec)
	, m_fQtInitialized(false)
	, m_pImageGWorld(NULL)
	, m_hImageDesc(NULL)
	, m_cinst(NULL)
{
	if(phr) *phr = S_OK;

	m_fQtInitialized = false;
	if(InitializeQTML(0) != 0) {if(phr) *phr = E_FAIL; return;}
//	if(EnterMovies() != 0) {TerminateQTML(); if(phr) *phr = E_FAIL; return;}
	m_fQtInitialized = true;
}

CQTDec::~CQTDec()
{
	if(m_cinst)
		CloseComponent(m_cinst),
		m_cinst = NULL;

	if(m_hImageDesc)
		DisposeHandle((Handle)m_hImageDesc), 
		m_hImageDesc = NULL;

	FreeGWorld(m_pImageGWorld);

	if(m_fQtInitialized)
	{
//		ExitMovies();
		TerminateQTML();
	}
}

//

GWorldPtr CQTDec::MakeGWorld()
{
	if(!m_pOutput->IsConnected())
		return NULL;

	const CMediaType& mt = m_pOutput->CurrentMediaType();
	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.pbFormat;
	BITMAPINFOHEADER& bih = vih->bmiHeader;

	GWorldPtr pImageWorld = NULL;
	Rect rect = {0, 0, (short)abs(bih.biHeight), (short)bih.biWidth};

	OSType PixelFormat = 
		mt.subtype == MEDIASUBTYPE_YUY2 ? kYUVSPixelFormat : // 'yuvs'
		mt.subtype == MEDIASUBTYPE_UYVY ? k2vuyPixelFormat : // '2vuy'
		mt.subtype == MEDIASUBTYPE_RGB32 ? k32BGRAPixelFormat : // 'BGRA'
		mt.subtype == MEDIASUBTYPE_RGB565 ? k16LE565PixelFormat : // 'L565'
		mt.subtype == MEDIASUBTYPE_RGB555 ? k16LE555PixelFormat : // 'L555'
		0;

	if(!PixelFormat || noErr != QTNewGWorld(&pImageWorld, PixelFormat, &rect, NULL, NULL, 0))
		pImageWorld = NULL;

	if(pImageWorld)
        LockPixels(GetGWorldPixMap(pImageWorld));

	return pImageWorld;
}

void CQTDec::FreeGWorld(GWorldPtr& pImageGWorld)
{
	if(pImageGWorld)
	{
        UnlockPixels(GetGWorldPixMap(pImageGWorld));
		DisposeGWorld(pImageGWorld);
		pImageGWorld = NULL;
	}
}

bool CQTDec::InitComponent()
{
	if(m_cinst)
		CloseComponent(m_cinst),
		m_cinst = NULL;

	if(m_hImageDesc)
		DisposeHandle((Handle)m_hImageDesc), 
		m_hImageDesc = NULL;

	if(!m_pInput->IsConnected())
		return NULL;

	BITMAPINFOHEADER& bih = ((VIDEOINFOHEADER*)m_pInput->CurrentMediaType().pbFormat)->bmiHeader;

	ComponentDescription cd = {decompressorComponentType, DWSWAP(bih.biCompression), 0, 0, 0};
	Component c = FindNextComponent(0, &cd);
	if(!c) return(false);

	m_cinst = OpenComponent(c);
	if(!m_cinst) return(false);

	ComponentResult cres;

    ImageSubCodecDecompressCapabilities icap;
    memset(&icap, 0, sizeof(icap));
    cres = ImageCodecInitialize(m_cinst, &icap);

    CodecInfo cinfo;
    memset(&cinfo, 0, sizeof(cinfo));
    cres = ImageCodecGetCodecInfo(m_cinst, &cinfo);

	m_hImageDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	if(!m_hImageDesc)
		return(false);

    (**m_hImageDesc).idSize = sizeof(ImageDescription);
	(**m_hImageDesc).cType = DWSWAP(bih.biCompression);
    (**m_hImageDesc).temporalQuality = 0;
    (**m_hImageDesc).spatialQuality = codecNormalQuality;
	(**m_hImageDesc).width = (short)bih.biWidth;
    (**m_hImageDesc).height = (short)abs(bih.biHeight);
    (**m_hImageDesc).hRes = 72 << 16;
    (**m_hImageDesc).vRes = 72 << 16;
    (**m_hImageDesc).frameCount = 1; // ?
	(**m_hImageDesc).depth = bih.biBitCount; // should be 24 for unknown/compressed types
    (**m_hImageDesc).clutID = -1;

	memset(&m_cdpar, 0, sizeof(m_cdpar));
    m_cdpar.imageDescription = m_hImageDesc;
    m_cdpar.startLine = 0;
    m_cdpar.stopLine = (**m_hImageDesc).height;
    m_cdpar.frameNumber = 1;
    m_cdpar.matrixFlags = 0;
    m_cdpar.matrixType = 0;
    m_cdpar.matrix = 0;
    m_cdpar.capabilities = &m_ccap;
	memset(&m_ccap, 0, sizeof(m_ccap));
    m_cdpar.accuracy = codecNormalQuality;
    m_cdpar.port = m_pImageGWorld;
	Rect rect = {0, 0, (**m_hImageDesc).height, (**m_hImageDesc).width};
	m_cdpar.srcRect = rect;
    m_cdpar.transferMode = srcCopy;
    m_cdpar.dstPixMap = **GetGWorldPixMap(m_pImageGWorld);

    cres = ImageCodecPreDecompress(m_cinst, &m_cdpar);

	return(true);
}

//

HRESULT CQTDec::BreakConnect(PIN_DIRECTION dir)
{
	if(dir == PINDIR_INPUT)
	{
		m_mts.RemoveAll();
	}
	else if(dir == PINDIR_OUTPUT)
	{
		FreeGWorld(m_pImageGWorld);
	}

	return __super::BreakConnect(dir);
}

HRESULT CQTDec::CompleteConnect(PIN_DIRECTION dir, IPin* pReceivePin)
{
	if(dir == PINDIR_INPUT)
	{
		m_mts.RemoveAll();

		VIDEOINFOHEADER* vihin = (VIDEOINFOHEADER*)m_pInput->CurrentMediaType().pbFormat;
		BITMAPINFOHEADER& bihin = vihin->bmiHeader;

		CMediaType mt;
		mt.majortype = MEDIATYPE_Video;
		mt.subtype = MEDIASUBTYPE_None;
		mt.formattype = FORMAT_VideoInfo;
		mt.bFixedSizeSamples = TRUE;
		mt.bTemporalCompression = FALSE;
		mt.lSampleSize = 0;
		mt.pUnk = NULL;

		VIDEOINFOHEADER vih;
		memset(&vih, 0, sizeof(vih));
		vih.AvgTimePerFrame = vihin->AvgTimePerFrame;
		vih.rcSource = vihin->rcSource;
		vih.rcTarget = vihin->rcTarget;
		vih.dwBitRate = vihin->dwBitRate;
		vih.dwBitErrorRate = vihin->dwBitErrorRate;

		BITMAPINFOHEADER& bih = vih.bmiHeader;
		bih.biSize = sizeof(bih);
		bih.biWidth = bihin.biWidth;
		bih.biHeight = abs(bihin.biHeight);
		bih.biPlanes = 1;
		bih.biXPelsPerMeter = bih.biYPelsPerMeter = 0;
		bih.biClrUsed = bih.biClrImportant = 0;

//		if(fRGB32) // always can decompress to (?)
		{
			VIDEOINFOHEADER* vihout = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
			memcpy(vihout, &vih, sizeof(vih));

			BITMAPINFOHEADER& bihout = vihout->bmiHeader;
			bihout.biBitCount = 32;
			bihout.biSizeImage = bihout.biWidth*abs(bihout.biHeight)*bihout.biBitCount>>3;

			mt.subtype = MEDIASUBTYPE_RGB32;

			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biCompression = BI_BITFIELDS;
			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biHeight = -bih.biHeight;
			CorrectMediaType(&mt);
			m_mts.Add(mt);

			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biCompression = BI_RGB;
			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biHeight = bih.biHeight;
			CorrectMediaType(&mt);
			m_mts.Add(mt);
		}

//		if(fRGB16) // always can decompress to (?)
		{
			VIDEOINFOHEADER* vihout = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
			memcpy(vihout, &vih, sizeof(vih));

			BITMAPINFOHEADER& bihout = vihout->bmiHeader;
			bihout.biBitCount = 16;
			bihout.biSizeImage = bihout.biWidth*abs(bihout.biHeight)*bihout.biBitCount>>3;

			mt.subtype = MEDIASUBTYPE_RGB565;

			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biCompression = BI_BITFIELDS;
			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biHeight = -bih.biHeight;
			CorrectMediaType(&mt);
			m_mts.Add(mt);

			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biCompression = BI_RGB;
			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biHeight = bih.biHeight;
			CorrectMediaType(&mt);
			m_mts.Add(mt);

			mt.subtype = MEDIASUBTYPE_RGB555;

			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biCompression = BI_BITFIELDS;
			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biHeight = -bih.biHeight;
			CorrectMediaType(&mt);
			m_mts.Add(mt);

			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biCompression = BI_RGB;
			((VIDEOINFOHEADER*)mt.pbFormat)->bmiHeader.biHeight = bih.biHeight;
			CorrectMediaType(&mt);
			m_mts.Add(mt);
		}
	}
	else if(dir == PINDIR_OUTPUT)
	{
		FreeGWorld(m_pImageGWorld);
		m_pImageGWorld = MakeGWorld();
	}

	return __super::CompleteConnect(dir, pReceivePin);
}

HRESULT CQTDec::Transform(IMediaSample* pSample, IMediaSample* pOutSample)
{
	HRESULT hr;

	BYTE* pIn = NULL;
	if(FAILED(hr = pSample->GetPointer(&pIn))) return hr;
	long len = pSample->GetActualDataLength();
	if(len <= 0) return S_FALSE;

	BYTE* pOut = NULL;
	if(FAILED(hr = pOutSample->GetPointer(&pOut))) return hr;
	int size = pOutSample->GetSize();

	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_pOutput->CurrentMediaType().pbFormat;
	BITMAPINFOHEADER& bih = vih->bmiHeader;

	bool fInitialized = true;

	if(m_cinst)
	{
		AM_MEDIA_TYPE* pmt = NULL;
		if(S_OK == pSample->GetMediaType((AM_MEDIA_TYPE**)&pmt) && pmt)
		{
			CMediaType mt(*pmt);
			DeleteMediaType(pmt), pmt = NULL;

			if(mt != m_pInput->CurrentMediaType())
			{
				m_pInput->SetMediaType(&mt);
				fInitialized = InitComponent();
			}
		}
	}
	else
	{
		fInitialized = InitComponent();
	}

	if(!fInitialized)
		return E_FAIL;

	m_cdpar.data = (Ptr)pIn;
	m_cdpar.bufferSize = len;
	(**m_cdpar.imageDescription).dataSize = len;
    ComponentResult cres = ImageCodecBandDecompress(m_cinst, &m_cdpar);
    m_cdpar.frameNumber++;

	if(cres == noErr)
	{
		PixMapHandle hPixMap = GetGWorldPixMap(m_pImageGWorld);
		Ptr pPixels = GetPixBaseAddr(hPixMap);
		long theRowBytes = QTGetPixMapHandleRowBytes(hPixMap);

		DWORD pitch = bih.biWidth*bih.biBitCount>>3;

		for(int i = 0, h = abs(bih.biHeight); i < h; i++)
		{
			memcpy(pOut + i*pitch, (BYTE*)pPixels + i*theRowBytes, min(pitch, theRowBytes));
		}
	}

	return S_OK;
}

HRESULT CQTDec::CheckInputType(const CMediaType* mtIn)
{
	if(mtIn->majortype != MEDIATYPE_Video || mtIn->formattype != FORMAT_VideoInfo)
		return VFW_E_TYPE_NOT_ACCEPTED;
	
	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mtIn->pbFormat;
	BITMAPINFOHEADER& bih = vih->bmiHeader;

	OSErr err;
	ComponentInstance ci;
	if(noErr == (err = OpenADefaultComponent('imdc', DWSWAP(bih.biCompression), &ci)))
	{
		err = CloseComponent(ci);
		return S_OK;
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CQTDec::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return S_OK;
	// TODO
	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CQTDec::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	CComPtr<IMemAllocator> pAllocatorIn;
	m_pInput->GetAllocator(&pAllocatorIn);
	if(!pAllocatorIn) return E_UNEXPECTED;

	pAllocatorIn->GetProperties(pProperties);

	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_pInput->CurrentMediaType().pbFormat;
	BITMAPINFOHEADER& bih = vih->bmiHeader;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = bih.biWidth*abs(bih.biHeight)*4; // TODO
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) 
		return hr;

    return(pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		? E_FAIL
		: NOERROR);
}

HRESULT CQTDec::GetMediaType(int iPosition, CMediaType* pmt)
{
    if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

	if(iPosition < 0) return E_INVALIDARG;
	if(iPosition >= m_mts.GetCount()) return VFW_S_NO_MORE_ITEMS;

	*pmt = m_mts[iPosition];

	return S_OK;
}
