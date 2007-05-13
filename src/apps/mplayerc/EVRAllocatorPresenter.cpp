/* 
 *	Copyright (C) 2007 Casimir666
 *	http://tibrium.neuf.fr
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

#include "stdafx.h"

#include "stdafx.h"
#include "mplayerc.h"
#include <atlbase.h>
#include <atlcoll.h>
#include "..\..\DSUtil\DSUtil.h"

#include <Videoacc.h>

#include <initguid.h>
#include "..\..\SubPic\ISubPic.h"
#include "EVRAllocatorPresenter.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <Vmr9.h>
#include <evr.h>
#include <mfapi.h>	// API Media Foundation
#include "..\..\SubPic\DX9SubPic.h"
#include "IQTVideoSurface.h"
#include "..\..\..\include\moreuuids.h"

#include "IPinHook.h"

#include "PixelShaderCompiler.h"
#include "MainFrm.h"

#include "AllocatorCommon.h"

// dxva.dll
typedef HRESULT (__stdcall *PTR_DXVA2CreateDirect3DDeviceManager9)(UINT* pResetToken, IDirect3DDeviceManager9** ppDeviceManager);

// mf.dll
typedef HRESULT (__stdcall *PTR_MFCreatePresentationClock)(IMFPresentationClock** ppPresentationClock);

// evr.dll
typedef HRESULT (__stdcall *PTR_MFCreateDXSurfaceBuffer)(REFIID riid, IUnknown* punkSurface, BOOL fBottomUpWhenLinear, IMFMediaBuffer** ppBuffer);

// mfplat.dll
typedef HRESULT (__stdcall *PTR_MFCreateSample)(IMFSample** ppIMFSample);
typedef HRESULT (__stdcall *PTR_MFCreateSystemTimeSource)(IMFPresentationTimeSource** ppSystemTimeSource);

// AVRT.dll
typedef HANDLE  (__stdcall *PTR_AvSetMmThreadCharacteristicsW)(LPCWSTR TaskName, LPDWORD TaskIndex);
typedef BOOL	(__stdcall *PTR_AvSetMmThreadPriority)(HANDLE AvrtHandle, AVRT_PRIORITY Priority);
typedef BOOL	(__stdcall *PTR_AvRevertMmThreadCharacteristics)(HANDLE AvrtHandle);


class CEVRAllocatorPresenter : 
	public CDX9AllocatorPresenter,
	public IMFVideoPresenter,
	public IMFTopologyServiceLookup,
	public IMFTopologyServiceLookupClient,
	public IMFVideoDeviceID,
	public IMFGetService,
	public IMFAsyncCallback
/*	public IMFRateSupport,				// Non mandatory EVR Presenter Interfaces (see later...)
	public IMFVideoPositionMapper,
	public IEVRTrustedVideoPlugin,
	public IQualProp,
	public IMFVideoDisplayControl 
*/
{
public:
	CEVRAllocatorPresenter(HWND hWnd, HRESULT& hr);
	~CEVRAllocatorPresenter(void);

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	STDMETHODIMP	CreateRenderer(IUnknown** ppRenderer);
	STDMETHODIMP_(bool) Paint(bool fAll);
	STDMETHODIMP	GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight);
	STDMETHODIMP	InitializeDevice(AM_MEDIA_TYPE*	pMediaType);


	// IMFClockStateSink
	STDMETHODIMP	OnClockStart(/* [in] */ MFTIME hnsSystemTime, /* [in] */ LONGLONG llClockStartOffset);        
	STDMETHODIMP	STDMETHODCALLTYPE OnClockStop(/* [in] */ MFTIME hnsSystemTime);
	STDMETHODIMP	STDMETHODCALLTYPE OnClockPause(/* [in] */ MFTIME hnsSystemTime);
	STDMETHODIMP	STDMETHODCALLTYPE OnClockRestart(/* [in] */ MFTIME hnsSystemTime);
	STDMETHODIMP	STDMETHODCALLTYPE OnClockSetRate(/* [in] */ MFTIME hnsSystemTime, /* [in] */ float flRate);


	// IMFVideoPresenter
	STDMETHODIMP	ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam);
	STDMETHODIMP	GetCurrentMediaType(__deref_out  IMFVideoMediaType **ppMediaType);

	// IMFTopologyServiceLookup
    STDMETHODIMP	LookupService(  /* [in] */		MF_SERVICE_LOOKUP_TYPE Type,
									/* [in] */		DWORD dwIndex,
									/* [in] */		REFGUID guidService,
									/* [in] */		REFIID riid,
									/* [out]*/		__out_ecount_full(*pnObjects)  LPVOID *ppvObjects,
									/* [out][in] */ __inout  DWORD *pnObjects);

	// IMFTopologyServiceLookupClient        
	STDMETHODIMP	InitServicePointers(/* [in] */ __in  IMFTopologyServiceLookup *pLookup);
	STDMETHODIMP	ReleaseServicePointers();

	// IMFVideoDeviceID
	STDMETHODIMP	GetDeviceID(/* [out] */	__out  IID *pDeviceID);

	// IMFGetService
	STDMETHODIMP	GetService (/* [in] */ __RPC__in REFGUID guidService,
								/* [in] */ __RPC__in REFIID riid,
								/* [iid_is][out] */ __RPC__deref_out_opt LPVOID *ppvObject);

	// IMFAsyncCallback
	STDMETHODIMP	GetParameters(	/* [out] */ __RPC__out DWORD *pdwFlags, /* [out] */ __RPC__out DWORD *pdwQueue);
	STDMETHODIMP	Invoke		 (	/* [in] */ __RPC__in_opt IMFAsyncResult *pAsyncResult);


