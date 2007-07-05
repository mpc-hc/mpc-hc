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
#include <Mferror.h>
#include "..\..\SubPic\DX9SubPic.h"
#include "IQTVideoSurface.h"
#include "..\..\..\include\moreuuids.h"

#include "MacrovisionKicker.h"
#include "IPinHook.h"

#include "PixelShaderCompiler.h"
#include "MainFrm.h"

#include "AllocatorCommon.h"


typedef enum 
{
	MSG_MIXERIN,
	MSG_MIXEROUT
} EVR_STATS_MSG;


// dxva.dll
typedef HRESULT (__stdcall *PTR_DXVA2CreateDirect3DDeviceManager9)(UINT* pResetToken, IDirect3DDeviceManager9** ppDeviceManager);

// mf.dll
typedef HRESULT (__stdcall *PTR_MFCreatePresentationClock)(IMFPresentationClock** ppPresentationClock);

// evr.dll
typedef HRESULT (__stdcall *PTR_MFCreateDXSurfaceBuffer)(REFIID riid, IUnknown* punkSurface, BOOL fBottomUpWhenLinear, IMFMediaBuffer** ppBuffer);
typedef HRESULT (__stdcall *PTR_MFCreateVideoSampleFromSurface)(IUnknown* pUnkSurface, IMFSample** ppSample);
typedef HRESULT (__stdcall *PTR_MFCreateVideoMediaType)(const MFVIDEOFORMAT* pVideoFormat, IMFVideoMediaType** ppIVideoMediaType);



// mfplat.dll
//typedef HRESULT (__stdcall *PTR_MFCreateSample)(IMFSample** ppIMFSample);
//typedef HRESULT (__stdcall *PTR_MFCreateSystemTimeSource)(IMFPresentationTimeSource** ppSystemTimeSource);

// AVRT.dll
typedef HANDLE  (__stdcall *PTR_AvSetMmThreadCharacteristicsW)(LPCWSTR TaskName, LPDWORD TaskIndex);
typedef BOOL	(__stdcall *PTR_AvSetMmThreadPriority)(HANDLE AvrtHandle, AVRT_PRIORITY Priority);
typedef BOOL	(__stdcall *PTR_AvRevertMmThreadCharacteristics)(HANDLE AvrtHandle);


// === Helper functions
#define CheckHR(exp) {if(FAILED(hr = exp)) return hr;}

MFOffset MakeOffset(float v)
{
    MFOffset offset;
    offset.value = short(v);
    offset.fract = WORD(65536 * (v-offset.value));
    return offset;
}

MFVideoArea MakeArea(float x, float y, DWORD width, DWORD height)
{
    MFVideoArea area;
    area.OffsetX = MakeOffset(x);
    area.OffsetY = MakeOffset(y);
    area.Area.cx = width;
    area.Area.cy = height;
    return area;
}


/// === Outer EVR

class COuterEVR
	: public CUnknown
	, public IVMRffdshow9
	, public IVMRMixerBitmap9
{
	CComPtr<IUnknown>	m_pEVR;
	VMR9AlphaBitmap*	m_pVMR9AlphaBitmap;

public:

	COuterEVR(const TCHAR* pName, LPUNKNOWN pUnk, HRESULT& hr, VMR9AlphaBitmap* pVMR9AlphaBitmap) : CUnknown(pName, pUnk)
	{
		hr = m_pEVR.CoCreateInstance(CLSID_EnhancedVideoRenderer, GetOwner());
		m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
	}

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv)
	{
		HRESULT hr;

		if(riid == __uuidof(IVMRMixerBitmap9))
			return GetInterface((IVMRMixerBitmap9*)this, ppv);

		hr = m_pEVR ? m_pEVR->QueryInterface(riid, ppv) : E_NOINTERFACE;
		if(m_pEVR && FAILED(hr))
		{
			if(riid == __uuidof(IVMRffdshow9)) // Support ffdshow queueing. We show ffdshow that this is patched Media Player Classic.
				return GetInterface((IVMRffdshow9*)this, ppv);
		}

		return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
	}

	// IVMRffdshow9
	STDMETHODIMP support_ffdshow()
	{
		queueu_ffdshow_support = true;
		return S_OK;
	}

	// IVMRMixerBitmap9
	STDMETHODIMP GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		memcpy (pBmpParms, m_pVMR9AlphaBitmap, sizeof(VMR9AlphaBitmap));
		return S_OK;
	}
	
	STDMETHODIMP SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
		m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
		return S_OK;
	}

	STDMETHODIMP UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms)
	{
		CheckPointer(pBmpParms, E_POINTER);
		memcpy (m_pVMR9AlphaBitmap, pBmpParms, sizeof(VMR9AlphaBitmap));
		m_pVMR9AlphaBitmap->dwFlags |= VMRBITMAP_UPDATE;
		return S_OK;
	}
};







class CEVRAllocatorPresenter : 
	public CDX9AllocatorPresenter,
	public IMFGetService,
	public IMFTopologyServiceLookupClient,
	public IMFVideoDeviceID,
	public IMFVideoPresenter,
	public IDirect3DDeviceManager9,

	public IMFAsyncCallback,
	public IQualProp,
	public IMFRateSupport				// Non mandatory EVR Presenter Interfaces (see later...)
