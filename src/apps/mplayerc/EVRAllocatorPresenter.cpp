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
//typedef HRESULT (__stdcall *PTR_MFCreateSystemTimeSource)(IMFPresentationTimeSource** ppSystemTimeSource);

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
	public IMFAsyncCallback,
	public IQualProp
/*	public IMFRateSupport,				// Non mandatory EVR Presenter Interfaces (see later...)
	public IMFVideoPositionMapper,
	public IEVRTrustedVideoPlugin,
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

	// IQualProp (EVR statistics window)
    STDMETHODIMP	get_FramesDroppedInRenderer		(int *pcFrames);
    STDMETHODIMP	get_FramesDrawn					(int *pcFramesDrawn);
    STDMETHODIMP	get_AvgFrameRate				(int *piAvgFrameRate);
    STDMETHODIMP	get_Jitter						(int *iJitter);
    STDMETHODIMP	get_AvgSyncOffset				(int *piAvg);
    STDMETHODIMP	get_DevSyncOffset				(int *piDev);

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
		Paused,
		Playing
	} EVRCP_STATE;

	IBaseFilter*							m_pEVR;
	CComPtr<IReferenceClock>				m_pClock;
	CComPtr<IDirect3DDeviceManager9>		m_pD3DManager;
	CComPtr<IMFTransform>					m_pMixer;
	CComPtr<IMediaEventSink>				m_pSink;

	HANDLE									m_hEvtQuit;			// Stop rendering thread event
	HANDLE									m_hEvtPresent;		// Render next frame (cued order)
	HANDLE									m_hEvtFrameTimer;	// Render next frame (timer based)
	HANDLE									m_hEvtNewFrame;		// New frame on mixer output
	HANDLE									m_hEvtFlush;		// Discard all buffers

	HANDLE									m_hSemPicture;
	HANDLE									m_hSemSlot;
	int										m_nFreeSlot;
	REFERENCE_TIME							m_rtStart;
	REFERENCE_TIME							m_rtPause;
	bool									m_fUseInternalTimer;

	HANDLE									m_hThread[2];
	EVRCP_STATE								m_nState;
	CComPtr<IMFSample>						m_pMFSample[PICTURE_SLOTS];
	CComPtr<IMFMediaBuffer>					m_pMFBuffer[PICTURE_SLOTS];

	int										m_pcFrames;
	int										m_pcFramesDrawn;	// Retrieves the number of frames drawn since streaming started
	int										m_piAvgFrameRate;
	int										m_iJitter;
	int										m_piAvg;
	int										m_piDev;

	HRESULT									GetImageFromMixer();
	void									RenderThread();
	void									ReadMixerThread();
	static DWORD WINAPI						PresentThread(LPVOID lpParam);
	static DWORD WINAPI						ProduceThread(LPVOID lpParam);
	HRESULT									AllocRessources();
	void									ResetStats();


	// === Functions pointers on Vista / .Net3 specifics library
	PTR_DXVA2CreateDirect3DDeviceManager9	pfDXVA2CreateDirect3DDeviceManager9;
//	PTR_MFCreateSystemTimeSource			pfMFCreateSystemTimeSource;
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
	UINT		nResetToken;

	if (FAILED (hr)) return;

	// Load EVR specifics DLLs
	hLib = LoadLibrary (L"dxva2.dll");
	pfDXVA2CreateDirect3DDeviceManager9	= hLib ? (PTR_DXVA2CreateDirect3DDeviceManager9) GetProcAddress (hLib, "DXVA2CreateDirect3DDeviceManager9") : NULL;

//	hLib = LoadLibrary (L"mf.dll");
//	pfMFCreatePresentationClock = hLib ? (PTR_MFCreatePresentationClock) GetProcAddress (hLib, "MFCreatePresentationClock") : NULL;

	hLib = LoadLibrary (L"mfplat.dll");
//	pfMFCreateSystemTimeSource	= hLib ? (PTR_MFCreateSystemTimeSource) GetProcAddress (hLib, "MFCreateSystemTimeSource") : NULL;
	pfMFCreateSample			= hLib ? (PTR_MFCreateSample)			GetProcAddress (hLib, "MFCreateSample") : NULL;
	
	hLib = LoadLibrary (L"evr.dll");
	pfMFCreateDXSurfaceBuffer	= hLib ? (PTR_MFCreateDXSurfaceBuffer)	GetProcAddress (hLib, "MFCreateDXSurfaceBuffer") : NULL;

	if (!pfDXVA2CreateDirect3DDeviceManager9 || !pfMFCreateSample || !pfMFCreateDXSurfaceBuffer)
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
	hr = pfDXVA2CreateDirect3DDeviceManager9(&nResetToken, &m_pD3DManager);
	if (SUCCEEDED (hr)) hr = m_pD3DManager->ResetDevice(m_pD3DDev, nResetToken);

	ResetStats();
	m_nState		= Stopped;
	m_hEvtQuit		= CreateEvent (NULL, TRUE, FALSE, NULL);
	m_hEvtPresent	= CreateEvent (NULL, FALSE, FALSE, NULL);
	m_hEvtFrameTimer= CreateEvent (NULL, FALSE, FALSE, NULL);
	m_hEvtNewFrame	= CreateEvent (NULL, FALSE, FALSE, NULL);
	m_hEvtFlush		= CreateEvent (NULL, TRUE, FALSE, NULL);
	m_hSemPicture	= CreateSemaphore(NULL, 0, PICTURE_SLOTS, NULL);
	m_hSemSlot		= CreateSemaphore(NULL, PICTURE_SLOTS, PICTURE_SLOTS, NULL);
	m_nFreeSlot		= 0;
	m_fUseInternalTimer	= false;

	m_hThread[0]	= ::CreateThread(NULL, 0, PresentThread, (LPVOID)this, 0, &dwThreadId);
	m_hThread[1]	= ::CreateThread(NULL, 0, ProduceThread, (LPVOID)this, 0, &dwThreadId);
}

CEVRAllocatorPresenter::~CEVRAllocatorPresenter(void)
{
	int			i;

	SetEvent (m_hEvtQuit);
	if (WaitForMultipleObjects (countof(m_hThread), m_hThread, TRUE, 10000) == WAIT_TIMEOUT)
	{
		ASSERT (FALSE);
		for (i=0; i<2; i++)
			TerminateThread (m_hThread[i], 0xDEAD);
	}

	for (i=0; i<countof(m_pMFSample); i++)
	{
		m_pMFSample[i] = NULL;
		m_pMFBuffer[i] = NULL;
	}

	m_pClock		= NULL;
	m_pEVR			= NULL;
	m_pD3DManager	= NULL;

	for (i=0; i<2; i++)
		CloseHandle (m_hThread[i]);
	CloseHandle (m_hSemPicture);
	CloseHandle (m_hSemSlot);
	CloseHandle (m_hEvtPresent);
	CloseHandle (m_hEvtFrameTimer);
	CloseHandle (m_hEvtNewFrame);
	CloseHandle (m_hEvtFlush);
	CloseHandle (m_hEvtQuit);
}


void CEVRAllocatorPresenter::ResetStats()
{
	m_pcFrames			= 0;
	m_pcFramesDrawn		= 0;
	m_piAvgFrameRate	= 0;
	m_iJitter			= 100;
	m_piAvg				= 0;
	m_piDev				= 0;
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

		m_pEVR = pBF;

		hr = pMFGS->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoRenderer, (void**)&pMFVR);
		if(SUCCEEDED(hr)) hr = QueryInterface (__uuidof(IMFVideoPresenter), (void**)&pVP);
		if(SUCCEEDED(hr)) hr = pMFVR->InitializeRenderer (NULL, pVP);

		CComPtr<IPin> pPin = GetFirstPin(pBF);
		CComQIPtr<IMemInputPin> pMemInputPin = pPin;
		
		// No NewSegment : no chocolate :o)
		m_fUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);

		if(FAILED(hr))
		{
			*ppRenderer = NULL;
		}
		else
		{
			*ppRenderer = pBF.Detach();
			m_pEVR		= (IBaseFilter*)*ppRenderer;		// TODO : trouver mieux  (addref impossible sinon pas de destruction)
		}
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
	else if(riid == IID_IQualProp)
		hr = GetInterface((IQualProp*)this, ppv);
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
	m_nState		= Playing;
	
	m_rtStart = llClockStartOffset;
	m_rtPause = 0;
	SetEvent(m_hEvtPresent);
	TRACE ("OnClockStart  new segment : %I64d\n", llClockStartOffset);

	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockStop(/* [in] */ MFTIME hnsSystemTime)
{
	TRACE ("OnClockStop  %I64d\n", hnsSystemTime);
	m_nState		= Stopped;

	return E_NOTIMPL;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockPause(/* [in] */ MFTIME hnsSystemTime)
{
	TRACE ("OnClockPause  %I64d\n", hnsSystemTime);
	m_nState		= Paused;
	m_rtPause		= hnsSystemTime;
	
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockRestart(/* [in] */ MFTIME hnsSystemTime)
{
	m_nState	= Playing;
	m_rtStart	 = hnsSystemTime + ((m_rtPause != 0) ? - m_rtPause + m_rtStart : 0);
//	m_rtStart	 = hnsSystemTime;
	m_rtPause	= 0;
	SetEvent(m_hEvtPresent);

	TRACE ("OnClockRestart  new segment : %I64d\n", hnsSystemTime);

	return S_OK;
}


STDMETHODIMP CEVRAllocatorPresenter::OnClockSetRate(/* [in] */ MFTIME hnsSystemTime, /* [in] */ float flRate)
{
	ASSERT (FALSE);
	return E_NOTIMPL;
}

// IQualProp
STDMETHODIMP CEVRAllocatorPresenter::get_FramesDroppedInRenderer(int *pcFrames)
{
	*pcFrames	= m_pcFrames;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_FramesDrawn(int *pcFramesDrawn)
{
	*pcFramesDrawn = m_pcFramesDrawn;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_AvgFrameRate(int *piAvgFrameRate)
{
	*piAvgFrameRate = m_piAvgFrameRate;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_Jitter(int *iJitter)
{
	*iJitter = m_iJitter;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_AvgSyncOffset(int *piAvg)
{
	*piAvg = m_piAvg;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_DevSyncOffset(int *piDev)
{
	*piDev = m_piDev;
	return S_OK;
}


// IMFVideoPresenter
STDMETHODIMP CEVRAllocatorPresenter::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
	HRESULT						hr = S_OK;
	int							i;
	CComPtr<IMFMediaType>		pMediaType;

	switch (eMessage)
	{
	case MFVP_MESSAGE_BEGINSTREAMING :			// The EVR switched from stopped to paused. The presenter should allocate resources
		m_nState		= Stopped;
		ResetStats();
		TRACE ("MFVP_MESSAGE_BEGINSTREAMING\n");
		m_pClock = NULL;
		hr = m_pEVR->GetSyncSource (&m_pClock);
		SetEvent(m_hEvtPresent);
		break;

	case MFVP_MESSAGE_CANCELSTEP :				// Cancels a frame step
		TRACE ("MFVP_MESSAGE_CANCELSTEP\n");
		break;

	case MFVP_MESSAGE_ENDOFSTREAM :				// All input streams have ended. 
		TRACE ("MFVP_MESSAGE_ENDOFSTREAM\n");
		m_pSink->Notify (EC_COMPLETE, 0, 0);
		break;

	case MFVP_MESSAGE_ENDSTREAMING :			// The EVR switched from running or paused to stopped. The presenter should free resources
		TRACE ("MFVP_MESSAGE_ENDSTREAMING\n");
		break;

	case MFVP_MESSAGE_FLUSH :					// The presenter should discard any pending samples
		TRACE ("MFVP_MESSAGE_FLUSH\n");
		SetEvent(m_hEvtFlush);
		while (WaitForSingleObject(m_hEvtFlush, 1) == WAIT_OBJECT_0);
		m_rtPause = 0;
//		ResetEvent(m_hEvtFlush);
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
		SetEvent (m_hEvtNewFrame);
		TRACE ("MFVP_MESSAGE_PROCESSINPUTNOTIFY\n");
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
	int				i;

	// Create Media Foundation buffers for EVR Mixer
	for (i=0; i<countof(m_pMFSample); i++)
	{
		m_pMFSample[i] = NULL;
		m_pMFBuffer[i] = NULL;
		hr = pfMFCreateDXSurfaceBuffer (__uuidof(IDirect3DSurface9), m_pVideoSurface[i], false, &m_pMFBuffer[i]);
		hr = pfMFCreateSample(&m_pMFSample[i]);
		hr = m_pMFSample[i]->AddBuffer (m_pMFBuffer[i]);
	}

	return hr;
}

HRESULT CEVRAllocatorPresenter::GetImageFromMixer()
{
	MFT_OUTPUT_DATA_BUFFER		Buffer;
	HRESULT						hr = S_FALSE;
	DWORD						dwStatus;
	REFERENCE_TIME				nsSampleTime;
	HANDLE						hEvts[] = { m_hEvtFlush , m_hSemSlot };

	if (WaitForMultipleObjects (countof(hEvts), hEvts, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)		// TODO : 300 ????
	{
		memset (&Buffer, 0, sizeof(Buffer));
		Buffer.pSample = m_pMFSample[m_nFreeSlot];

		hr = m_pMixer->ProcessOutput (0 , 1, &Buffer, &dwStatus);
		Buffer.pSample->GetSampleTime (&nsSampleTime);

		// Update internal subtitle clock
		if(m_fUseInternalTimer && m_pSubPicQueue)
		{
			MFTIME		nsDuration;
			float		m_fps;
			Buffer.pSample->GetSampleDuration(&nsDuration);
			m_fps = 10000000.0 / nsDuration;
			m_pSubPicQueue->SetFPS(m_fps);
			__super::SetTime (g_tSegmentStart + nsSampleTime);
		}

		if (AfxGetMyApp()->m_fTearingTest)
		{
			RECT		rcTearing;
			
			rcTearing.left		= m_nTearingPos;
			rcTearing.top		= 0;
			rcTearing.right		= rcTearing.left + 4;
			rcTearing.bottom	= m_NativeVideoSize.cy;
			m_pD3DDev->ColorFill (m_pVideoSurface[m_nFreeSlot], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

			rcTearing.left	= (rcTearing.right + 15) % m_NativeVideoSize.cx;
			rcTearing.right	= rcTearing.left + 4;
			m_pD3DDev->ColorFill (m_pVideoSurface[m_nFreeSlot], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

			m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
		}
	
		m_nFreeSlot = (m_nFreeSlot+1) % PICTURE_SLOTS;
		ReleaseSemaphore (m_hSemPicture, 1, NULL);

		TRACE ("New image from muxer : %I64d\n", nsSampleTime/417188);
	}

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
	HRESULT						hr;
	DWORD						dwObjects = 1;

	TRACE ("EVR : CEVRAllocatorPresenter::InitServicePointers\n");
	hr = pLookup->LookupService (MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_MIXER_SERVICE,
								  __uuidof (IMFTransform), (void**)&m_pMixer, &dwObjects);

	hr = pLookup->LookupService (MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE,
								  __uuidof (IMediaEventSink ), (void**)&m_pSink, &dwObjects);

	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::ReleaseServicePointers()
{
	TRACE ("EVR : CEVRAllocatorPresenter::ReleaseServicePointers\n");
	m_pMixer	= NULL;
	m_pSink		= NULL;
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
	if(lpWidth)		*lpWidth	= m_NativeVideoSize.cx;
	if(lpHeight)	*lpHeight	= m_NativeVideoSize.cy;
	if(lpARWidth)	*lpARWidth	= m_AspectRatio.cx;
	if(lpARHeight)	*lpARHeight	= m_AspectRatio.cy;
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

	return hr;
}



DWORD WINAPI CEVRAllocatorPresenter::PresentThread(LPVOID lpParam)
{
	CEVRAllocatorPresenter*		pThis = (CEVRAllocatorPresenter*) lpParam;
	pThis->RenderThread();
	return 0;
}


DWORD WINAPI CEVRAllocatorPresenter::ProduceThread(LPVOID lpParam)
{
	CEVRAllocatorPresenter*		pThis = (CEVRAllocatorPresenter*) lpParam;
	pThis->ReadMixerThread();
	return 0;
}

void CEVRAllocatorPresenter::ReadMixerThread()
{
	HANDLE				hEvts[]		= { m_hEvtQuit, m_hEvtNewFrame };
	bool				bQuit		= false;

	// Eat as much as you can (producer thread)
	while (!bQuit)
	{
		switch (WaitForMultipleObjects (sizeof(hEvts)/sizeof(HANDLE), hEvts, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0 :
			bQuit = true;
			break;
		case WAIT_OBJECT_0 + 1 :
			GetImageFromMixer();
			break;
		}
	}
}

void CEVRAllocatorPresenter::RenderThread()
{
	HANDLE				hAvrt;
	DWORD				dwTaskIndex	= 0;
	HANDLE				hEvts[]		= { m_hEvtQuit, m_hEvtFlush , m_hEvtPresent, m_hEvtFrameTimer};
	HANDLE				hEvtsBuff[]	= { m_hEvtFlush, m_hSemPicture };
	bool				bQuit		= false;
	HRESULT				hr;
    TIMECAPS			tc;
	DWORD				dwResolution;
	MFTIME				nsSampleTime;
	MFTIME				nsCurrentTime;
	long				lDelay;
	DWORD				dwUser = 0;
	DWORD				dwObject;

	// Tell Vista Multimedia Class Scheduler we are a playback thretad (increase priority)
	if (pfAvSetMmThreadCharacteristicsW)	hAvrt = pfAvSetMmThreadCharacteristicsW (L"Playback", &dwTaskIndex);
	if (pfAvSetMmThreadPriority)			pfAvSetMmThreadPriority (hAvrt, AVRT_PRIORITY_HIGH /*AVRT_PRIORITY_CRITICAL*/);

    timeGetDevCaps(&tc, sizeof(TIMECAPS));
    dwResolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
    dwUser		= timeBeginPeriod(dwResolution);

	
	while (!bQuit)
	{
		dwObject = WaitForMultipleObjects (countof(hEvts), hEvts, FALSE, INFINITE);
		switch (dwObject)
		{
		case WAIT_OBJECT_0 :
			bQuit = true;
			break;
		case WAIT_OBJECT_0 + 1 :
			// Flush pending samples!
			if (dwUser != -1) timeKillEvent (dwUser);
			dwUser = -1;
			while (WaitForSingleObject (m_hSemPicture, 0) == WAIT_OBJECT_0)
				ReleaseSemaphore (m_hSemSlot, 1, NULL);
			m_nCurPicture = m_nFreeSlot;
			Sleep(1);
			ResetEvent(m_hEvtFlush);
			TRACE ("End flush\n");
			break;

		case WAIT_OBJECT_0 + 2 :
		case WAIT_OBJECT_0 + 3 :
			
			if ((dwObject == WAIT_OBJECT_0 + 3) && (m_nState != Playing)) continue;
			TRACE ("RenderThread ==>> Waiting buffer\n");
			
			if (WaitForMultipleObjects (countof(hEvtsBuff), hEvtsBuff, FALSE, INFINITE) == WAIT_OBJECT_0+1)
			{
				m_pClock->GetTime(&nsCurrentTime);
				
				m_pMFSample[m_nCurPicture]->GetSampleTime (&nsSampleTime);
				if (m_nState == Playing)
				{
					lDelay = (nsSampleTime - nsCurrentTime + m_rtStart) / 10000;
					if (lDelay > 0)
					{
//ASSERT (uDelay < 300);
						TRACE ("RenderThread ==>> Set timer %d   %I64d\n", lDelay, nsSampleTime/417188);
						dwUser = timeSetEvent (lDelay, dwResolution, (LPTIMECALLBACK)m_hEvtFrameTimer, NULL, TIME_CALLBACK_EVENT_SET); 
					}
					else
					{
						dwUser = -1;
						m_pcFrames++;
						TRACE ("RenderThread ==>> immediate display   %I64d  (delay=%d)\n", nsSampleTime/417188, lDelay);
						SetEvent (m_hEvtPresent);
					}
				}
//				TRACE ("RenderThread ==>> Presenting\n");
				Paint(true);
				m_pcFramesDrawn++;

				m_nCurPicture = (m_nCurPicture + 1) % PICTURE_SLOTS;
				ReleaseSemaphore (m_hSemSlot, 1, NULL);
//				TRACE ("RenderThread ==>> Sleeping\n");
			}
			else
			{
				ASSERT (FALSE);
				TRACE ("RenderThread ==>> ERROR cannot render picture in time!\n");
			}

			break;
		}
	}

	timeEndPeriod (dwResolution);
	if (pfAvRevertMmThreadCharacteristics) pfAvRevertMmThreadCharacteristics (hAvrt);
}