private :

	typedef enum
	{
		Stopped,
		Playing
	} EVRCP_STATE;

	CComPtr<IMFTopologyServiceLookup>		m_pMFTSL;
	CComPtr<IDirect3DDeviceManager9>		m_pD3DManager;
	UINT									m_nResetToken;
	CComPtr<IMFTransform>					m_pMixer;
	HANDLE									m_hEvtQuit;
	HANDLE									m_hEvtPresent;
	HANDLE									m_hThread;
	EVRCP_STATE								m_nState;
	MFTIME									m_hnsStartTime;
	CComPtr<IMFSample>						m_pMFSample;
	CComPtr<IMFMediaBuffer>					m_pMFBuffer;
	CComPtr<IMFPresentationClock>			m_pPresentationClock;

	HRESULT									GetImageFromMixer(MFTIME nsTime);
	void									RenderThread();
	static DWORD WINAPI						PresentThread(LPVOID lpParam);
	HRESULT									AllocRessources();

	PTR_DXVA2CreateDirect3DDeviceManager9	pfDXVA2CreateDirect3DDeviceManager9;
	PTR_MFCreatePresentationClock			pfMFCreatePresentationClock;
	PTR_MFCreateSystemTimeSource			pfMFCreateSystemTimeSource;
	PTR_MFCreateDXSurfaceBuffer				pfMFCreateDXSurfaceBuffer;
	PTR_MFCreateSample						pfMFCreateSample;
											
	PTR_AvSetMmThreadCharacteristicsW		pfAvSetMmThreadCharacteristicsW;
	PTR_AvSetMmThreadPriority				pfAvSetMmThreadPriority;
	PTR_AvRevertMmThreadCharacteristics		pfAvRevertMmThreadCharacteristics;
};


HRESULT CreateEVR(const CLSID& clsid, HWND hWnd, ISubPicAllocatorPresenter** ppAP)
{
	HRESULT		hr = E_FAIL;
	if (clsid == CLSID_EVRAllocatorPresenter)
	{
		*ppAP	= new CEVRAllocatorPresenter(hWnd, hr);
		(*ppAP)->AddRef();

		if(FAILED(hr))
		{
			(*ppAP)->Release();
			*ppAP = NULL;
		}
	}

	return hr;
}


