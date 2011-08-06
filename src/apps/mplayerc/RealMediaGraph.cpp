/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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
#include "mplayerc.h"
#include "MainFrm.h"
#include <math.h>
#include <atlbase.h>
#include <atlcoll.h>
#include <InitGuid.h>
#include "RealMediaGraph.h"
#include "RealMediaWindowlessSite.h"
#include <realmedia/rmavsurf.h>
#include <realmedia/rmaevent.h>
#include <realmedia/rmaprefs.h>
#include "../../DSUtil/DSUtil.h"
#include "AuthDlg.h"

using namespace DSObjects;

// CRealMediaPlayer

CRealMediaPlayer::CRealMediaPlayer(HWND hWndParent, CRealMediaGraph* pRMG)
	: CUnknown(NAME("CRealMediaPlayer"), NULL)
	, m_pRMG(pRMG)
	, m_hWndParent(hWndParent)
	, m_fpCreateEngine(NULL), m_fpCloseEngine(NULL), m_hRealMediaCore(NULL)
	, m_State(State_Stopped), m_UserState(State_Stopped), m_nCurrent(0), m_nDuration(0)
	, m_VideoSize(0, 0)
	, m_fVideoSizeChanged(true)
{
}

CRealMediaPlayer::~CRealMediaPlayer()
{
	Deinit();
}

bool CRealMediaPlayer::Init()
{
	CString prefs(_T("Software\\RealNetworks\\Preferences"));

	CRegKey key;

	if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, prefs + _T("\\DT_Common"), KEY_READ)) {
		return(false);
	}

	TCHAR buff[_MAX_PATH];
	ULONG len = sizeof(buff)/sizeof(buff[0]);
	if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len)) {
		return(false);
	}

	key.Close();

	m_hRealMediaCore = LoadLibrary(CString(buff) + _T("pnen3260.dll"));
	if (!m_hRealMediaCore) {
		return(false);
	}

	m_fpCreateEngine = (FPRMCREATEENGINE)GetProcAddress(m_hRealMediaCore, "CreateEngine");
	m_fpCloseEngine = (FPRMCLOSEENGINE)GetProcAddress(m_hRealMediaCore, "CloseEngine");
	m_fpSetDLLAccessPath = (FPRMSETDLLACCESSPATH)GetProcAddress(m_hRealMediaCore, "SetDLLAccessPath");

	if (!m_fpCreateEngine || !m_fpCloseEngine || !m_fpSetDLLAccessPath) {
		return(false);
	}

	if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, prefs, KEY_READ)) {
		CString dllpaths;

		len = sizeof(buff)/sizeof(buff[0]);
		for (int i = 0; ERROR_SUCCESS == key.EnumKey(i, buff, &len); i++,len = sizeof(buff)/sizeof(buff[0])) {
			CRegKey key2;
			TCHAR buff2[_MAX_PATH];
			ULONG len2 = sizeof(buff2)/sizeof(buff2[0]);
			if (ERROR_SUCCESS != key2.Open(HKEY_CLASSES_ROOT, prefs + _T("\\") + buff, KEY_READ)
					|| ERROR_SUCCESS != key2.QueryStringValue(NULL, buff2, &len2)) {
				continue;
			}

			dllpaths += CString(buff) + '=' + buff2 + '|';
		}

		key.Close();

		if (!dllpaths.IsEmpty()) {
			char* s = DNew char[dllpaths.GetLength()+1];
			strcpy(s, CStringA(dllpaths));
			for (int i = 0, j = strlen(s); i < j; i++) {
				if (s[i] == '|') {
					s[i] = '\0';
				}
			}
			m_fpSetDLLAccessPath(s);
			delete [] s;
		}
	}

	if (PNR_OK != m_fpCreateEngine(&m_pEngine)) {
		return(false);
	}

	if (PNR_OK != m_pEngine->CreatePlayer(*&m_pPlayer)) {
		return(false);
	}

	if (!(m_pSiteManager = m_pPlayer) || !(m_pCommonClassFactory = m_pPlayer)) {
		return(false);
	}

	m_pAudioPlayer = m_pPlayer;
	m_pAudioPlayer->AddPostMixHook(static_cast<IRMAAudioHook*>(this), FALSE, FALSE);
	//	m_pVolume = m_pAudioPlayer->GetDeviceVolume();
	m_pVolume = m_pAudioPlayer->GetAudioVolume();

	// IRMAVolume::SetVolume has a huge latency when used via GetAudioVolume,
	// but by lowering this audio pushdown thing it can get better
	CComQIPtr<IRMAAudioPushdown, &IID_IRMAAudioPushdown> pAP = m_pAudioPlayer;
	if (pAP) {
		pAP->SetAudioPushdown(300);    // 100ms makes the playback sound choppy, 200ms looks ok, but for safety we set this to 300ms... :P
	}

	CComQIPtr<IRMAErrorSinkControl, &IID_IRMAErrorSinkControl> pErrorSinkControl = m_pPlayer;
	if (pErrorSinkControl) {
		pErrorSinkControl->AddErrorSink(static_cast<IRMAErrorSink*>(this), PNLOG_EMERG, PNLOG_INFO);
	}

	if (PNR_OK != m_pPlayer->AddAdviseSink(static_cast<IRMAClientAdviseSink*>(this))) {
		return(false);
	}

	if (PNR_OK != m_pPlayer->SetClientContext((IUnknown*)(INonDelegatingUnknown*)(this))) {
		return(false);
	}

	// TODO
	/*
		if (CComQIPtr<IRMAPreferences, &IID_IRMAPreferences> pPrefs = m_pPlayer)
		{
			CComPtr<IRMABuffer> pBuffer;
			HRESULT hr = pPrefs->ReadPref("HTTPProxyHost", *&pBuffer);

			UCHAR* pData = NULL;
			ULONG32 ulLength = 0;
			hr = pBuffer->Get(pData, ulLength);

			pBuffer = NULL;
			hr = m_pCommonClassFactory->CreateInstance(CLSID_IRMABuffer, (void**)&pBuffer);
			hr = pBuffer->SetSize(strlen("localhost")+1);
			pData = pBuffer->GetBuffer();
			strcpy((char*)pData, "localhost");
			hr = pBuffer->Set(pData, strlen("localhost")+1);

			pData = NULL;
			ulLength = 0;
			hr = pBuffer->Get(pData, ulLength);

			hr = pPrefs->WritePref("HTTPProxyHost", pBuffer);

			hr = hr;
		}
	*/
	return(true);
}

