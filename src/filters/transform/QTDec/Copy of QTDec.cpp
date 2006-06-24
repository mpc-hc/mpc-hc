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
	, m_hImageDescription(NULL)
	, m_pImageGWorld(NULL)
{
	if(phr) *phr = S_OK;

	m_fQtInitialized = false;
	if(InitializeQTML(0) != 0) {if(phr) *phr = E_FAIL; return;}
//	if(EnterMovies() != 0) {TerminateQTML(); if(phr) *phr = E_FAIL; return;}
	m_fQtInitialized = true;
}

CQTDec::~CQTDec()
{
	FreeImageDescription(m_hImageDescription);
	FreeGWorld(m_pImageGWorld);

	if(m_fQtInitialized)
	{
//		ExitMovies();
		TerminateQTML();
	}
}

//

bool CQTDec::CanDecompress(OSType fourcc, bool& fYUY2, bool& fUYVY)
{
	fYUY2 = fUYVY = false;

	ComponentDescription cd = {decompressorComponentType, fourcc, 0, 0, cmpIsMissing};

	if(Component decompressor = FindNextComponent(0, &cd))
	{
		do
		{
			OSErr err;
			Handle cpix = NULL;
			if(noErr == (err = GetComponentPublicResource(decompressor, FOUR_CHAR_CODE('cpix'), 1, &cpix)))
			{
				int cpixFormatCount = GetHandleSize(cpix) / sizeof(OSType);
				for(int i = 0; i < cpixFormatCount; i++)
				{
					switch((*(OSType**)cpix)[i])
					{
					case 'yuvs': fYUY2 = true; break; // yuy2
					case '2vuy': fUYVY = true; break; // uyvy
					default: break;
					}
				}

				DisposeHandle(cpix);
			}

			decompressor = FindNextComponent(decompressor, &cd);
		}
		while(decompressor && !(fYUY2 && fUYVY));

		return(true);
	}

	return(false);
}

ImageDescriptionHandle CQTDec::MakeImageDescription()
{
	if(!m_pInput->IsConnected())
		return NULL;

	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_pInput->CurrentMediaType().pbFormat;
	BITMAPINFOHEADER& bih = vih->bmiHeader;

	ImageDescriptionHandle h = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));

    if(h)
	{
        (**h).idSize = sizeof(ImageDescription);
		(**h).cType = DWSWAP(bih.biCompression);
        (**h).temporalQuality = 0;
        (**h).spatialQuality = codecNormalQuality;
		(**h).width = (short)bih.biWidth;
        (**h).height = (short)abs(bih.biHeight);
        (**h).hRes = 72 << 16;
        (**h).vRes = 72 << 16;
        (**h).frameCount = 1; // ?
		(**h).depth = bih.biBitCount; // should be 24 for unknown/compressed types
        (**h).clutID = -1;
    }

    return h;
}

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