CEVRAllocatorPresenter::CEVRAllocatorPresenter(HWND hWnd, HRESULT& hr)
	: CDX9AllocatorPresenter(hWnd, hr)
{
	DWORD		dwThreadId;
	HMODULE		hLib;

	if (FAILED (hr)) return;

	// Load EVR specifics DLLs
	hLib = LoadLibrary (L"dxva.dll");
	pfDXVA2CreateDirect3DDeviceManager9	= hLib ? (PTR_DXVA2CreateDirect3DDeviceManager9) GetProcAddress (hLib, "DXVA2CreateDirect3DDeviceManager9") : NULL;

	hLib = LoadLibrary (L"mf.dll");
	pfMFCreatePresentationClock = hLib ? (PTR_MFCreatePresentationClock) GetProcAddress (hLib, "MFCreatePresentationClock") : NULL;

	hLib = LoadLibrary (L"mfplat.dll");
	pfMFCreateSystemTimeSource	= hLib ? (PTR_MFCreateSystemTimeSource) GetProcAddress (hLib, "MFCreateSystemTimeSource") : NULL;
	pfMFCreateSample			= hLib ? (PTR_MFCreateSample)			GetProcAddress (hLib, "MFCreateSample") : NULL;
	
	hLib = LoadLibrary (L"evr.dll");
	pfMFCreateDXSurfaceBuffer	= hLib ? (PTR_MFCreateDXSurfaceBuffer)	GetProcAddress (hLib, "MFCreateDXSurfaceBuffer") : NULL;

	if (!pfDXVA2CreateDirect3DDeviceManager9 || !pfMFCreatePresentationClock || !pfMFCreateSystemTimeSource ||
		!pfMFCreateSample || !pfMFCreateDXSurfaceBuffer)
	{
		hr = E_FAIL;
		return;
	}

	// Load Vista specifics DLLs
	hLib = LoadLibrary (L"AVRT.dll");
	pfAvSetMmThreadCharacteristicsW		= hLib ? (PTR_AvSetMmThreadCharacteristicsW)	GetProcAddress (hLib, "AvSetMmThreadCharacteristicsW") : NULL;
	pfAvSetMmThreadPriority				= hLib ? (PTR_AvSetMmThreadPriority)			GetProcAddress (hLib, "AvSetMmThreadPriority") : NULL;
	pfAvRevertMmThreadCharacteristics	= hLib ? (PTR_AvRevertMmThreadCharacteristics)	GetProcAddress (hLib, "AvRevertMmThreadCharacteristics") : NULL;

	// Init DXVA manager
	hr = pfDXVA2CreateDirect3DDeviceManager9(&m_nResetToken, &m_pD3DManager);
	if (SUCCEEDED (hr)) hr = m_pD3DManager->ResetDevice(m_pD3DDev, m_nResetToken);

	m_nState		= Stopped;
	m_hnsStartTime	= -1;
	m_hEvtQuit		= CreateEvent (NULL, FALSE, FALSE, NULL);
	m_hEvtPresent	= CreateEvent (NULL, FALSE, FALSE, NULL);
	m_hThread		= ::CreateThread(NULL, 0, PresentThread, (LPVOID)this, 0, &dwThreadId);
}

CEVRAllocatorPresenter::~CEVRAllocatorPresenter(void)
{
	SetEvent (m_hEvtQuit);
	if (WaitForSingleObject (m_hThread, 10000) == WAIT_TIMEOUT)
	{
		ASSERT (FALSE);
		TerminateThread (m_hThread, 0xDEAD);
	}
}