/*	public IMFVideoPositionMapper,
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


	// IMFRateSupport
    STDMETHODIMP	GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate);
    STDMETHODIMP	GetFastestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate);
    STDMETHODIMP	IsRateSupported(BOOL fThin, float flRate, float *pflNearestSupportedRate);

	float			GetMaxRate(BOOL bThin);


	// IMFVideoPresenter
	STDMETHODIMP	ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam);
	STDMETHODIMP	GetCurrentMediaType(__deref_out  IMFVideoMediaType **ppMediaType);

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


	// IDirect3DDeviceManager9
	STDMETHODIMP	ResetDevice(IDirect3DDevice9 *pDevice,UINT resetToken);        
	STDMETHODIMP	OpenDeviceHandle(HANDLE *phDevice);
	STDMETHODIMP	CloseDeviceHandle(HANDLE hDevice);        
    STDMETHODIMP	TestDevice(HANDLE hDevice);
	STDMETHODIMP	LockDevice(HANDLE hDevice, IDirect3DDevice9 **ppDevice, BOOL fBlock);
	STDMETHODIMP	UnlockDevice(HANDLE hDevice, BOOL fSaveState);
	STDMETHODIMP	GetVideoService(HANDLE hDevice, REFIID riid, void **ppService);

protected :
	void			OnResetDevice();
private :

	typedef enum
	{
		Started,
		Stopped,
		Paused,
		Shutdown
	} RENDER_STATE;

	CComPtr<IMFClock>						m_pClock;
	CComPtr<IDirect3DDeviceManager9>		m_pD3DManager;
	CComPtr<IMFTransform>					m_pMixer;
	CComPtr<IMediaEventSink>				m_pSink;
	CComPtr<IMFVideoMediaType>				m_pMediaType;

	HANDLE									m_hEvtQuit;			// Stop rendering thread event
	HANDLE									m_hEvtPresent;		// Render next frame (cued order)
	HANDLE									m_hEvtFrameTimer;	// Render next frame (timer based)
	HANDLE									m_hEvtFlush;		// Discard all buffers

	HANDLE									m_hSemPicture;
	HANDLE									m_hSemSlot;
	int										m_nFreeSlot;
	bool									m_fUseInternalTimer;

	HANDLE									m_hThread[1];
	RENDER_STATE							m_nRenderState;
	CComPtr<IMFSample>						m_pMFSample[MAX_PICTURE_SLOTS];
	UINT									m_nResetToken;
	UINT									m_nWaitingSample;
	UINT									m_nStepCount;

	// Stats variable for IQualProp
	UINT									m_pcFrames;
	UINT									m_pcFramesDrawn;	// Retrieves the number of frames drawn since streaming started
	UINT									m_piAvgFrameRate;
	UINT									m_iJitter;
	UINT									m_piAvg;
	UINT									m_piDev;

	HRESULT									GetImageFromMixer();
	void									RenderThread();
	static DWORD WINAPI						PresentThread(LPVOID lpParam);
	void									ResetStats();
	void									StartWorkerThreads();
	void									StopWorkerThreads();
	HRESULT									CheckShutdown() const;
	void									CompleteFrameStep(bool bCancel);
	void									CheckWaitingSampleFromMixer();

	// === Media type negociation functions
	HRESULT									RenegotiateMediaType();
	HRESULT									IsMediaTypeSupported(IMFMediaType* pMixerType);
	HRESULT									CreateProposedOutputType(IMFMediaType* pMixerType, IMFMediaType** pType);
	HRESULT									SetMediaType(IMFMediaType* pType);

//	virtual HRESULT							AllocSurfaces();

	// === Functions pointers on Vista / .Net3 specifics library
	PTR_DXVA2CreateDirect3DDeviceManager9	pfDXVA2CreateDirect3DDeviceManager9;
//	PTR_MFCreateSystemTimeSource			pfMFCreateSystemTimeSource;
	PTR_MFCreateDXSurfaceBuffer				pfMFCreateDXSurfaceBuffer;
	PTR_MFCreateVideoSampleFromSurface		pfMFCreateVideoSampleFromSurface;
	PTR_MFCreateVideoMediaType				pfMFCreateVideoMediaType;
//	PTR_MFCreateSample						pfMFCreateSample;
											
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
	HMODULE		hLib;
	AppSettings& s = AfxGetAppSettings();

	if (FAILED (hr)) return;

	// Load EVR specifics DLLs
	hLib = LoadLibrary (L"dxva2.dll");
	pfDXVA2CreateDirect3DDeviceManager9	= hLib ? (PTR_DXVA2CreateDirect3DDeviceManager9) GetProcAddress (hLib, "DXVA2CreateDirect3DDeviceManager9") : NULL;
	m_nResetToken = 0;

//	hLib = LoadLibrary (L"mf.dll");
//	pfMFCreatePresentationClock = hLib ? (PTR_MFCreatePresentationClock) GetProcAddress (hLib, "MFCreatePresentationClock") : NULL;

//	hLib = LoadLibrary (L"mfplat.dll");
//	pfMFCreateSystemTimeSource	= hLib ? (PTR_MFCreateSystemTimeSource) GetProcAddress (hLib, "MFCreateSystemTimeSource") : NULL;
//	pfMFCreateSample			= hLib ? (PTR_MFCreateSample)			GetProcAddress (hLib, "MFCreateSample") : NULL;
	
	// Load EVR functions
	hLib = LoadLibrary (L"evr.dll");
	pfMFCreateDXSurfaceBuffer			= hLib ? (PTR_MFCreateDXSurfaceBuffer)			GetProcAddress (hLib, "MFCreateDXSurfaceBuffer") : NULL;
	pfMFCreateVideoSampleFromSurface	= hLib ? (PTR_MFCreateVideoSampleFromSurface)	GetProcAddress (hLib, "MFCreateVideoSampleFromSurface") : NULL;
	pfMFCreateVideoMediaType			= hLib ? (PTR_MFCreateVideoMediaType)			GetProcAddress (hLib, "MFCreateVideoMediaType") : NULL;

	if (!pfDXVA2CreateDirect3DDeviceManager9 || !pfMFCreateDXSurfaceBuffer || !pfMFCreateVideoSampleFromSurface || !pfMFCreateVideoMediaType)
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

	// Bufferize frame only with 3D texture!
	if (s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
		m_nNbDXSurface	= max (min (s.iEvrBuffers, MAX_PICTURE_SLOTS-2), 1);
	else
		m_nNbDXSurface = 1;

	ResetStats();
	m_nRenderState		= Shutdown;
	m_fUseInternalTimer	= false;
	m_nWaitingSample	= 0;
	m_nStepCount		= 0;
}

CEVRAllocatorPresenter::~CEVRAllocatorPresenter(void)
{
	StopWorkerThreads();	// If not already done...
	m_pMediaType	= NULL;
	m_pClock		= NULL;
	m_pD3DManager	= NULL;
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


HRESULT CEVRAllocatorPresenter::CheckShutdown() const 
{
    if (m_nRenderState == Shutdown)
    {
        return MF_E_SHUTDOWN;
    }
    else
    {
        return S_OK;
    }
}


void CEVRAllocatorPresenter::StartWorkerThreads()
{
	DWORD		dwThreadId;

	if (m_nRenderState == Shutdown)
	{
		m_hEvtQuit		= CreateEvent (NULL, TRUE, FALSE, NULL);
		m_hEvtPresent	= CreateEvent (NULL, FALSE, FALSE, NULL);
		m_hEvtFrameTimer= CreateEvent (NULL, FALSE, FALSE, NULL);
		m_hEvtFlush		= CreateEvent (NULL, TRUE, FALSE, NULL);
		m_hSemPicture	= CreateSemaphore(NULL, 0, m_nNbDXSurface, NULL);
		// Si possible un freeslot de moins pour éviter d'écraser l'image en cours sur un pause
		m_hSemSlot		= CreateSemaphore(NULL, max(1, m_nNbDXSurface-1), max (1, m_nNbDXSurface-1), NULL);
		m_nFreeSlot		= 0;
		m_nCurSurface	= m_nNbDXSurface-1;

		m_hThread[0]	= ::CreateThread(NULL, 0, PresentThread, (LPVOID)this, 0, &dwThreadId);

		m_nRenderState		= Stopped;
		TRACE ("Worker threads started...\n");
	}
}

void CEVRAllocatorPresenter::StopWorkerThreads()
{
	if (m_nRenderState != Shutdown)
	{
		int			i;

		SetEvent (m_hEvtFlush);
		SetEvent (m_hEvtQuit);
		if (WaitForMultipleObjects (countof(m_hThread), m_hThread, TRUE, 10000) == WAIT_TIMEOUT)
		{
			ASSERT (FALSE);
			for (i=0; i<countof(m_hThread); i++)
				TerminateThread (m_hThread[i], 0xDEAD);
		}

		for (i=0; i<m_nNbDXSurface; i++)
		{
			m_pMFSample[i] = NULL;
		}

		for (i=0; i<countof(m_hThread); i++)
			CloseHandle (m_hThread[i]);
		CloseHandle (m_hSemPicture);
		CloseHandle (m_hSemSlot);
		CloseHandle (m_hEvtPresent);
		CloseHandle (m_hEvtFrameTimer);
		CloseHandle (m_hEvtFlush);
		CloseHandle (m_hEvtQuit);

		TRACE ("Worker threads stopped...\n");
	}
	m_nRenderState = Shutdown;
}

STDMETHODIMP CEVRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
	HRESULT					hr = E_FAIL;

	do
	{
		CMacrovisionKicker*		pMK  = new CMacrovisionKicker(NAME("CMacrovisionKicker"), NULL);
		CComPtr<IUnknown>		pUnk = (IUnknown*)(INonDelegatingUnknown*)pMK;
		pMK->SetInner((IUnknown*)(INonDelegatingUnknown*)new COuterEVR(NAME("COuterEVR"), pUnk, hr, &m_VMR9AlphaBitmap));
		CComQIPtr<IBaseFilter>	pBF = pUnk;
		if (FAILED (hr)) break;

		// Set EVR custom presenter
		CComPtr<IMFVideoPresenter>		pVP;
		CComPtr<IMFVideoRenderer>		pMFVR;
		CComQIPtr<IMFGetService, &__uuidof(IMFGetService)> pMFGS = pBF;

		hr = pMFGS->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoRenderer, (void**)&pMFVR);
		if(SUCCEEDED(hr)) hr = QueryInterface (__uuidof(IMFVideoPresenter), (void**)&pVP);
		if(SUCCEEDED(hr)) hr = pMFVR->InitializeRenderer (NULL, pVP);

		CComPtr<IPin>			pPin = GetFirstPin(pBF);
		CComQIPtr<IMemInputPin> pMemInputPin = pPin;
		
		// No NewSegment : no chocolate :o)
		m_fUseInternalTimer = HookNewSegmentAndReceive((IPinC*)(IPin*)pPin, (IMemInputPinC*)(IMemInputPin*)pMemInputPin);

		if(FAILED(hr))
			*ppRenderer = NULL;
		else
			*ppRenderer = pBF.Detach();

	} while (0);

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
	else if(riid == __uuidof(IMFRateSupport))
		hr = GetInterface((IMFRateSupport*)this, ppv);
	else if(riid == __uuidof(IDirect3DDeviceManager9))
//		hr = GetInterface((IDirect3DDeviceManager9*)this, ppv);
		hr = m_pD3DManager->QueryInterface (__uuidof(IDirect3DDeviceManager9), (void**) ppv);
	else
		hr = __super::NonDelegatingQueryInterface(riid, ppv);

	return hr;
}


// IMFClockStateSink
STDMETHODIMP CEVRAllocatorPresenter::OnClockStart(MFTIME hnsSystemTime,  LONGLONG llClockStartOffset)
{
	m_nRenderState		= Started;

	SetEvent(m_hEvtPresent);
	TRACE ("OnClockStart  hnsSystemTime = %I64d,   llClockStartOffset = %I64d\n", hnsSystemTime, llClockStartOffset);

	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockStop(MFTIME hnsSystemTime)
{
	TRACE ("OnClockStop  hnsSystemTime = %I64d\n", hnsSystemTime);
	m_nRenderState		= Stopped;

	return E_NOTIMPL;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockPause(MFTIME hnsSystemTime)
{
	TRACE ("OnClockPause  hnsSystemTime = %I64d\n", hnsSystemTime);
	m_nRenderState		= Paused;
	
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::OnClockRestart(MFTIME hnsSystemTime)
{
	m_nRenderState	= Started;
	SetEvent(m_hEvtPresent);

	TRACE ("OnClockRestart  hnsSystemTime = %I64d\n", hnsSystemTime);

	return S_OK;
}


STDMETHODIMP CEVRAllocatorPresenter::OnClockSetRate(MFTIME hnsSystemTime, float flRate)
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


// IMFRateSupport
STDMETHODIMP CEVRAllocatorPresenter::GetSlowestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate)
{
	// TODO : not finished...
	*pflRate = 0;
	return S_OK;
}
    
STDMETHODIMP CEVRAllocatorPresenter::GetFastestRate(MFRATE_DIRECTION eDirection, BOOL fThin, float *pflRate)
{
	HRESULT		hr = S_OK;
	float		fMaxRate = 0.0f;

	CAutoLock lock(this);

	CheckPointer(pflRate, E_POINTER);
	CheckHR(CheckShutdown());
    
	// Get the maximum forward rate.
	fMaxRate = GetMaxRate(fThin);

	// For reverse playback, swap the sign.
	if (eDirection == MFRATE_REVERSE)
		fMaxRate = -fMaxRate;

	*pflRate = fMaxRate;

	return hr;
}
    
STDMETHODIMP CEVRAllocatorPresenter::IsRateSupported(BOOL fThin, float flRate, float *pflNearestSupportedRate)
{
    // fRate can be negative for reverse playback.
    // pfNearestSupportedRate can be NULL.

    CAutoLock lock(this);

    HRESULT hr = S_OK;
    float   fMaxRate = 0.0f;
    float   fNearestRate = flRate;   // Default.

	CheckPointer (pflNearestSupportedRate, E_POINTER);
    CheckHR(hr = CheckShutdown());

    // Find the maximum forward rate.
    fMaxRate = GetMaxRate(fThin);

    if (fabsf(flRate) > fMaxRate)
    {
        // The (absolute) requested rate exceeds the maximum rate.
        hr = MF_E_UNSUPPORTED_RATE;

        // The nearest supported rate is fMaxRate.
        fNearestRate = fMaxRate;
        if (flRate < 0)
        {
            // For reverse playback, swap the sign.
            fNearestRate = -fNearestRate;
        }
    }

    // Return the nearest supported rate if the caller requested it.
    if (pflNearestSupportedRate != NULL)
        *pflNearestSupportedRate = fNearestRate;

    return hr;
}


float CEVRAllocatorPresenter::GetMaxRate(BOOL bThin)
{
	float   fMaxRate		= FLT_MAX;  // Default.
	UINT32  fpsNumerator	= 0, fpsDenominator = 0;
	UINT    MonitorRateHz	= 0; 

	if (!bThin && (m_pMediaType != NULL))
	{
		// Non-thinned: Use the frame rate and monitor refresh rate.
        
		// Frame rate:
		MFGetAttributeRatio(m_pMediaType, MF_MT_FRAME_RATE, 
			&fpsNumerator, &fpsDenominator);

		// Monitor refresh rate:
		MonitorRateHz = m_RefreshRate; // D3DDISPLAYMODE

		if (fpsDenominator && fpsNumerator && MonitorRateHz)
		{
			// Max Rate = Refresh Rate / Frame Rate
			fMaxRate = (float)MulDiv(
				MonitorRateHz, fpsDenominator, fpsNumerator);
		}
	}
	return fMaxRate;
}

void CEVRAllocatorPresenter::CompleteFrameStep(bool bCancel)
{
	if (m_nStepCount > 0)
	{
		if (bCancel || (m_nStepCount == 1)) 
		{
			m_pSink->Notify(EC_STEP_COMPLETE, bCancel ? TRUE : FALSE, 0);
			m_nStepCount = 0;
		}
		else
			m_nStepCount--;
	}
}

// IMFVideoPresenter
STDMETHODIMP CEVRAllocatorPresenter::ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam)
{
	HRESULT						hr = S_OK;

	switch (eMessage)
	{
	case MFVP_MESSAGE_BEGINSTREAMING :			// The EVR switched from stopped to paused. The presenter should allocate resources
		ResetStats();		
		TRACE ("MFVP_MESSAGE_BEGINSTREAMING\n");
		SetEvent(m_hEvtPresent);
		break;

	case MFVP_MESSAGE_CANCELSTEP :				// Cancels a frame step
		TRACE ("MFVP_MESSAGE_CANCELSTEP\n");
		CompleteFrameStep (true);
		break;

	case MFVP_MESSAGE_ENDOFSTREAM :				// All input streams have ended. 
		TRACE ("MFVP_MESSAGE_ENDOFSTREAM\n");
		m_pSink->Notify (EC_COMPLETE, 0, 0);
		break;

	case MFVP_MESSAGE_ENDSTREAMING :			// The EVR switched from running or paused to stopped. The presenter should free resources
		TRACE ("MFVP_MESSAGE_ENDSTREAMING\n");
		break;

	case MFVP_MESSAGE_FLUSH :					// The presenter should discard any pending samples
		SetEvent(m_hEvtFlush);
		TRACE ("MFVP_MESSAGE_FLUSH\n");
		while (WaitForSingleObject(m_hEvtFlush, 1) == WAIT_OBJECT_0);
		SetEvent(m_hEvtPresent);
		break;

	case MFVP_MESSAGE_INVALIDATEMEDIATYPE :		// The mixer's output format has changed. The EVR will initiate format negotiation, as described previously
		/*
			1) The EVR sets the media type on the reference stream.
			2) The EVR calls IMFVideoPresenter::ProcessMessage on the presenter with the MFVP_MESSAGE_INVALIDATEMEDIATYPE message.
			3) The presenter sets the media type on the mixer's output stream.
			4) The EVR sets the media type on the substreams.
		*/
		RenegotiateMediaType();		
		break;

	case MFVP_MESSAGE_PROCESSINPUTNOTIFY :		// One input stream on the mixer has received a new sample
		TRACE ("=>MFVP_MESSAGE_PROCESSINPUTNOTIFY\n");
		GetImageFromMixer();
		break;

	case MFVP_MESSAGE_STEP :					// Requests a frame step.
		TRACE ("MFVP_MESSAGE_STEP\n");
		m_nStepCount = ulParam;
		hr = S_OK;
		break;

	default :
		ASSERT (FALSE);
		break;
	}
	return hr;
}