void CQTDec::FreeImageDescription(ImageDescriptionHandle& hImageDescription)
{
	if(hImageDescription)
	{
		DisposeHandle((Handle)hImageDescription);
		hImageDescription = NULL;
	}
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

//

HRESULT CQTDec::BreakConnect(PIN_DIRECTION dir)
{
	if(dir == PINDIR_INPUT)
	{
		m_mts.RemoveAll();
	}
	else if(dir == PINDIR_OUTPUT)
	{
		FreeImageDescription(m_hImageDescription);
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

		bool fYUY2 = false, fUYVY = false;
		if(CanDecompress(DWSWAP(bihin.biCompression), fYUY2, fUYVY))
		{
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
/*
			if(fYUY2 || fUYVY)
			{
				VIDEOINFOHEADER* vihout = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
				memcpy(vihout, &vih, sizeof(vih));

				BITMAPINFOHEADER& bihout = vihout->bmiHeader;
				bihout.biBitCount = 16;
				bihout.biSizeImage = bihout.biWidth*abs(bihout.biHeight)*bihout.biBitCount>>3;

				if(fYUY2)
				{
					mt.subtype = MEDIASUBTYPE_YUY2;
					bihout.biCompression = '2YUY';
					m_mts.Add(mt);
				}

				if(fUYVY)
				{
					mt.subtype = MEDIASUBTYPE_UYVY;
					bihout.biCompression = 'YVYU';
                    m_mts.Add(mt);
				}
			}
*/
//			if(fRGB32) // always can decompress to (?)
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

//			if(fRGB16) // always can decompress to (?)
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
	}
	else if(dir == PINDIR_OUTPUT)
	{
		FreeGWorld(m_pImageGWorld);
		m_pImageGWorld = MakeGWorld();
	}

	return __super::CompleteConnect(dir, pReceivePin);
}

HRESULT CQTDec::StartStreaming()
{
	FreeImageDescription(m_hImageDescription);
	m_hImageDescription = MakeImageDescription();

	if(!m_hImageDescription)
		return E_FAIL;
/*
    Rect theSrcBounds = {0, 0};
    Rect theDestBounds;
    GetPortBounds((CGrafPtr)m_pImageGWorld, &theDestBounds);
    theSrcBounds.right  = (*m_hImageDescription)->width;
    theSrcBounds.bottom = (*m_hImageDescription)->height;

    MatrixRecord rMatrix;
    RectMatrix(&rMatrix, &theSrcBounds, &theDestBounds);
*/
    m_outSeqID = 0;

	OSErr err;

	DecompressorComponent decompressor = NULL; 

	err = FindCodec((*m_hImageDescription)->cType, anyCodec, NULL, &decompressor);

	CodecInfo info;
	err = GetCodecInfo(&info, (*m_hImageDescription)->cType, decompressor);

    err = DecompressSequenceBeginS(
		&m_outSeqID, m_hImageDescription, 
		NULL, 0, 
		m_pImageGWorld, NULL,
		NULL, NULL/*&rMatrix*/, srcCopy, (RgnHandle)NULL, 
		codecFlagUseImageBuffer, codecNormalQuality, anyCodec);

	if(noErr != err || m_outSeqID == 0)
		return E_FAIL;

	return __super::StartStreaming();
}


HRESULT CQTDec::StopStreaming()
{
	if(m_outSeqID)
	{
        OSErr err = CDSequenceEnd(m_outSeqID);
        m_outSeqID = 0;
    }

	FreeImageDescription(m_hImageDescription);

	return __super::StopStreaming();
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

	OSErr err;

	{
		AM_MEDIA_TYPE* pmt = NULL;
		if(S_OK == pSample->GetMediaType((AM_MEDIA_TYPE**)&pmt) && pmt)
		{
			CMediaType mt(*pmt);
			DeleteMediaType(pmt), pmt = NULL;

			if(mt != m_pInput->CurrentMediaType())
			{
				StopStreaming();
				m_pInput->SetMediaType(&mt);
				StartStreaming();
			}
		}
	}

	CodecFlags inf = codecFlagNoScreenUpdate, outf = 0;

	err = DecompressSequenceFrameWhen(m_outSeqID, (Ptr)pIn, len, inf, &outf, NULL, NULL);
//	if(err == noErr)
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

/*
	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_pOutput->CurrentMediaType().pbFormat;
	BITMAPINFOHEADER& bih = vih->bmiHeader;

	Rect						myRect = {0, 0, (short)abs(bih.biHeight), (short)bih.biWidth};
//	GraphicsImportComponent		myImporter = NULL;
//	ComponentInstance			myComponent = NULL;
	GWorldPtr					myImageWorld = NULL;
	PixMapHandle				myPixMap = NULL;
	ImageDescriptionHandle		myDesc = NULL;
	Handle						myHandle = NULL;
	OSErr						myErr = noErr;

	myErr = QTNewGWorld(&myImageWorld, 0, &myRect, NULL, NULL, noNewDevice|keepLocal);
//	myErr = QTNewGWorldFromPtr(&myImageWorld, k422YpCbCr8PixelFormat, &myRect, NULL, NULL, 0, pOut, bih.biWidth*bih.biBitCount>>3);
	if(myErr == noErr)
	{
		myPixMap = GetGWorldPixMap(myImageWorld);

		if(myDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription)))
		{
			ImageDescription& id = *(ImageDescription*)*myDesc;
			memset(&id, 0, sizeof(id));
			id.idSize = sizeof(id);
			id.cType = DWSWAP(((VIDEOINFOHEADER*)m_pInput->CurrentMediaType().pbFormat)->bmiHeader.biCompression);
			id.temporalQuality = codecNormalQuality; // ?
			id.spatialQuality = codecNormalQuality; // ?
			id.width = (short)bih.biWidth;
			id.height = (short)bih.biHeight;
			id.depth = ((VIDEOINFOHEADER*)m_pInput->CurrentMediaType().pbFormat)->bmiHeader.biBitCount;
			id.dataSize = len;
			id.hRes = 0;
			id.vRes = 0;
			id.frameCount = 1;
			id.clutID = -1; // TODO

			myErr = DecompressImage((Ptr)pIn, myDesc, myPixMap, NULL, &myRect, srcCopy, NULL);

			DisposeHandle((Handle)myDesc);
		}
	}

	if(myImageWorld) DisposeGWorld(myImageWorld);
*/
/*
	// TODO
	pOutSample->SetActualDataLength(0);
	return S_FALSE;
*/

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
