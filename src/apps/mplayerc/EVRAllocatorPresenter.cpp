/*
 * $Id$
 *
 * (C) 2006-2007 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <moreuuids.h>

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

// AVRT.dll
typedef HANDLE  (__stdcall *PTR_AvSetMmThreadCharacteristicsW)(LPCWSTR TaskName, LPDWORD TaskIndex);
typedef BOOL	(__stdcall *PTR_AvSetMmThreadPriority)(HANDLE AvrtHandle, AVRT_PRIORITY Priority);
typedef BOOL	(__stdcall *PTR_AvRevertMmThreadCharacteristics)(HANDLE AvrtHandle);


// Guid to tag IMFSample with DirectX surface index
static const GUID GUID_SURFACE_INDEX = { 0x30c8e9f6, 0x415, 0x4b81, { 0xa3, 0x15, 0x1, 0xa, 0xc6, 0xa9, 0xda, 0x19 } };


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
	public IMFRateSupport,				
	public IMFVideoDisplayControl,
	public IEVRTrustedVideoPlugin
/*	public IMFVideoPositionMapper,		// Non mandatory EVR Presenter Interfaces (see later...)
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

	// IMFVideoDisplayControl
    STDMETHODIMP GetNativeVideoSize(SIZE *pszVideo, SIZE *pszARVideo);    
    STDMETHODIMP GetIdealVideoSize(SIZE *pszMin, SIZE *pszMax);
    STDMETHODIMP SetVideoPosition(const MFVideoNormalizedRect *pnrcSource, const LPRECT prcDest);
    STDMETHODIMP GetVideoPosition(MFVideoNormalizedRect *pnrcSource, LPRECT prcDest);
    STDMETHODIMP SetAspectRatioMode(DWORD dwAspectRatioMode);
    STDMETHODIMP GetAspectRatioMode(DWORD *pdwAspectRatioMode);
    STDMETHODIMP SetVideoWindow(HWND hwndVideo);
    STDMETHODIMP GetVideoWindow(HWND *phwndVideo);
    STDMETHODIMP RepaintVideo( void);
    STDMETHODIMP GetCurrentImage(BITMAPINFOHEADER *pBih, BYTE **pDib, DWORD *pcbDib, LONGLONG *pTimeStamp);
    STDMETHODIMP SetBorderColor(COLORREF Clr);
    STDMETHODIMP GetBorderColor(COLORREF *pClr);
    STDMETHODIMP SetRenderingPrefs(DWORD dwRenderFlags);
    STDMETHODIMP GetRenderingPrefs(DWORD *pdwRenderFlags);
    STDMETHODIMP SetFullscreen(BOOL fFullscreen);
    STDMETHODIMP GetFullscreen(BOOL *pfFullscreen);

	// IEVRTrustedVideoPlugin
    STDMETHODIMP IsInTrustedVideoMode(BOOL *pYes);
    STDMETHODIMP CanConstrict(BOOL *pYes);
    STDMETHODIMP SetConstriction(DWORD dwKPix);
    STDMETHODIMP DisableImageExport(BOOL bDisable);

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
	MFVideoAspectRatioMode					m_dwVideoAspectRatioMode;
	MFVideoRenderPrefs						m_dwVideoRenderPrefs;
	COLORREF								m_BorderColor;


	HANDLE									m_hEvtQuit;			// Stop rendering thread event
	HANDLE									m_hEvtPresent;		// Render next frame (cued order)
	HANDLE									m_hEvtFrameTimer;	// Render next frame (timer based)
	HANDLE									m_hEvtFlush;		// Discard all buffers
	HANDLE									m_hSemPicture;		// Indicate present of buffered frames

	bool									m_fUseInternalTimer;

	HANDLE									m_hThread;
	RENDER_STATE							m_nRenderState;
	
	CInterfaceList<IMFSample, &IID_IMFSample>		m_FreeSamples;
	CInterfaceList<IMFSample, &IID_IMFSample>		m_ScheduledSamples;
	bool									m_bWaitingSample;

	UINT									m_nResetToken;
	UINT									m_nStepCount;

	// Stats variable for IQualProp
	UINT									m_pcFrames;
	UINT									m_pcFramesDrawn;	// Retrieves the number of frames drawn since streaming started
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

	void									RemoveAllSamples();
	HRESULT									GetFreeSample(IMFSample** ppSample);
	HRESULT									GetScheduledSample(IMFSample** ppSample);
	void									MoveToFreeList(IMFSample* pSample);
	void									MoveToScheduledList(IMFSample* pSample);
	void									FlushSamples();

	// === Media type negociation functions
	HRESULT									RenegotiateMediaType();
	HRESULT									IsMediaTypeSupported(IMFMediaType* pMixerType);
	HRESULT									CreateProposedOutputType(IMFMediaType* pMixerType, IMFMediaType** pType);
	HRESULT									SetMediaType(IMFMediaType* pType);

	// === Functions pointers on Vista / .Net3 specifics library
	PTR_DXVA2CreateDirect3DDeviceManager9	pfDXVA2CreateDirect3DDeviceManager9;
	PTR_MFCreateDXSurfaceBuffer				pfMFCreateDXSurfaceBuffer;
	PTR_MFCreateVideoSampleFromSurface		pfMFCreateVideoSampleFromSurface;
	PTR_MFCreateVideoMediaType				pfMFCreateVideoMediaType;
											
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
	m_nResetToken	 = 0;
	m_hThread		 = INVALID_HANDLE_VALUE;
	m_hSemPicture	 = INVALID_HANDLE_VALUE;
	m_hEvtPresent	 = INVALID_HANDLE_VALUE;
	m_hEvtFrameTimer = INVALID_HANDLE_VALUE;
	m_hEvtFlush		 = INVALID_HANDLE_VALUE;
	m_hEvtQuit		 = INVALID_HANDLE_VALUE;
	
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

	CComPtr<IDirectXVideoDecoderService>	pDecoderService;
	HANDLE							hDevice;
	if (SUCCEEDED (m_pD3DManager->OpenDeviceHandle(&hDevice)) &&
		SUCCEEDED (m_pD3DManager->GetVideoService (hDevice, __uuidof(IDirectXVideoDecoderService), (void**)&pDecoderService)))
	{
		TRACE ("DXVA2 : device handle = 0x%08x", hDevice);
		HookDirectXVideoDecoderService (pDecoderService);

		m_pD3DManager->CloseDeviceHandle (hDevice);
	}


	// Bufferize frame only with 3D texture!
	if (s.iAPSurfaceUsage == VIDRNDT_AP_TEXTURE3D)
		m_nNbDXSurface	= max (min (s.iEvrBuffers, MAX_PICTURE_SLOTS-2), 1);
	else
		m_nNbDXSurface = 1;

	ResetStats();
	m_nRenderState				= Shutdown;
	m_fUseInternalTimer			= false;
	m_bWaitingSample			= false;
	m_nStepCount				= 0;
	m_dwVideoAspectRatioMode	= MFVideoARMode_PreservePicture;
	m_dwVideoRenderPrefs		= (MFVideoRenderPrefs)0;
	m_BorderColor				= RGB (0,0,0);
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

		m_hThread		= ::CreateThread(NULL, 0, PresentThread, (LPVOID)this, 0, &dwThreadId);

		m_nRenderState		= Stopped;
		TRACE ("Worker threads started...\n");
	}
}

void CEVRAllocatorPresenter::StopWorkerThreads()
{
	if (m_nRenderState != Shutdown)
	{
		SetEvent (m_hEvtFlush);
		SetEvent (m_hEvtQuit);
		if ((m_hThread != INVALID_HANDLE_VALUE) && (WaitForSingleObject (m_hThread, 10000) == WAIT_TIMEOUT))
		{
			ASSERT (FALSE);
			TerminateThread (m_hThread, 0xDEAD);
		}

		if (m_hThread		 != INVALID_HANDLE_VALUE) CloseHandle (m_hThread);
		if (m_hSemPicture	 != INVALID_HANDLE_VALUE) CloseHandle (m_hSemPicture);
		if (m_hEvtPresent	 != INVALID_HANDLE_VALUE) CloseHandle (m_hEvtPresent);
		if (m_hEvtFrameTimer != INVALID_HANDLE_VALUE) CloseHandle (m_hEvtFrameTimer);
		if (m_hEvtFlush		 != INVALID_HANDLE_VALUE) CloseHandle (m_hEvtFlush);
		if (m_hEvtQuit		 != INVALID_HANDLE_VALUE) CloseHandle (m_hEvtQuit);

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
	else if(riid == __uuidof(IMFVideoDisplayControl))
		hr = GetInterface((IMFVideoDisplayControl*)this, ppv);
	else if(riid == __uuidof(IEVRTrustedVideoPlugin))
		hr = GetInterface((IEVRTrustedVideoPlugin*)this, ppv);
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

	return S_OK;
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
	*piAvgFrameRate = (int)(m_fAvrFps * 100);
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::get_Jitter(int *iJitter)
{
	*iJitter = (int)(m_pllJitter[m_nNextJitter]/5000);
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

	m_AspectRatio.cx	= VideoFormat->videoInfo.PixelAspectRatio.Numerator;
	m_AspectRatio.cy	= VideoFormat->videoInfo.PixelAspectRatio.Denominator;

	if (SUCCEEDED (hr))
	{
		i64Size.HighPart = VideoFormat->videoInfo.dwWidth;
		i64Size.LowPart	 = VideoFormat->videoInfo.dwHeight;
		m_pMediaType->SetUINT64 (MF_MT_FRAME_SIZE, i64Size.QuadPart);

		m_pMediaType->SetUINT32 (MF_MT_PAN_SCAN_ENABLED, 0);

		i64Size.HighPart = m_AspectRatio.cx;
		i64Size.LowPart  = m_AspectRatio.cy;
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


HRESULT CEVRAllocatorPresenter::GetImageFromMixer()
{
	MFT_OUTPUT_DATA_BUFFER		Buffer;
	HRESULT						hr = S_OK;
	DWORD						dwStatus;
	REFERENCE_TIME				nsSampleTime;
	MFTIME						nsCurrentTime;
	LONGLONG					llClockBefore = 0;
	LONGLONG					llClockAfter  = 0;
	LONGLONG					llMixerLatency;
	UINT						dwSurface;

	while (SUCCEEDED(hr))
	{
		CComPtr<IMFSample>		pSample;

		if (FAILED (GetFreeSample (&pSample)))
		{
			m_bWaitingSample = true;
			break;
		}

		memset (&Buffer, 0, sizeof(Buffer));
		Buffer.pSample	= pSample;
		pSample->GetUINT32 (GUID_SURFACE_INDEX, &dwSurface);

		if (m_pClock) m_pClock->GetCorrelatedTime(0, &llClockBefore, &nsCurrentTime);			
		hr = m_pMixer->ProcessOutput (0 , 1, &Buffer, &dwStatus);
		if (m_pClock) m_pClock->GetCorrelatedTime(0, &llClockAfter, &nsCurrentTime);

		if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) 
		{
			MoveToFreeList (pSample);
			break;
		}

		if (m_pSink) 
		{
			CAutoLock autolock(this);
			llMixerLatency = llClockAfter - llClockBefore;
			m_pSink->Notify (EC_PROCESSING_LATENCY, (LONG_PTR)&llMixerLatency, 0);
		}

		pSample->GetSampleTime (&nsSampleTime);

		// Update internal subtitle clock
		if(m_fUseInternalTimer && m_pSubPicQueue)
		{
			MFTIME		nsDuration;
			float		m_fps;
			pSample->GetSampleDuration(&nsDuration);
			m_fps = (float)(10000000.0 / nsDuration);
			m_pSubPicQueue->SetFPS(m_fps);
		}

		if (AfxGetMyApp()->m_fTearingTest)
		{
			RECT		rcTearing;
			
			rcTearing.left		= m_nTearingPos;
			rcTearing.top		= 0;
			rcTearing.right		= rcTearing.left + 4;
			rcTearing.bottom	= m_NativeVideoSize.cy;
			m_pD3DDev->ColorFill (m_pVideoSurface[dwSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));

			rcTearing.left	= (rcTearing.right + 15) % m_NativeVideoSize.cx;
			rcTearing.right	= rcTearing.left + 4;
			m_pD3DDev->ColorFill (m_pVideoSurface[dwSurface], &rcTearing, D3DCOLOR_ARGB (255,255,0,0));
			m_nTearingPos = (m_nTearingPos + 7) % m_NativeVideoSize.cx;
		}	

		TRACE ("Get from Mixer : %d  (%I64d)\n", dwSurface, nsSampleTime);

		MoveToScheduledList (pSample);
		ReleaseSemaphore (m_hSemPicture, 1, NULL);
		InterlockedIncrement (&m_nUsedBuffer);
	}

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
	else if (guidService == MR_VIDEO_ACCELERATION_SERVICE)
		return m_pD3DManager->QueryInterface (__uuidof(IDirect3DDeviceManager9), (void**) ppvObject);

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


// IMFVideoDisplayControl
STDMETHODIMP CEVRAllocatorPresenter::GetNativeVideoSize(SIZE *pszVideo, SIZE *pszARVideo)
{
	if (pszVideo)
	{
		pszVideo->cx	= m_NativeVideoSize.cx;
		pszVideo->cy	= m_NativeVideoSize.cy;
	}
	if (pszARVideo)
	{
		pszARVideo->cx	= m_NativeVideoSize.cx * m_AspectRatio.cx;
		pszARVideo->cy	= m_NativeVideoSize.cy * m_AspectRatio.cy;
	}
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetIdealVideoSize(SIZE *pszMin, SIZE *pszMax)
{
	if (pszMin)
	{
		pszMin->cx	= 1;
		pszMin->cy	= 1;
	}

	if (pszMax)
	{
		D3DDISPLAYMODE	d3ddm;

		ZeroMemory(&d3ddm, sizeof(d3ddm));
		if(SUCCEEDED(m_pD3D->GetAdapterDisplayMode(GetAdapter(m_pD3D), &d3ddm)))
		{
			pszMax->cx	= d3ddm.Width;
			pszMax->cy	= d3ddm.Height;
		}
	}

	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetVideoPosition(const MFVideoNormalizedRect *pnrcSource, const LPRECT prcDest)
{
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetVideoPosition(MFVideoNormalizedRect *pnrcSource, LPRECT prcDest)
{
	// Always all source rectangle ?
	if (pnrcSource)
	{
		pnrcSource->left	= 0.0;
		pnrcSource->top		= 0.0;
		pnrcSource->right	= 1.0;
		pnrcSource->bottom	= 1.0;
	}

	if (prcDest)
		memcpy (prcDest, &m_VideoRect, sizeof(m_VideoRect));//GetClientRect (m_hWnd, prcDest);

	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetAspectRatioMode(DWORD dwAspectRatioMode)
{
	m_dwVideoAspectRatioMode = (MFVideoAspectRatioMode)dwAspectRatioMode;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetAspectRatioMode(DWORD *pdwAspectRatioMode)
{
	CheckPointer (pdwAspectRatioMode, E_POINTER);
	*pdwAspectRatioMode = m_dwVideoAspectRatioMode;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetVideoWindow(HWND hwndVideo)
{
	ASSERT (m_hWnd == hwndVideo);	// What if not ??
//	m_hWnd = hwndVideo;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetVideoWindow(HWND *phwndVideo)
{
	CheckPointer (phwndVideo, E_POINTER);
	*phwndVideo = m_hWnd;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::RepaintVideo()
{
	Paint (true);
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetCurrentImage(BITMAPINFOHEADER *pBih, BYTE **pDib, DWORD *pcbDib, LONGLONG *pTimeStamp)
{
	ASSERT (FALSE);
	return E_NOTIMPL;
}
STDMETHODIMP CEVRAllocatorPresenter::SetBorderColor(COLORREF Clr)
{
	m_BorderColor = Clr;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetBorderColor(COLORREF *pClr)
{
	CheckPointer (pClr, E_POINTER);
	*pClr = m_BorderColor;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetRenderingPrefs(DWORD dwRenderFlags)
{
	m_dwVideoRenderPrefs = (MFVideoRenderPrefs)dwRenderFlags;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::GetRenderingPrefs(DWORD *pdwRenderFlags)
{
	CheckPointer(pdwRenderFlags, E_POINTER);
	*pdwRenderFlags = m_dwVideoRenderPrefs;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetFullscreen(BOOL fFullscreen)
{
	ASSERT (FALSE);
	return E_NOTIMPL;
}
STDMETHODIMP CEVRAllocatorPresenter::GetFullscreen(BOOL *pfFullscreen)
{
	ASSERT (FALSE);
	return E_NOTIMPL;
}


// IEVRTrustedVideoPlugin
STDMETHODIMP CEVRAllocatorPresenter::IsInTrustedVideoMode(BOOL *pYes)
{
	CheckPointer(pYes, E_POINTER);
	*pYes = TRUE;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::CanConstrict(BOOL *pYes)
{
	CheckPointer(pYes, E_POINTER);
	*pYes = TRUE;
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::SetConstriction(DWORD dwKPix)
{
	return S_OK;
}
STDMETHODIMP CEVRAllocatorPresenter::DisableImageExport(BOOL bDisable)
{
	return S_OK;
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
	}

	return hr;
}


STDMETHODIMP CEVRAllocatorPresenter::GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight)
{
	// This function should be called...
	ASSERT (FALSE);

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

	m_NativeVideoSize = CSize(w, h);
	hr = AllocSurfaces();

	CAutoLock lock(this);
	RemoveAllSamples();
	for(int i = 0; i < m_nNbDXSurface; i++)
	{
		CComPtr<IMFSample>		pMFSample;
		hr = pfMFCreateVideoSampleFromSurface (m_pVideoSurface[i], &pMFSample);

		if (SUCCEEDED (hr))
		{
			pMFSample->SetUINT32 (GUID_SURFACE_INDEX, i);
			m_FreeSamples.AddTail (pMFSample);
		}
		ASSERT (SUCCEEDED (hr));
	}


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
	if (m_bWaitingSample)
	{
		m_bWaitingSample = false;
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
			// Flush pending samples!
			if (dwUser != -1) timeKillEvent (dwUser);
			dwUser = -1;		

			FlushSamples();
			ResetEvent(m_hEvtFlush);
			TRACE ("Flush done!\n");
			break;

		case WAIT_OBJECT_0 + 2 :
		case WAIT_OBJECT_0 + 3 :
			// Discard timer events if playback stop
			if ((dwObject == WAIT_OBJECT_0 + 3) && (m_nRenderState != Started)) continue;

//			TRACE ("RenderThread ==>> Waiting buffer\n");
			
			if (WaitForMultipleObjects (countof(hEvtsBuff), hEvtsBuff, FALSE, INFINITE) == WAIT_OBJECT_0+2)
			{
//				m_nCurSurface = (m_nCurSurface + 1) % m_nNbDXSurface;
				CComPtr<IMFSample>		pMFSample = m_ScheduledSamples.RemoveHead();
			
				pMFSample->GetUINT32 (GUID_SURFACE_INDEX, (UINT32*)&m_nCurSurface);
				pMFSample->GetSampleTime (&nsSampleTime);

				TRACE ("RenderThread ==>> Presenting surface %d  (%I64d)\n", m_nCurSurface, nsSampleTime);

				__super::SetTime (g_tSegmentStart + nsSampleTime);
				Paint(true);

				m_pcFramesDrawn++;
				InterlockedDecrement (&m_nUsedBuffer);
				CompleteFrameStep (false);

				if ((m_nRenderState == Started) && (m_nStepCount == 0))
				{
					// Calculate wake up timer
					m_pClock->GetCorrelatedTime(0, &llClockTime, &nsCurrentTime);			

					//if (m_rtTimePerFrame == 0) CalculateFrameRate(/*nsSampleTime*/);
					// Wakup 1/2 refresh rate before next VSync!
					lDelay = (UINT)((nsSampleTime + g_rtTimePerFrame - llClockTime) / 10000) - (500/m_RefreshRate);

					if (lDelay > 0)
					{
//						TRACE ("RenderThread ==>> Set timer %d   %I64d  Cons=%d \n", lDelay, nsSampleTime, m_nCurSurface);
						dwUser			= timeSetEvent (lDelay, dwResolution, (LPTIMECALLBACK)m_hEvtFrameTimer, NULL, TIME_CALLBACK_EVENT_SET); 

						// Update statistics
						nNbPlayingFrame++;
					}
					else
					{
						dwUser = -1;
						if (m_nRenderState == Started) m_pcFrames++;
//						TRACE ("RenderThread ==>> immediate display   %I64d  (delay=%d)  Cons=%d\n", nsSampleTime/417188, lDelay, m_nCurSurface);
						SetEvent (m_hEvtPresent);
					}
				}

				MoveToFreeList(pMFSample);
				CheckWaitingSampleFromMixer();
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
	HRESULT		hr;

	// Reset DXVA Manager, and get new buffers
	hr = m_pD3DManager->ResetDevice(m_pD3DDev, m_nResetToken);

	// Not necessary, but Microsoft documentation say Presenter should send this message...
	if (m_pSink)
		m_pSink->Notify (EC_DISPLAY_CHANGED, 0, 0);
}