HRESULT CEVRAllocatorPresenter::IsMediaTypeSupported(IMFMediaType* pMixerType)
{
	HRESULT				hr;
	AM_MEDIA_TYPE*		pAMMedia;
	UINT				nInterlaceMode;

	CheckHR (pMixerType->GetRepresentation(FORMAT_VideoInfo2, (void**)&pAMMedia));
	CheckHR (pMixerType->GetUINT32 (MF_MT_INTERLACE_MODE, &nInterlaceMode));


	if ( (pAMMedia->majortype != MEDIATYPE_Video) ||
		 (nInterlaceMode != MFVideoInterlace_Progressive) ||
		 ( (pAMMedia->subtype != MEDIASUBTYPE_RGB32) && (pAMMedia->subtype != MEDIASUBTYPE_RGB24) &&
		   (pAMMedia->subtype != MEDIASUBTYPE_YUY2)  && (pAMMedia->subtype != MEDIASUBTYPE_NV12) ) )
		   hr = MF_E_INVALIDMEDIATYPE;
	pMixerType->FreeRepresentation (FORMAT_VideoInfo2, (void*)pAMMedia);
	return hr;
}

HRESULT CEVRAllocatorPresenter::CreateProposedOutputType(IMFMediaType* pMixerType, IMFMediaType** pType)
{
	HRESULT				hr;
	AM_MEDIA_TYPE*		pAMMedia = NULL;
	LARGE_INTEGER		i64Size;
	MFVIDEOFORMAT*		VideoFormat;

	CheckHR (pMixerType->GetRepresentation  (FORMAT_MFVideoFormat, (void**)&pAMMedia));
	VideoFormat = (MFVIDEOFORMAT*)pAMMedia->pbFormat;
	hr = pfMFCreateVideoMediaType  (VideoFormat, &m_pMediaType);

	if (SUCCEEDED (hr))
	{
		i64Size.HighPart = VideoFormat->videoInfo.dwWidth;
		i64Size.LowPart	 = VideoFormat->videoInfo.dwHeight;
		m_pMediaType->SetUINT64 (MF_MT_FRAME_SIZE, i64Size.QuadPart);

		m_pMediaType->SetUINT32 (MF_MT_PAN_SCAN_ENABLED, 0);

		i64Size.HighPart = 1;
		i64Size.LowPart  = 1;
		m_pMediaType->SetUINT64 (MF_MT_PIXEL_ASPECT_RATIO, i64Size.QuadPart);

		MFVideoArea Area = MakeArea (0, 0, VideoFormat->videoInfo.dwWidth, VideoFormat->videoInfo.dwHeight);
		m_pMediaType->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&Area, sizeof(MFVideoArea));
	}

	pMixerType->FreeRepresentation (FORMAT_MFVideoFormat, (void*)pAMMedia);
	m_pMediaType->QueryInterface (__uuidof(IMFMediaType), (void**) pType);

	return hr;
}