STDMETHODIMP CEVRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	HRESULT					hr = E_FAIL;
	CComPtr<IBaseFilter>	pBF;

	hr = pBF.CoCreateInstance(CLSID_EnhancedVideoRenderer);
	if(SUCCEEDED(hr))
	{
		CComPtr<IMFVideoPresenter>		pVP;
		CComPtr<IMFVideoRenderer>		pMFVR;
		CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS = pBF;

		hr = pMFGS->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoRenderer, (void**)&pMFVR);
		if(SUCCEEDED(hr)) hr = QueryInterface (__uuidof(IMFVideoPresenter), (void**)&pVP);
		if(SUCCEEDED(hr)) hr = pMFVR->InitializeRenderer (NULL, pVP);

		if(SUCCEEDED(hr))
		{
			hr = pfMFCreatePresentationClock(&m_pPresentationClock);

			CComPtr<IMFPresentationTimeSource>	pSystemTimeSource;
			pfMFCreateSystemTimeSource (&pSystemTimeSource);
			m_pPresentationClock->SetTimeSource (pSystemTimeSource);
		}

		if(FAILED(hr))
		{
			*ppRenderer = NULL;
		}
		else
			*ppRenderer = pBF.Detach();
	}

	return hr;
}

STDMETHODIMP_(bool) CEVRAllocatorPresenter::Paint(bool fAll)
{
	
	return __super::Paint (fAll);
}

STDMETHODIMP CEVRAllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	HRESULT		hr;
	if(riid == __uuidof(IMFClockStateSink))
		hr = GetInterface((IMFClockStateSink*)this, ppv);
	else if(riid == __uuidof(IMFVideoPresenter))
		hr = GetInterface((IMFVideoPresenter*)this, ppv);
	else if(riid == __uuidof(IMFTopologyServiceLookup))
		hr = GetInterface((IMFTopologyServiceLookup*)this, ppv);
	else if(riid == __uuidof(IMFTopologyServiceLookupClient))
		hr = GetInterface((IMFTopologyServiceLookupClient*)this, ppv);
	else if(riid == __uuidof(IMFVideoDeviceID))
		hr = GetInterface((IMFVideoDeviceID*)this, ppv);
	else if(riid == __uuidof(IMFGetService))
		hr = GetInterface((IMFGetService*)this, ppv);
	else if(riid == __uuidof(IMFAsyncCallback))
		hr = GetInterface((IMFAsyncCallback*)this, ppv);
	else if(riid == __uuidof(IDirect3DDeviceManager9))
		hr = m_pD3DManager->QueryInterface (__uuidof(IDirect3DDeviceManager9), (void**) ppv);
	else
		hr = __super::NonDelegatingQueryInterface(riid, ppv);

//	ASSERT (SUCCEEDED(hr));

	return hr;
}