void CRealMediaPlayer::Deinit()
{
	if (m_pPlayer) {
		m_pPlayer->Stop();

		CComQIPtr<IRMAErrorSinkControl, &IID_IRMAErrorSinkControl> pErrorSinkControl = m_pPlayer;
		if (pErrorSinkControl) {
			pErrorSinkControl->RemoveErrorSink(static_cast<IRMAErrorSink*>(this));
		}

		m_pPlayer->RemoveAdviseSink(static_cast<IRMAClientAdviseSink*>(this));

		m_pVolume = NULL;
		m_pAudioPlayer->RemovePostMixHook(static_cast<IRMAAudioHook*>(this));
		m_pAudioPlayer.Release();

		m_pEngine->ClosePlayer(m_pPlayer);

		m_pSiteManager.Release();
		m_pCommonClassFactory.Release();

		m_pPlayer = NULL;
	}

	if (m_pEngine) {
		m_fpCloseEngine(m_pEngine);
		m_pEngine.Detach();
	}

	if (m_hRealMediaCore) {
		FreeLibrary(m_hRealMediaCore);
		m_hRealMediaCore = NULL;
	}
}

STDMETHODIMP CRealMediaPlayer::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		QI2(IRMAErrorSink)
		QI2(IRMAClientAdviseSink)
		QI2(IRMAAuthenticationManager)
		QI2(IRMASiteSupplier)
		QI2(IRMAPassiveSiteWatcher)
		QI2(IRMAAudioHook)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

char* AllocateErrorMessage(const char* msg)
{
	char* errmsg = NULL;
	int len = strlen(msg);
	if (len > 0) {
		errmsg = (char*)CoTaskMemAlloc(len+1);
		if (errmsg) {
			strcpy(errmsg, msg);
		}
	}
	return errmsg;
}

// IRMAErrorSink
STDMETHODIMP CRealMediaPlayer::ErrorOccurred(const UINT8 unSeverity, const UINT32 ulRMACode, const UINT32 ulUserCode, const char* pUserString, const char* pMoreInfoURL)
{
	char* errmsg = NULL;

	if (unSeverity < 5) {
		if (CComQIPtr<IRMAErrorMessages, &IID_IRMAErrorMessages> pErrorMessages = m_pPlayer) {
			CComPtr<IRMABuffer> pBuffer = pErrorMessages->GetErrorText(ulRMACode);
			if (pBuffer) {
				char* buff = (char*)pBuffer->GetBuffer();
				errmsg = AllocateErrorMessage(buff);
			}
		}

		if (!errmsg) {
			errmsg = AllocateErrorMessage("RealMedia error");
			TRACE("RealMedia error\n");
		}

		m_pRMG->NotifyEvent(EC_BG_ERROR, (LONG_PTR)errmsg, 0);
	}

	return PNR_OK;
}