HRESULT CEVRAllocatorPresenter::SetMediaType(IMFMediaType* pType)
{
	HRESULT				hr;
	AM_MEDIA_TYPE*		pAMMedia = NULL;
	CString				strTemp;

	CheckPointer (pType, E_POINTER);
	CheckHR (pType->GetRepresentation(FORMAT_VideoInfo2, (void**)&pAMMedia));
	
	hr = InitializeDevice (pAMMedia);
	if (SUCCEEDED (hr))
	{
		strTemp = GetMediaTypeName (pAMMedia->subtype);
		strTemp.Replace (L"MEDIASUBTYPE_", L"");
		m_strStatsMsg[MSG_MIXEROUT].Format (L"Mixer output : %s", strTemp);
	}

	pType->FreeRepresentation (FORMAT_VideoInfo2, (void*)pAMMedia);

	return hr;
}

HRESULT CEVRAllocatorPresenter::RenegotiateMediaType()
{
    HRESULT			hr = S_OK;
    BOOL			fFoundMediaType = FALSE;

    CComPtr<IMFMediaType>	pMixerType;
    CComPtr<IMFMediaType>	pType;

    if (!m_pMixer)
    {
        return MF_E_INVALIDREQUEST;
    }

    // Loop through all of the mixer's proposed output types.
    DWORD iTypeIndex = 0;
    while (!fFoundMediaType && (hr != MF_E_NO_MORE_TYPES))
    {
        pMixerType	 = NULL;
        pType		 = NULL;
		m_pMediaType = NULL;

        // Step 1. Get the next media type supported by mixer.
        hr = m_pMixer->GetOutputAvailableType(0, iTypeIndex++, &pMixerType);
        if (FAILED(hr))
        {
            break;
        }

        // Step 2. Check if we support this media type.
        if (SUCCEEDED(hr))
        {
            hr = IsMediaTypeSupported(pMixerType);
        }

        // Step 3. Adjust the mixer's type to match our requirements.
        if (SUCCEEDED(hr))
        {
            hr = CreateProposedOutputType(pMixerType, &pType);
        }

        // Step 4. Check if the mixer will accept this media type.
        if (SUCCEEDED(hr))
        {
            hr = m_pMixer->SetOutputType(0, pType, MFT_SET_TYPE_TEST_ONLY);
        }

        // Step 5. Try to set the media type on ourselves.
        if (SUCCEEDED(hr))
        {
            hr = SetMediaType(pType);
        }

        // Step 6. Set output media type on mixer.
        if (SUCCEEDED(hr))
        {
            hr = m_pMixer->SetOutputType(0, pType, 0);

            // If something went wrong, clear the media type.
            if (FAILED(hr))
            {
                SetMediaType(NULL);
            }
        }

        if (SUCCEEDED(hr))
        {
            fFoundMediaType = TRUE;
        }
    }

    pMixerType	= NULL;
    pType		= NULL;
    return hr;
}