// IMFClockStateSink
STDMETHODIMP CEVRAllocatorPresenter::OnClockStart(/* [in] */ MFTIME hnsSystemTime, /* [in] */ LONGLONG llClockStartOffset)
{
	TRACE ("OnClockStart  %I64d\n", hnsSystemTime);
	m_nState		= Playing;
	m_hnsStartTime	= hnsSystemTime;

	m_pPresentationClock->Start(0);
	SetEvent(m_hEvtPresent);

	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::OnClockStop(/* [in] */ MFTIME hnsSystemTime)
{
	TRACE ("OnClockStop  %I64d\n", hnsSystemTime);
	m_hnsStartTime	= -1;
	m_pPresentationClock->Stop();

	return E_NOTIMPL;
}
STDMETHODIMP CEVRAllocatorPresenter::OnClockPause(/* [in] */ MFTIME hnsSystemTime)
{
	TRACE ("OnClockPause  %I64d\n", hnsSystemTime);
	m_pPresentationClock->Pause();

	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::OnClockRestart(/* [in] */ MFTIME hnsSystemTime)
{
	TRACE ("OnClockRestart  %I64d\n", hnsSystemTime);
	m_pPresentationClock->Start(0);
	SetEvent(m_hEvtPresent);

	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::OnClockSetRate(/* [in] */ MFTIME hnsSystemTime, /* [in] */ float flRate)
{
	return E_NOTIMPL;
}


// IMFVideoPresenter
STDMETHODIMP CEVRAllocatorPresenter::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
	HRESULT					hr = S_OK;
	int						i;
	CComPtr<IMFMediaType>	pMediaType;

	switch (eMessage)
	{
	case MFVP_MESSAGE_BEGINSTREAMING :			// The EVR switched from stopped to paused. The presenter should allocate resources
		m_nState		= Stopped;
//		SetEvent(m_hEvtPresent);
		TRACE ("MFVP_MESSAGE_BEGINSTREAMING\n");
		break;
	case MFVP_MESSAGE_CANCELSTEP :				// Cancels a frame step
		TRACE ("MFVP_MESSAGE_CANCELSTEP\n");
		break;
	case MFVP_MESSAGE_ENDOFSTREAM :				// All input streams have ended. 
		TRACE ("MFVP_MESSAGE_ENDOFSTREAM\n");
		break;
	case MFVP_MESSAGE_ENDSTREAMING :			// The EVR switched from running or paused to stopped. The presenter should free resources
		TRACE ("MFVP_MESSAGE_ENDSTREAMING\n");
		break;
	case MFVP_MESSAGE_FLUSH :					// The presenter should discard any pending samples
		TRACE ("MFVP_MESSAGE_FLUSH\n");
//		SetEvent(m_hEvtPresent);
		break;
	case MFVP_MESSAGE_INVALIDATEMEDIATYPE :		// The mixer's output format has changed. The EVR will initiate format negotiation, as described previously
		/*
			1) The EVR sets the media type on the reference stream.
			2) The EVR calls IMFVideoPresenter::ProcessMessage on the presenter with the MFVP_MESSAGE_INVALIDATEMEDIATYPE message.
			3) The presenter sets the media type on the mixer's output stream.
			4) The EVR sets the media type on the substreams.
		*/
		
		AM_MEDIA_TYPE*			pAMMedia;
//		memset (&AMMedia, 0, sizeof(AM_MEDIA_TYPE));
//		AMMedia.majortype	= MEDIATYPE_Video;
//		AMMedia.subtype		= MEDIASUBTYPE_YUY2;
//		MFCreateMediaTypeFromRepresentation (AM_MEDIA_TYPE_REPRESENTATION, &AMMedia,  &MediaType);

		hr	= S_OK;
		i	= 0;
		while (SUCCEEDED (hr))
		{
			hr = m_pMixer->GetOutputAvailableType(0,i, &pMediaType);
			if (SUCCEEDED (hr))
			{
				hr = pMediaType->GetRepresentation(FORMAT_VideoInfo2, (void**)&pAMMedia);
				if (pAMMedia->subtype == MEDIASUBTYPE_RGB32)
				{
					InitializeDevice (pAMMedia);
					AllocRessources();
					hr = m_pMixer->SetOutputType(0, pMediaType, 0);
					break;
				}
			}
			pMediaType = NULL;
			i++;
		}
		if (SUCCEEDED (hr))
		{
			TRACE ("Ok");
		}
		break;
	case MFVP_MESSAGE_PROCESSINPUTNOTIFY :		// One input stream on the mixer has received a new sample
		// call ProcessOutput on the mixer ???
//		hr = GetImageFromMixer();
		break;
	case MFVP_MESSAGE_STEP :					// Requests a frame step.
		TRACE ("MFVP_MESSAGE_STEP\n");
		break;
	default :
		ASSERT (FALSE);
		break;
	}
	return hr;
}


HRESULT CEVRAllocatorPresenter::AllocRessources()
{
	HRESULT			hr;

	m_pMFSample = NULL;
	m_pMFBuffer = NULL;
	hr = pfMFCreateDXSurfaceBuffer (__uuidof(IDirect3DSurface9), m_pVideoSurface[0], false, &m_pMFBuffer);
	hr = pfMFCreateSample(&m_pMFSample);
	hr = m_pMFSample->AddBuffer (m_pMFBuffer);

	return hr;
}

HRESULT CEVRAllocatorPresenter::GetImageFromMixer(MFTIME nsCurrentTime)
{
	MFT_OUTPUT_DATA_BUFFER		Buffer;
	HRESULT						hr;
	DWORD						dwStatus;

	memset (&Buffer, 0, sizeof(Buffer));
	Buffer.pSample = m_pMFSample;

	MFTIME	nsSampleTime;
	do
	{
		hr = m_pMixer->ProcessOutput (0 , 1, &Buffer, &dwStatus);
		Buffer.pSample->GetSampleTime (&nsSampleTime);
	} while (nsSampleTime < nsCurrentTime);

	if(m_pSubPicQueue)
	{
		MFTIME		nsDuration;
		float		m_fps;
		Buffer.pSample->GetSampleDuration(&nsDuration);
		m_fps = 10000000.0 / nsDuration;
		m_pSubPicQueue->SetFPS(m_fps);
		__super::SetTime (nsSampleTime);
	}

	if (AfxGetMyApp()->m_fTearingTest)
	{
		RECT		rcTearing;
		
		rcTearing.left		= m_nTearingPos;
		rcTearing.top		= 0;
		rcTearing.right		= rcTearing.left + 4;
		rcTearing.bottom	= m_NativeVideoSize.cy;
		m_pD3DDev->ColorFill (m_pVideoSurface[0], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

		rcTearing.left	= (rcTearing.right + 15) % m_NativeVideoSize.cx;
		rcTearing.right	= rcTearing.left + 4;
		m_pD3DDev->ColorFill (m_pVideoSurface[0], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

		m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
	}

	Paint(true);

	return hr;
}



STDMETHODIMP CEVRAllocatorPresenter::GetCurrentMediaType(__deref_out  IMFVideoMediaType **ppMediaType)
{
	ASSERT(FALSE);
	return E_NOTIMPL;
}


// IMFTopologyServiceLookup
STDMETHODIMP CEVRAllocatorPresenter::LookupService(  /* [in] */		MF_SERVICE_LOOKUP_TYPE Type,
									/* [in] */		DWORD dwIndex,
									/* [in] */		REFGUID guidService,
									/* [in] */		REFIID riid,
									/* [out]*/		__out_ecount_full(*pnObjects)  LPVOID *ppvObjects,
									/* [out][in] */ __inout  DWORD *pnObjects)
{
	ASSERT(FALSE);
	return E_NOTIMPL;
}


	// IMFTopologyServiceLookupClient        
STDMETHODIMP CEVRAllocatorPresenter::InitServicePointers(/* [in] */ __in  IMFTopologyServiceLookup *pLookup)
{
	TRACE ("EVR : CEVRAllocatorPresenter::InitServicePointers\n");
	m_pMFTSL = pLookup;

	HRESULT						hr;
	DWORD						dwObjects = 1;

	hr = m_pMFTSL->LookupService (MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_MIXER_SERVICE,
								  __uuidof (IMFTransform), (void**)&m_pMixer, &dwObjects);

	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::ReleaseServicePointers()
{
	TRACE ("EVR : CEVRAllocatorPresenter::ReleaseServicePointers\n");
	m_pMixer	= NULL;
	m_pMFTSL	= NULL;
	return S_OK;
}


// IMFVideoDeviceID
STDMETHODIMP CEVRAllocatorPresenter::GetDeviceID(/* [out] */	__out  IID *pDeviceID)
{
	*pDeviceID = IID_IDirect3DDevice9;
	return S_OK;
}


// IMFGetService
STDMETHODIMP CEVRAllocatorPresenter::GetService (/* [in] */ __RPC__in REFGUID guidService,
								/* [in] */ __RPC__in REFIID riid,
								/* [iid_is][out] */ __RPC__deref_out_opt LPVOID *ppvObject)
{
	if (guidService == MR_VIDEO_RENDER_SERVICE)
		return NonDelegatingQueryInterface (riid, ppvObject);
	return E_NOINTERFACE;
}


// IMFAsyncCallback
STDMETHODIMP CEVRAllocatorPresenter::GetParameters(	/* [out] */ __RPC__out DWORD *pdwFlags, /* [out] */ __RPC__out DWORD *pdwQueue)
{
	return E_NOTIMPL;
}

STDMETHODIMP CEVRAllocatorPresenter::Invoke		 (	/* [in] */ __RPC__in_opt IMFAsyncResult *pAsyncResult)
{
	return E_NOTIMPL;
}


STDMETHODIMP CEVRAllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	if(lpWidth) *lpWidth = m_NativeVideoSize.cx;
	if(lpHeight) *lpHeight = m_NativeVideoSize.cy;
	if(lpARWidth) *lpARWidth = m_AspectRatio.cx;
	if(lpARHeight) *lpARHeight = m_AspectRatio.cy;
	return S_OK;
}


STDMETHODIMP CEVRAllocatorPresenter::InitializeDevice(AM_MEDIA_TYPE*	pMediaType)
{
	HRESULT			hr;

	DeleteSurfaces();

	VIDEOINFOHEADER2*		vih2 = (VIDEOINFOHEADER2*) pMediaType->pbFormat;
	int						w = vih2->bmiHeader.biWidth;
	int						h = abs(vih2->bmiHeader.biHeight);

	m_NativeVideoSize = m_AspectRatio = CSize(w, h);

	hr = AllocSurfaces();
	if (SUCCEEDED (hr))	hr = m_pD3DDev->ColorFill(m_pVideoSurface[0], NULL, 0);

	return hr;
}




DWORD WINAPI CEVRAllocatorPresenter::PresentThread(LPVOID lpParam)
{
	CEVRAllocatorPresenter*		pThis = (CEVRAllocatorPresenter*) lpParam;
	pThis->RenderThread();
	return 0;
}

void CEVRAllocatorPresenter::RenderThread()
{
	HANDLE				hAvrt;
	DWORD				dwTaskIndex	= 0;
	HANDLE				hEvts[]		= { m_hEvtQuit, m_hEvtPresent };
	bool				bQuit		= false;
	HRESULT				hr;
    TIMECAPS			tc;
	DWORD				dwResolution;
	MFTIME				nsSampleTime;
	MFTIME				nsCurrentTime;
	UINT				uDelay;
	DWORD				dwUser = 0;

	// Tell Vista Multimedia Class Scheduler we are a playback thread (increase priority)
	if (pfAvSetMmThreadCharacteristicsW)	hAvrt = pfAvSetMmThreadCharacteristicsW (L"Playback", &dwTaskIndex);
	if (pfAvSetMmThreadPriority)			pfAvSetMmThreadPriority (hAvrt, AVRT_PRIORITY_HIGH /*AVRT_PRIORITY_CRITICAL*/);

    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    dwResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    dwUser = timeBeginPeriod(dwResolution);
	
	uDelay = 50;
	while (!bQuit)
	{
		switch (WaitForMultipleObjects (sizeof(hEvts)/sizeof(HANDLE), hEvts, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0 :
			bQuit = true;
			break;
		case WAIT_OBJECT_0 + 1 :

			hr = m_pPresentationClock->GetTime(&nsCurrentTime);

			GetImageFromMixer(nsCurrentTime);

			TRACE ("==>> Present new picture\n");
			m_pMFSample->GetSampleTime (&nsSampleTime);
			if ((nsSampleTime > nsCurrentTime))
			{
				uDelay = (nsSampleTime - nsCurrentTime) / 10000;
				TRACE ("==>> Set timer %d\n", uDelay);
				dwUser = timeSetEvent (uDelay, dwResolution, (LPTIMECALLBACK)m_hEvtPresent, NULL, TIME_CALLBACK_EVENT_SET); 
			}

			break;
		}
	}

	timeEndPeriod (dwResolution);
	if (pfAvRevertMmThreadCharacteristics) pfAvRevertMmThreadCharacteristics (hAvrt);
}