void CEVRAllocatorPresenter::RemoveAllSamples()
{
	m_ScheduledSamples.RemoveAll();
	m_FreeSamples.RemoveAll();
}

HRESULT CEVRAllocatorPresenter::GetFreeSample(IMFSample** ppSample)
{
	CAutoLock lock(this);
	HRESULT		hr = S_OK;

	if (m_FreeSamples.GetCount() > 1)	// <= Cannot use first free buffer (can be currently displayed)
		*ppSample = m_FreeSamples.RemoveHead().Detach();
	else
		hr = MF_E_SAMPLEALLOCATOR_EMPTY;

	return hr;
}


HRESULT CEVRAllocatorPresenter::GetScheduledSample(IMFSample** ppSample)
{
	CAutoLock lock(this);
	HRESULT		hr = S_OK;

	if (m_ScheduledSamples.GetCount() > 0)
		*ppSample = m_ScheduledSamples.RemoveHead().Detach();
	else
		hr = MF_E_SAMPLEALLOCATOR_EMPTY;

	return hr;
}


void CEVRAllocatorPresenter::MoveToFreeList(IMFSample* pSample)
{
	CAutoLock lock(this);
	m_FreeSamples.AddTail (pSample);
}


void CEVRAllocatorPresenter::MoveToScheduledList(IMFSample* pSample)
{
	CAutoLock lock(this);
	m_ScheduledSamples.AddTail (pSample);
}


void CEVRAllocatorPresenter::FlushSamples()
{
	CAutoLock				lock(this);

	m_nUsedBuffer = 0;
	while (m_ScheduledSamples.GetCount() > 0)
	{
		CComPtr<IMFSample>		pMFSample;

		pMFSample = m_ScheduledSamples.RemoveHead();
		MoveToFreeList (pMFSample);

		WaitForSingleObject (m_hSemPicture, 0);
	}
}