//HRESULT CEVRAllocatorPresenter::AllocRessources()
//{
//	HRESULT			hr;
//	int				i;
//
//	// Create Media Foundation buffers for EVR Mixer
//	for (i=0; i<m_nNbDXSurface; i++)
//	{
//		m_pMFSample[i] = NULL;
//
//		if (m_pVideoSurface[i] == NULL) return E_POINTER;
//
//		hr = pfMFCreateVideoSampleFromSurface (m_pVideoSurface[i], &m_pMFSample[i]);
//	}
//
//	return hr;
//}

HRESULT CEVRAllocatorPresenter::GetImageFromMixer()
{
	MFT_OUTPUT_DATA_BUFFER		Buffer;
	HRESULT						hr = S_FALSE;
	DWORD						dwStatus;
	REFERENCE_TIME				nsSampleTime;
	HANDLE						hEvts[] = { /*m_hEvtFlush ,*/ m_hSemSlot };
	MFTIME						nsCurrentTime;
	LONGLONG					llClockBefore = 0;
	LONGLONG					llClockAfter  = 0;
	LONGLONG					llMixerLatency;

	// Get image from Mixer until sample queue is full
	if (WaitForMultipleObjects (countof(hEvts), hEvts, FALSE, 50) == WAIT_OBJECT_0)
	{
		memset (&Buffer, 0, sizeof(Buffer));
		m_pMFSample[m_nFreeSlot] = NULL;
		hr = pfMFCreateVideoSampleFromSurface (m_pVideoSurface[m_nFreeSlot], &m_pMFSample[m_nFreeSlot]);
		Buffer.pSample = m_pMFSample[m_nFreeSlot];

		if (m_pClock) m_pClock->GetCorrelatedTime(0, &llClockBefore, &nsCurrentTime);			
		hr = m_pMixer->ProcessOutput (0 , 1, &Buffer, &dwStatus);
		if (m_pClock) m_pClock->GetCorrelatedTime(0, &llClockAfter, &nsCurrentTime);
		llMixerLatency = llClockAfter - llClockBefore;
		if (m_pSink) 
		{
//			CAutoLock autolock(this);
			m_pSink->Notify (EC_PROCESSING_LATENCY, (LONG_PTR)&llMixerLatency, 0);
		}

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
	
		TRACE ("New image from muxer Slot %d : %I64d\n", m_nFreeSlot, nsSampleTime/417188);

		m_nFreeSlot = (m_nFreeSlot+1) % m_nNbDXSurface;
		ReleaseSemaphore (m_hSemPicture, 1, NULL);

		InterlockedIncrement (&m_nUsedBuffer);
	}
	else
		m_nWaitingSample++;

	return hr;
}