// IRMAClientAdviseSink
STDMETHODIMP CRealMediaPlayer::OnPosLength(UINT32 ulPosition, UINT32 ulLength)
{
	m_nCurrent = (REFERENCE_TIME)ulPosition*10000;
	m_nDuration = (REFERENCE_TIME)ulLength*10000;
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnPresentationOpened()
{
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnPresentationClosed()
{
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnStatisticsChanged()
{
	m_pRMG->NotifyEvent(EC_LENGTH_CHANGED);
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnPreSeek(UINT32 ulOldTime, UINT32 ulNewTime)
{
	m_nCurrent = (REFERENCE_TIME)ulNewTime*10000;
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnPostSeek(UINT32 ulOldTime, UINT32 ulNewTime)
{
	m_nCurrent = (REFERENCE_TIME)ulNewTime*10000;
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnStop()
{
	m_nCurrent = 0;
	m_State = State_Stopped;
	if (m_UserState != State_Stopped) {
		m_pRMG->NotifyEvent(EC_COMPLETE);
	}
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnPause(UINT32 ulTime)
{
	m_State = State_Paused;
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnBegin(UINT32 ulTime)
{
	m_State = State_Running;
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnBuffering(UINT32 ulFlags, UINT16 unPercentComplete)
{
	m_unPercentComplete = unPercentComplete;
	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::OnContacting(const char* pHostName)
{
	return PNR_OK;
}

// IRMAAuthenticationManager
STDMETHODIMP CRealMediaPlayer::HandleAuthenticationRequest(IRMAAuthenticationManagerResponse* pResponse)
{
	CAuthDlg dlg;

	if (dlg.DoModal() == IDOK) {
		pResponse->AuthenticationRequestDone(
			PNR_OK, CStringA(dlg.m_username), CStringA(dlg.m_password));
		return PNR_OK;
	}

	return pResponse->AuthenticationRequestDone(PNR_NOT_AUTHORIZED, NULL, NULL);
}

// IRMASiteSupplier
STDMETHODIMP CRealMediaPlayer::SitesNeeded(UINT32 uRequestID, IRMAValues* pProps)
{
	if (!pProps) {
		return PNR_INVALID_PARAMETER;
	}

	if (m_pTheSite || m_pTheSite2 || !m_hWndParent) {
		return PNR_UNEXPECTED;
	}

	HRESULT hr = PNR_OK;

	if (!CreateSite(&m_pTheSite)) {
		return E_FAIL;
	}

	ULONG refc = ((IRMASite*)m_pTheSite)->AddRef();
	refc = ((IRMASite*)m_pTheSite)->Release();

	if (!(m_pTheSite2 = m_pTheSite)) {
		return E_NOINTERFACE;
	}

	CComQIPtr<IRMAValues, &IID_IRMAValues> pSiteProps = m_pTheSite;
	if (!pSiteProps) {
		return E_NOINTERFACE;
	}

	IRMABuffer* pValue;

	// no idea what these supposed to do... but they were in the example
	hr = pProps->GetPropertyCString("playto", pValue);
	if (PNR_OK == hr) {
		pSiteProps->SetPropertyCString("channel", pValue);
		pValue->Release();
	} else {
		hr = pProps->GetPropertyCString("name", pValue);
		if (PNR_OK == hr) {
			pSiteProps->SetPropertyCString("LayoutGroup", pValue);
			pValue->Release();
		}
	}

	m_pTheSite2->AddPassiveSiteWatcher(static_cast<IRMAPassiveSiteWatcher*>(this));

	hr = m_pSiteManager->AddSite(m_pTheSite);
	if (PNR_OK != hr) {
		return hr;
	}

	m_CreatedSites[uRequestID] = m_pTheSite;

	return hr;
}

STDMETHODIMP CRealMediaPlayer::SitesNotNeeded(UINT32 uRequestID)
{
	IRMASite* pSite;
	if (!m_CreatedSites.Lookup(uRequestID, pSite)) {
		return PNR_INVALID_PARAMETER;
	}

	m_CreatedSites.RemoveKey(uRequestID);

	m_pSiteManager->RemoveSite(pSite);

	m_pTheSite2->RemovePassiveSiteWatcher(static_cast<IRMAPassiveSiteWatcher*>(this));

	m_pTheSite.Release();
	m_pTheSite2.Release();

	DestroySite(pSite);

	return PNR_OK;
}

STDMETHODIMP CRealMediaPlayer::BeginChangeLayout()
{
	return E_NOTIMPL;
}

STDMETHODIMP CRealMediaPlayer::DoneChangeLayout()
{
	if (m_fVideoSizeChanged) {
		m_pRMG->NotifyEvent(EC_VIDEO_SIZE_CHANGED, MAKELPARAM(m_VideoSize.cx, m_VideoSize.cy), 0);
		m_fVideoSizeChanged = false;
	}

	return PNR_OK;
}

// IRMAPassiveSiteWatcher
STDMETHODIMP CRealMediaPlayer::PositionChanged(PNxPoint* pos)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRealMediaPlayer::SizeChanged(PNxSize* size)
{
	if (m_VideoSize.cx == 0 || m_VideoSize.cy == 0) {
		m_fVideoSizeChanged = true;
		m_VideoSize.cx = size->cx;
		m_VideoSize.cy = size->cy;
	}
	return PNR_OK;
}

// IRMAAudioHook
STDMETHODIMP CRealMediaPlayer::OnBuffer(RMAAudioData* pAudioInData, RMAAudioData* pAudioOutData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CRealMediaPlayer::OnInit(RMAAudioFormat* pFormat)
{
	m_pRMG->NotifyEvent(EC_BG_AUDIO_CHANGED, pFormat->uChannels, 0);
	return PNR_OK;
}

//
// CRealMediaPlayerWindowed
//

CRealMediaPlayerWindowed::CRealMediaPlayerWindowed(HWND hWndParent, CRealMediaGraph* pRMG)
	: CRealMediaPlayer(hWndParent, pRMG)
{
	if (!m_wndWindowFrame.CreateEx(WS_EX_NOPARENTNOTIFY, NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE,
								   CRect(0, 0, 0, 0), CWnd::FromHandle(m_hWndParent), 0, NULL)) {
		return;
	}

	if (!m_wndDestFrame.Create(NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
							   CRect(0, 0, 0, 0), &m_wndWindowFrame, 0, NULL)) {
		return;
	}
}

CRealMediaPlayerWindowed::~CRealMediaPlayerWindowed()
{
	m_wndDestFrame.DestroyWindow();
	m_wndWindowFrame.DestroyWindow();
}

void CRealMediaPlayerWindowed::SetWindowRect(CRect r)
{
	if (IsWindow(m_wndWindowFrame.m_hWnd)) {
		m_wndWindowFrame.MoveWindow(r);
	}
}

void CRealMediaPlayerWindowed::SetDestRect(CRect r)
{
	if (IsWindow(m_wndDestFrame.m_hWnd)) {
		m_wndDestFrame.MoveWindow(r);
	}

	if (m_pTheSite) {
		PNxSize s = {r.Width(), r.Height()};
		m_pTheSite->SetSize(s);
	}
}

bool CRealMediaPlayerWindowed::CreateSite(IRMASite** ppSite)
{
	if (!ppSite) {
		return(false);
	}

	CComPtr<IRMASiteWindowed> pSiteWindowed;
	if (PNR_OK != m_pCommonClassFactory->CreateInstance(CLSID_IRMASiteWindowed, (void**)&pSiteWindowed)) {
		return(false);
	}

	DWORD style = WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
	if (!AfxGetAppSettings().fIntRealMedia) {
		style |= WS_DISABLED;
	}
	if (PNR_OK != pSiteWindowed->Create(m_wndDestFrame.m_hWnd, style)) {
		return(false);
	}

	*ppSite = CComQIPtr<IRMASite, &IID_IRMASite>(pSiteWindowed).Detach();
	return !!(*ppSite);
}

void CRealMediaPlayerWindowed::DestroySite(IRMASite* pSite)
{
	if (CComQIPtr<IRMASiteWindowed, &IID_IRMASiteWindowed> pRMASiteWindowed = pSite) {
		pRMASiteWindowed->Destroy();
	}
}
//
// CRealMediaPlayerWindowless
//

CRealMediaPlayerWindowless::CRealMediaPlayerWindowless(HWND hWndParent, CRealMediaGraph* pRMG)
	: CRealMediaPlayer(hWndParent, pRMG)
{
	AppSettings& s = AfxGetAppSettings();

	bool bFullscreen = (AfxGetApp()->m_pMainWnd != NULL) && (((CMainFrame*)AfxGetApp()->m_pMainWnd)->IsD3DFullScreenMode());
	switch (s.iRMVideoRendererType) {
		default:
		case VIDRNDT_RM_DX7:
			if (FAILED(CreateAP7(CLSID_RM7AllocatorPresenter, hWndParent, &m_pRMAP))) {
				return;
			}
			break;
		case VIDRNDT_RM_DX9:
			if (FAILED(CreateAP9(CLSID_RM9AllocatorPresenter, hWndParent, bFullscreen, &m_pRMAP))) {
				return;
			}
			break;
	}
}

CRealMediaPlayerWindowless::~CRealMediaPlayerWindowless()
{
}

STDMETHODIMP CRealMediaPlayerWindowless::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		(m_pRMAP && (riid == __uuidof(ISubPicAllocatorPresenter) || riid == IID_IRMAVideoSurface)) ? m_pRMAP->QueryInterface(riid, ppv) :
		__super::NonDelegatingQueryInterface(riid, ppv);
}

bool CRealMediaPlayerWindowless::CreateSite(IRMASite** ppSite)
{
	if (!ppSite || !m_pRMAP) {
		return(false);
	}

	HRESULT hr = S_OK;

	CRealMediaWindowlessSite* pWMWlS;

	CComPtr<IRMASiteWindowless> pSiteWindowless;
	pSiteWindowless = (IRMASiteWindowless*)(pWMWlS = DNew CRealMediaWindowlessSite(hr, m_pPlayer, NULL, NULL));
	if (FAILED(hr)) {
		return(false);
	}

	pWMWlS->SetBltService(CComQIPtr<IRMAVideoSurface, &IID_IRMAVideoSurface>(m_pRMAP));

	*ppSite = CComQIPtr<IRMASite, &IID_IRMASite>(pSiteWindowless).Detach();
	return !!(*ppSite);
}

void CRealMediaPlayerWindowless::DestroySite(IRMASite* pSite)
{
}

STDMETHODIMP CRealMediaPlayerWindowless::SizeChanged(PNxSize* size)
{
	if (CComQIPtr<IRMAVideoSurface, &IID_IRMAVideoSurface> pRMAVS = m_pRMAP) {
		RMABitmapInfoHeader BitmapInfo;
		memset(&BitmapInfo, 0, sizeof(BitmapInfo));
		BitmapInfo.biWidth = size->cx;
		BitmapInfo.biHeight = size->cy;
		pRMAVS->BeginOptimizedBlt(&BitmapInfo);
	}

	return __super::SizeChanged(size);
}


////////////////

CRealMediaGraph::CRealMediaGraph(HWND hWndParent, HRESULT& hr)
	: CBaseGraph()
{
	hr = S_OK;

	m_pRMP = AfxGetAppSettings().iRMVideoRendererType == VIDRNDT_RM_DEFAULT
			 ? (CRealMediaPlayer*)DNew CRealMediaPlayerWindowed(hWndParent, this)
			 : (CRealMediaPlayer*)DNew CRealMediaPlayerWindowless(hWndParent, this);

	if (!m_pRMP) {
		hr = E_OUTOFMEMORY;
		return;
	}

	if (!m_pRMP->Init()) {
		delete m_pRMP, m_pRMP = NULL;
		hr = E_FAIL;
		return;
	}

	m_pRMP->AddRef();
}

CRealMediaGraph::~CRealMediaGraph()
{
	if (m_pRMP) {
		m_pRMP->Deinit();
		m_pRMP->Release();
		m_pRMP = NULL;
	}
}

STDMETHODIMP CRealMediaGraph::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		(m_pRMP && (riid == __uuidof(ISubPicAllocatorPresenter) || riid == __uuidof(ISubPicAllocatorPresenter))) ? m_pRMP->QueryInterface(riid, ppv) :
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// IGraphBuilder
STDMETHODIMP CRealMediaGraph::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
	m_fn = lpcwstrFile;

	CHAR buff[_MAX_PATH] = {0};
	WideCharToMultiByte(GetACP(), 0, lpcwstrFile, -1, buff, _MAX_PATH, 0, 0);

	CStringA fn(buff);
	if (fn.Find("://") < 0) {
		fn = "file://" + fn;
	}

	m_pRMP->m_unPercentComplete = 100;

	ClearMessageQueue();

	if (PNR_OK != m_pRMP->m_pPlayer->OpenURL(fn)) {
		return E_FAIL;
	}

	m_pRMP->m_pPlayer->Pause()/*Stop()*/; // please, don't start just yet

	return S_OK;
}

// IMediaControl
STDMETHODIMP CRealMediaGraph::Run()
{
	if (m_pRMP->m_pPlayer->IsDone()) {
		RenderFile(m_fn, NULL);
	}

	m_pRMP->m_UserState = State_Running;
	return (PNR_OK == m_pRMP->m_pPlayer->Begin()) ? S_OK : E_FAIL;
}

STDMETHODIMP CRealMediaGraph::Pause()
{
	m_pRMP->m_UserState = State_Paused;
	return (PNR_OK == m_pRMP->m_pPlayer->Pause()) ? S_OK : E_FAIL;
}

STDMETHODIMP CRealMediaGraph::Stop()
{
	m_pRMP->m_UserState = State_Stopped;
	return (PNR_OK == m_pRMP->m_pPlayer->Stop()) ? S_OK : E_FAIL;
}

STDMETHODIMP CRealMediaGraph::GetState(LONG msTimeout, OAFilterState* pfs)
{
	return pfs ? *pfs = m_pRMP->m_State, S_OK : E_POINTER;
}

// IMediaSeeking
STDMETHODIMP CRealMediaGraph::GetDuration(LONGLONG* pDuration)
{
	return pDuration ? *pDuration = m_pRMP->m_nDuration, S_OK : E_POINTER;
}

STDMETHODIMP CRealMediaGraph::GetCurrentPosition(LONGLONG* pCurrent)
{
	return pCurrent ? *pCurrent = m_pRMP->m_nCurrent, S_OK : E_POINTER;
}

STDMETHODIMP CRealMediaGraph::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
	return (dwCurrentFlags&AM_SEEKING_AbsolutePositioning)
		   && (PNR_OK == m_pRMP->m_pPlayer->Seek((ULONG)(*pCurrent / 10000))) ? S_OK : E_FAIL;
}

// IVideoWindow
STDMETHODIMP CRealMediaGraph::SetWindowPosition(long Left, long Top, long Width, long Height)
{
	if (m_pRMP) {
		m_pRMP->SetWindowRect(CRect(CPoint(Left, Top), CSize(Width, Height)));
	}

	return S_OK;
}

// IBasicVideo
STDMETHODIMP CRealMediaGraph::SetDestinationPosition(long Left, long Top, long Width, long Height)// {return E_NOTIMPL;}
{
	m_pRMP->SetDestRect(CRect(CPoint(Left, Top), CSize(Width, Height)));
	return S_OK;
}

STDMETHODIMP CRealMediaGraph::GetVideoSize(long* pWidth, long* pHeight)
{
	if (!pWidth || !pHeight) {
		return E_POINTER;
	}
	*pWidth = m_pRMP->GetVideoSize().cx;
	*pHeight = m_pRMP->GetVideoSize().cy;
	return S_OK;
}

// IBasicAudio
STDMETHODIMP CRealMediaGraph::put_Volume(long lVolume)
{
	if (!m_pRMP->m_pVolume) {
		return E_UNEXPECTED;
	}

	UINT16 volume = (lVolume <= -10000) ? 0 : (UINT16)(pow(10.0, lVolume/4000.0)*100);
	volume = max(min(volume, 100), 0);

	return PNR_OK == m_pRMP->m_pVolume->SetVolume(volume) ? S_OK : E_FAIL;
}

STDMETHODIMP CRealMediaGraph::get_Volume(long* plVolume)
{
	if (!m_pRMP->m_pVolume) {
		return E_UNEXPECTED;
	}

	CheckPointer(plVolume, E_POINTER);

	*plVolume = (long)m_pRMP->m_pVolume->GetVolume();
	*plVolume = (long)(4000*log10(*plVolume/100.0f));
	*plVolume = max(min(*plVolume, 0), -10000);

	return S_OK;
}

// IAMOpenProgress
STDMETHODIMP CRealMediaGraph::QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent)
{
	*pllTotal = 100;
	*pllCurrent = m_pRMP->m_unPercentComplete > 0 ? m_pRMP->m_unPercentComplete : 100; // after seeking it drops to 0 and would show annoying "Buffering... (0%)" messages on the status line
	return S_OK;
}

// IGraphEngine
STDMETHODIMP_(engine_t) CRealMediaGraph::GetEngine()
{
	return RealMedia;
}