STDMETHODIMP CEVRAllocatorPresenter::GetCurrentMediaType(__deref_out  IMFVideoMediaType **ppMediaType)
{
    HRESULT hr = S_OK;
    CAutoLock lock(this);  // Hold the critical section.

    CheckPointer (ppMediaType, E_POINTER);
    CheckHR (CheckShutdown());

    if (m_pMediaType == NULL)
        CheckHR(MF_E_NOT_INITIALIZED);

    CheckHR(m_pMediaType->QueryInterface( __uuidof(IMFVideoMediaType), (void**)&ppMediaType));

    return hr;
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

	hr = pLookup->LookupService (MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE,
								  __uuidof (IMFClock ), (void**)&m_pClock, &dwObjects);

	StartWorkerThreads();
	return S_OK;
}

STDMETHODIMP CEVRAllocatorPresenter::ReleaseServicePointers()
{
	TRACE ("EVR : CEVRAllocatorPresenter::ReleaseServicePointers\n");
	StopWorkerThreads();
	m_pMixer	= NULL;
	m_pSink		= NULL;
	m_pClock	= NULL;
	return S_OK;
}


// IMFVideoDeviceID
STDMETHODIMP CEVRAllocatorPresenter::GetDeviceID(/* [out] */	__out  IID *pDeviceID)
{
	CheckPointer(pDeviceID, E_POINTER);
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


// IDirect3DDeviceManager9
STDMETHODIMP CEVRAllocatorPresenter::ResetDevice(IDirect3DDevice9 *pDevice,UINT resetToken)
{
	HRESULT		hr = m_pD3DManager->ResetDevice (pDevice, resetToken);
	return hr;
}
STDMETHODIMP CEVRAllocatorPresenter::OpenDeviceHandle(HANDLE *phDevice)
{
	HRESULT		hr = m_pD3DManager->OpenDeviceHandle (phDevice);
	return hr;
}
STDMETHODIMP CEVRAllocatorPresenter::CloseDeviceHandle(HANDLE hDevice)
{
	HRESULT		hr = m_pD3DManager->CloseDeviceHandle(hDevice);
	return hr;
}
STDMETHODIMP CEVRAllocatorPresenter::TestDevice(HANDLE hDevice)
{
	HRESULT		hr = m_pD3DManager->TestDevice(hDevice);
	return hr;
}
STDMETHODIMP CEVRAllocatorPresenter::LockDevice(HANDLE hDevice, IDirect3DDevice9 **ppDevice, BOOL fBlock)
{
	HRESULT		hr = m_pD3DManager->LockDevice(hDevice, ppDevice, fBlock);
	return hr;
}
STDMETHODIMP CEVRAllocatorPresenter::UnlockDevice(HANDLE hDevice, BOOL fSaveState)
{
	HRESULT		hr = m_pD3DManager->UnlockDevice(hDevice, fSaveState);
	return hr;
}
STDMETHODIMP CEVRAllocatorPresenter::GetVideoService(HANDLE hDevice, REFIID riid, void **ppService)
{
	HRESULT		hr = m_pD3DManager->GetVideoService(hDevice, riid, ppService);

	if (riid == __uuidof(IDirectXVideoDecoderService))
	{
		UINT		nNbDecoder = 5;
		GUID*		pDecoderGuid;
		IDirectXVideoDecoderService*		pDXVAVideoDecoder = (IDirectXVideoDecoderService*) *ppService;
		pDXVAVideoDecoder->GetDecoderDeviceGuids (&nNbDecoder, &pDecoderGuid);
	}
	else if (riid == __uuidof(IDirectXVideoProcessorService))
	{
		IDirectXVideoProcessorService*		pDXVAProcessor = (IDirectXVideoProcessorService*) *ppService;
//		pDXVAProcessor->GetVideoProcessorRenderTargets(
	}

	return hr;
}


STDMETHODIMP CEVRAllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	if(lpWidth)		*lpWidth	= m_NativeVideoSize.cx;
	if(lpHeight)	*lpHeight	= m_NativeVideoSize.cy;
	if(lpARWidth)	*lpARWidth	= m_AspectRatio.cx;
	if(lpARHeight)	*lpARHeight	= m_AspectRatio.cy;
	return S_OK;
}


/*
HRESULT CEVRAllocatorPresenter::AllocSurfaces()
{
    CAutoLock cAutoLock(this);
	AppSettings& s = AfxGetAppSettings();

	HANDLE										hDevice;
	CComPtr<IDirectXVideoAccelerationService>	pDXVAService;
	HRESULT hr = m_pD3DManager->OpenDeviceHandle (&hDevice);
	hr = m_pD3DManager->GetVideoService (hDevice, __uuidof(IDirectXVideoAccelerationService), (void**)&pDXVAService);

	for(int i = 0; i < m_nNbDXSurface+2; i++)
	{
		m_pVideoTexture[i] = NULL;
		m_pVideoSurface[i] = NULL;
	}

	m_pResizerBicubic1stPass = NULL;

	if(s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE2D || s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
	{
		int nTexturesNeeded = s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D ? m_nNbDXSurface+2 : 1;

		for(int i = 0; i < nTexturesNeeded; i++)
		{
			hr = pDXVAService->CreateSurface (m_NativeVideoSize.cx, m_NativeVideoSize.cy, 0, D3DFMT_A8R8G8B8, 
				D3DPOOL_DEFAULT, 0, DXVA2_VideoProcessorRenderTarget, &m_pVideoSurface[i], NULL);

		}
	}
	else
	{
		if(FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(
			m_NativeVideoSize.cx, m_NativeVideoSize.cy, 
			D3DFMT_X8R8G8B8, 
			D3DPOOL_DEFAULT, &m_pVideoSurface[m_nCurSurface], NULL)))
			return hr;
	}

	hr = m_pD3DDev->ColorFill(m_pVideoSurface[m_nCurSurface], NULL, 0);

	return S_OK;
}
*/



STDMETHODIMP CEVRAllocatorPresenter::InitializeDevice(AM_MEDIA_TYPE*	pMediaType)
{
	HRESULT			hr;

	DeleteSurfaces();

	VIDEOINFOHEADER2*		vih2 = (VIDEOINFOHEADER2*) pMediaType->pbFormat;
	int						w = vih2->bmiHeader.biWidth;
	int						h = abs(vih2->bmiHeader.biHeight);

	m_NativeVideoSize = m_AspectRatio = CSize(w, h);
	int arx = vih2->dwPictAspectRatioX, ary = vih2->dwPictAspectRatioY;
	if(arx > 0 && ary > 0) m_AspectRatio.SetSize(arx, ary);

	hr = AllocSurfaces();

	return hr;
}



DWORD WINAPI CEVRAllocatorPresenter::PresentThread(LPVOID lpParam)
{
	CEVRAllocatorPresenter*		pThis = (CEVRAllocatorPresenter*) lpParam;
	pThis->RenderThread();
	return 0;
}


void CEVRAllocatorPresenter::CheckWaitingSampleFromMixer()
{
	if (m_nWaitingSample > 0)
	{
		m_nWaitingSample--;
		GetImageFromMixer();
	}
}


void CEVRAllocatorPresenter::RenderThread()
{
	HANDLE				hAvrt;
	DWORD				dwTaskIndex	= 0;
	HANDLE				hEvts[]		= { m_hEvtQuit, m_hEvtFlush, m_hEvtPresent, m_hEvtFrameTimer};
	HANDLE				hEvtsBuff[]	= { m_hEvtQuit, m_hEvtFlush, m_hSemPicture };
	bool				bQuit		= false;
    TIMECAPS			tc;
	DWORD				dwResolution;
	MFTIME				nsSampleTime;
	MFTIME				nsCurrentTime;
	LONGLONG			llClockTime;
	long				lDelay;
	DWORD				dwUser = 0;
	DWORD				dwObject;

	LONGLONG			llPerfTotalPlay	= 0;
	LONGLONG			llPerfLastFrame	= 0;
	UINT				nNbPlayingFrame	= 0;

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
			TRACE ("Begin flush\n");
			// Flush pending samples!
			if (dwUser != -1) timeKillEvent (dwUser);
			dwUser = -1;		

			while (WaitForSingleObject (m_hSemPicture, 0) == WAIT_OBJECT_0)
			{
				ReleaseSemaphore (m_hSemSlot, 1, NULL);
				m_nCurSurface = (m_nCurSurface + 1) % m_nNbDXSurface;
				TRACE ("Free slot\n");
				CheckWaitingSampleFromMixer();
			}

			m_nUsedBuffer = 0;
			ResetEvent(m_hEvtFlush);
			TRACE ("====>>> End flush  Cons=%d  Prod=%d\n", m_nCurSurface, m_nFreeSlot);
			break;

		case WAIT_OBJECT_0 + 2 :
		case WAIT_OBJECT_0 + 3 :
			// Discard timer events if playback stop
			if ((dwObject == WAIT_OBJECT_0 + 3) && (m_nRenderState != Started)) continue;

//			TRACE ("RenderThread ==>> Waiting buffer\n");
			
			if (WaitForMultipleObjects (countof(hEvtsBuff), hEvtsBuff, FALSE, INFINITE) == WAIT_OBJECT_0+2)
			{
				m_nCurSurface = (m_nCurSurface + 1) % m_nNbDXSurface;
				TRACE ("RenderThread ==>> Presenting Cons=%d  Prod=%d\n", m_nCurSurface, m_nFreeSlot);
				Paint(true);
				m_pcFramesDrawn++;
				InterlockedDecrement (&m_nUsedBuffer);
				CompleteFrameStep (false);

				if ((m_nRenderState == Started) && (m_nStepCount == 0))
				{
					// Calculate wake up timer
					m_pClock->GetCorrelatedTime(0, &llClockTime, &nsCurrentTime);			
					m_pMFSample[m_nCurSurface]->GetSampleTime (&nsSampleTime);

					EstimateFrameRate(nsSampleTime);
					// Wakup 1/2 refresh rate before next VSync!
					lDelay = (UINT)((nsSampleTime + m_rtTimePerFrame - llClockTime) / 10000) - (500/m_RefreshRate);

					if (lDelay > 0)
					{
						TRACE ("RenderThread ==>> Set timer %d   %I64d  Cons=%d  Prod=%d\n", lDelay, nsSampleTime, m_nCurSurface, m_nFreeSlot);
						dwUser			= timeSetEvent (lDelay, dwResolution, (LPTIMECALLBACK)m_hEvtFrameTimer, NULL, TIME_CALLBACK_EVENT_SET); 

						// Update statistics
						LONGLONG		llPerfCurrent = AfxGetMyApp()->GetPerfCounter();
						
						llPerfTotalPlay	+= (llPerfCurrent - llPerfLastFrame);
						nNbPlayingFrame++;
						m_piAvgFrameRate = (UINT)(Int32x32To64(nNbPlayingFrame, 1000000000) / llPerfTotalPlay);
						m_iJitter = (UINT)(((llPerfCurrent - llPerfLastFrame) - m_rtTimePerFrame)/10000);

						llPerfLastFrame	 = llPerfCurrent;
					}
					else
					{
						dwUser = -1;
						if (m_nRenderState == Started) m_pcFrames++;
						TRACE ("RenderThread ==>> immediate display   %I64d  (delay=%d)  Cons=%d  Prod=%d\n", nsSampleTime/417188, lDelay, m_nCurSurface, m_nFreeSlot);
						SetEvent (m_hEvtPresent);
					}
				}

				CheckWaitingSampleFromMixer();
				ReleaseSemaphore (m_hSemSlot, 1, NULL);
//				TRACE ("RenderThread ==>> Sleeping\n");
			}
			else
			{				
				TRACE ("RenderThread ==>> Flush before rendering frame!\n");
			}

			break;
		}
	}

	timeEndPeriod (dwResolution);
	if (pfAvRevertMmThreadCharacteristics) pfAvRevertMmThreadCharacteristics (hAvrt);
}

void CEVRAllocatorPresenter::OnResetDevice()
{
//	UINT		m_nResetToken = 0;
	HRESULT		hr;

	// Reset DXVA Manager, and get new buffers
	hr = m_pD3DManager->ResetDevice(m_pD3DDev, m_nResetToken);
//	AllocRessources();

	// Not necessary, but Microsoft documentation say Presenter should send this message...
	if (m_pSink)
		m_pSink->Notify (EC_DISPLAY_CHANGED, 0, 0);
}
