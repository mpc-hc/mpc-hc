/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <math.h>
#include <time.h>
#include "DirectVobSubFilter.h"
#include "TextInputPin.h"
#include "DirectVobSubPropPage.h"
#include "VSFilter.h"
#include "Systray.h"
#include "../../../DSUtil/MediaTypes.h"
#include "../../../SubPic/MemSubPic.h"
#include "../../../SubPic/SubPicQueueImpl.h"

#include <InitGuid.h>
#include <moreuuids.h>

///////////////////////////////////////////////////////////////////////////

/*removeme*/
bool g_RegOK = true;//false; // doesn't work with the dvd graph builder

////////////////////////////////////////////////////////////////////////////
//
// Constructor
//

CDirectVobSubFilter::CDirectVobSubFilter(LPUNKNOWN punk, HRESULT* phr, const GUID& clsid)
	: CBaseVideoFilter(NAME("CDirectVobSubFilter"), punk, phr, clsid)
	, m_nSubtitleId((DWORD_PTR)-1)
	, m_fMSMpeg4Fix(false)
	, m_fps(25)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_hdc = 0;
	m_hbm = 0;
	m_hfont = 0;

	{
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_CHARACTER_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = ANTIALIASED_QUALITY;
		HDC hdc = GetDC(NULL);
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(NULL, hdc);
		lf.lfWeight = FW_BOLD;
		_tcscpy(lf.lfFaceName, _T("Arial"));
		m_hfont = CreateFontIndirect(&lf);
	}

	theApp.WriteProfileString(ResStr(IDS_R_DEFTEXTPATHES), _T("Hint"), _T("The first three are fixed, but you can add more up to ten entries."));
	theApp.WriteProfileString(ResStr(IDS_R_DEFTEXTPATHES), _T("Path0"), _T("."));
	theApp.WriteProfileString(ResStr(IDS_R_DEFTEXTPATHES), _T("Path1"), _T("c:\\subtitles"));
	theApp.WriteProfileString(ResStr(IDS_R_DEFTEXTPATHES), _T("Path2"), _T(".\\subtitles"));

	m_fLoading = true;

	m_hSystrayThread = 0;
	m_tbid.hSystrayWnd = NULL;
	m_tbid.graph = NULL;
	m_tbid.fRunOnce = false;
	m_tbid.fShowIcon = (theApp.m_AppName.Find(_T("zplayer"), 0) < 0 || !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_ENABLEZPICON), 0));

	HRESULT hr = S_OK;
	m_pTextInput.Add(new CTextInputPin(this, m_pLock, &m_csSubLock, &hr));
	ASSERT(SUCCEEDED(hr));

	CAMThread::Create();
	m_frd.EndThreadEvent.Create(0, FALSE, FALSE, 0);
	m_frd.RefreshEvent.Create(0, FALSE, FALSE, 0);

	memset(&m_CurrentVIH2, 0, sizeof(VIDEOINFOHEADER2));
}

CDirectVobSubFilter::~CDirectVobSubFilter()
{
	CAutoLock cAutoLock(&m_csQueueLock);
	if (m_pSubPicQueue) {
		m_pSubPicQueue->Invalidate();
	}
	m_pSubPicQueue = NULL;

	if (m_hfont) {
		DeleteObject(m_hfont);
		m_hfont = 0;
	}
	if (m_hbm) {
		DeleteObject(m_hbm);
		m_hbm = 0;
	}
	if (m_hdc) {
		DeleteObject(m_hdc);
		m_hdc = 0;
	}

	for (size_t i = 0; i < m_pTextInput.GetCount(); i++) {
		delete m_pTextInput[i];
	}

	m_frd.EndThreadEvent.Set();
	CAMThread::Close();
}

STDMETHODIMP CDirectVobSubFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		QI(IDirectVobSub)
		QI(IDirectVobSub2)
		QI(IFilterVersion)
		QI(ISpecifyPropertyPages)
		QI(IAMStreamSelect)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// CBaseVideoFilter

void CDirectVobSubFilter::GetOutputSize(int& w, int& h, int& arx, int& ary, int& RealWidth, int& RealHeight, int& vsfilter)
{
	CSize s(w, h), os = s;
	AdjustFrameSize(s);
	w = s.cx;
	h = s.cy;
	vsfilter = 1; // enable workaround, see BaseVideoFilter.cpp

	if (w != os.cx) {
		while (arx < 100) {
			arx *= 10, ary *= 10;
		}
		arx = arx * w / os.cx;
	}

	if (h != os.cy) {
		while (ary < 100) {
			arx *= 10, ary *= 10;
		}
		ary = ary * h / os.cy;
	}
}

HRESULT CDirectVobSubFilter::Transform(IMediaSample* pIn)
{
	HRESULT hr;


	REFERENCE_TIME rtStart, rtStop;
	if (SUCCEEDED(pIn->GetTime(&rtStart, &rtStop))) {
		double dRate = m_pInput->CurrentRate();

		m_tPrev = m_pInput->CurrentStartTime() + dRate*rtStart;

		REFERENCE_TIME rtAvgTimePerFrame = rtStop - rtStart;
		if (CComQIPtr<ISubClock2> pSC2 = m_pSubClock) {
			REFERENCE_TIME rt;
			if (S_OK == pSC2->GetAvgTimePerFrame(&rt)) {
				rtAvgTimePerFrame = rt;
			}
		}

		m_fps = 10000000.0/rtAvgTimePerFrame / dRate;
	}

	//

	{
		CAutoLock cAutoLock(&m_csQueueLock);

		if (m_pSubPicQueue) {
			m_pSubPicQueue->SetTime(CalcCurrentTime());
			m_pSubPicQueue->SetFPS(m_fps);
		}
	}

	//

	BYTE* pDataIn = NULL;
	if (FAILED(pIn->GetPointer(&pDataIn)) || !pDataIn) {
		return S_FALSE;
	}

	const CMediaType& mt = m_pInput->CurrentMediaType();

	BITMAPINFOHEADER bihIn;
	ExtractBIH(&mt, &bihIn);

	bool fYV12 = (mt.subtype == MEDIASUBTYPE_YV12 || mt.subtype == MEDIASUBTYPE_I420 || mt.subtype == MEDIASUBTYPE_IYUV);
	int bpp = fYV12 ? 8 : bihIn.biBitCount;
	DWORD black = fYV12 ? 0x10101010 : (bihIn.biCompression == '2YUY') ? 0x80108010 : 0;

	CSize sub(m_w, m_h);
	CSize in(bihIn.biWidth, bihIn.biHeight);

	if (FAILED(Copy((BYTE*)m_pTempPicBuff, pDataIn, sub, in, bpp, mt.subtype, black))) {
		return E_FAIL;
	}

	if (fYV12) {
		BYTE* pSubV = (BYTE*)m_pTempPicBuff + (sub.cx*bpp>>3)*sub.cy;
		BYTE* pInV = pDataIn + (in.cx*bpp>>3)*in.cy;
		sub.cx >>= 1;
		sub.cy >>= 1;
		in.cx >>= 1;
		in.cy >>= 1;
		BYTE* pSubU = pSubV + (sub.cx*bpp>>3)*sub.cy;
		BYTE* pInU = pInV + (in.cx*bpp>>3)*in.cy;
		if (FAILED(Copy(pSubV, pInV, sub, in, bpp, mt.subtype, 0x80808080))) {
			return E_FAIL;
		}
		if (FAILED(Copy(pSubU, pInU, sub, in, bpp, mt.subtype, 0x80808080))) {
			return E_FAIL;
		}
	}

	//

	SubPicDesc spd = m_spd;

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if (FAILED(hr = GetDeliveryBuffer(spd.w, spd.h, &pOut))
			|| FAILED(hr = pOut->GetPointer(&pDataOut))) {
		return hr;
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);
	pOut->SetSyncPoint(pIn->IsSyncPoint() == S_OK);
	pOut->SetPreroll(pIn->IsPreroll() == S_OK);

	//

	BITMAPINFOHEADER bihOut;
	ExtractBIH(&m_pOutput->CurrentMediaType(), &bihOut);

	bool fInputFlipped = bihIn.biHeight >= 0 && bihIn.biCompression <= 3;
	bool fOutputFlipped = bihOut.biHeight >= 0 && bihOut.biCompression <= 3;

	bool fFlip = fInputFlipped != fOutputFlipped;
	if (m_fFlipPicture) {
		fFlip = !fFlip;
	}
	if (m_fMSMpeg4Fix) {
		fFlip = !fFlip;
	}

	bool fFlipSub = fOutputFlipped;
	if (m_fFlipSubtitles) {
		fFlipSub = !fFlipSub;
	}

	//

	{
		CAutoLock cAutoLock(&m_csQueueLock);

		if (m_pSubPicQueue) {
			CComPtr<ISubPic> pSubPic;
			if (SUCCEEDED(m_pSubPicQueue->LookupSubPic(CalcCurrentTime(), pSubPic)) && pSubPic) {
				CRect r;
				pSubPic->GetDirtyRect(r);

				if (fFlip ^ fFlipSub) {
					spd.h = -spd.h;
				}

				pSubPic->AlphaBlt(r, r, &spd);
			}
		}
	}

	CopyBuffer(pDataOut, (BYTE*)spd.bits, spd.w, abs(spd.h)*(fFlip?-1:1), spd.pitch, mt.subtype);

	PrintMessages(pDataOut);

	return m_pOutput->Deliver(pOut);
}

// CBaseFilter

CBasePin* CDirectVobSubFilter::GetPin(int n)
{
	if (n < __super::GetPinCount()) {
		return __super::GetPin(n);
	}

	n -= __super::GetPinCount();

	if (n >= 0 && (size_t)n < m_pTextInput.GetCount()) {
		return m_pTextInput[n];
	}

	n -= m_pTextInput.GetCount();

	return NULL;
}

int CDirectVobSubFilter::GetPinCount()
{
	return __super::GetPinCount() + m_pTextInput.GetCount();
}

HRESULT CDirectVobSubFilter::JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName)
{
	if (pGraph) {
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		if (!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SEENDIVXWARNING), 0)) {
			unsigned __int64 ver = GetFileVersion(_T("divx_c32.ax"));
			if (((ver >> 48)&0xffff) == 4 && ((ver >> 32)&0xffff) == 2) {
				DWORD dwVersion = GetVersion();
				DWORD dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
				//DWORD dwWindowsMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

				if (dwVersion < 0x80000000 && dwWindowsMajorVersion >= 5) {
					AfxMessageBox(IDS_DIVX_WARNING);
					theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SEENDIVXWARNING), 1);
				}
			}
		}

		/*removeme*/
		if (!g_RegOK) {
			DllRegisterServer();
			g_RegOK = true;
		}
	} else {
		if (m_hSystrayThread) {
			SendMessage(m_tbid.hSystrayWnd, WM_CLOSE, 0, 0);

			if (WaitForSingleObject(m_hSystrayThread, 10000) != WAIT_OBJECT_0) {
				DbgLog((LOG_TRACE, 0, _T("CALL THE AMBULANCE!!!")));
				TerminateThread(m_hSystrayThread, (DWORD)-1);
			}

			m_hSystrayThread = 0;
		}
	}

	return __super::JoinFilterGraph(pGraph, pName);
}

STDMETHODIMP CDirectVobSubFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
	CheckPointer(pInfo, E_POINTER);
	ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

	if (!get_Forced()) {
		return __super::QueryFilterInfo(pInfo);
	}

	wcscpy(pInfo->achName, L"DirectVobSub (forced auto-loading version)");
	pInfo->pGraph = m_pGraph;
	if (m_pGraph) {
		m_pGraph->AddRef();
	}

	return S_OK;
}

// CTransformFilter

HRESULT CDirectVobSubFilter::SetMediaType(PIN_DIRECTION dir, const CMediaType* pmt)
{
	HRESULT hr = __super::SetMediaType(dir, pmt);
	if (FAILED(hr)) {
		return hr;
	}

	if (dir == PINDIR_INPUT) {
		CAutoLock cAutoLock(&m_csReceive);

		REFERENCE_TIME atpf =
			pmt->formattype == FORMAT_VideoInfo ? ((VIDEOINFOHEADER*)pmt->Format())->AvgTimePerFrame :
			pmt->formattype == FORMAT_VideoInfo2 ? ((VIDEOINFOHEADER2*)pmt->Format())->AvgTimePerFrame :
			0;

		m_fps = atpf ? 10000000.0 / atpf : 25;

		if (pmt->formattype == FORMAT_VideoInfo2) {
			m_CurrentVIH2 = *(VIDEOINFOHEADER2*)pmt->Format();
		}

		InitSubPicQueue();
	} else if (dir == PINDIR_OUTPUT) {

	}

	return hr;
}

HRESULT CDirectVobSubFilter::CheckConnect(PIN_DIRECTION dir, IPin* pPin)
{
	if (dir == PINDIR_INPUT) {
	} else if (dir == PINDIR_OUTPUT) {
	}

	return __super::CheckConnect(dir, pPin);
}

HRESULT CDirectVobSubFilter::CompleteConnect(PIN_DIRECTION dir, IPin* pReceivePin)
{
	if (dir == PINDIR_INPUT) {
		CComPtr<IBaseFilter> pFilter;

		// needed when we have a decoder with a version number of 3.x
		if (SUCCEEDED(m_pGraph->FindFilterByName(L"DivX MPEG-4 DVD Video Decompressor ", &pFilter))
				&& (GetFileVersion(_T("divx_c32.ax")) >> 48) <= 4
				|| SUCCEEDED(m_pGraph->FindFilterByName(L"Microcrap MPEG-4 Video Decompressor", &pFilter))
				|| SUCCEEDED(m_pGraph->FindFilterByName(L"Microsoft MPEG-4 Video Decompressor", &pFilter))
				&& (GetFileVersion(_T("mpg4ds32.ax")) >> 48) <= 3) {
			m_fMSMpeg4Fix = true;
		}
	} else if (dir == PINDIR_OUTPUT) {
		if (!m_hSystrayThread) {
			m_tbid.graph = m_pGraph;
			m_tbid.dvs = static_cast<IDirectVobSub*>(this);

			DWORD tid;
			m_hSystrayThread = CreateThread(0, 0, SystrayThreadProc, &m_tbid, 0, &tid);
		}

		// HACK: triggers CBaseVideoFilter::SetMediaType to adjust m_w/m_h/.. and InitSubPicQueue() to realloc buffers
		m_pInput->SetMediaType(&m_pInput->CurrentMediaType());
	}

	return __super::CompleteConnect(dir, pReceivePin);
}

HRESULT CDirectVobSubFilter::BreakConnect(PIN_DIRECTION dir)
{
	if (dir == PINDIR_INPUT) {
		if (m_pOutput->IsConnected()) {
			m_pOutput->GetConnected()->Disconnect();
			m_pOutput->Disconnect();
		}
	} else if (dir == PINDIR_OUTPUT) {
		// not really needed, but may free up a little memory
		CAutoLock cAutoLock(&m_csQueueLock);
		m_pSubPicQueue = NULL;
	}

	return __super::BreakConnect(dir);
}

HRESULT CDirectVobSubFilter::StartStreaming()
{
	// WARNING : calls to m_pGraph member functions from here will generate deadlock with Haali Renderer
	// within MPC-HC (reason is CAutoLock's variables in IFilterGraph functions overriden by CFGManager class)

	m_fLoading = false;

	InitSubPicQueue();

	m_tbid.fRunOnce = true;

	put_MediaFPS(m_fMediaFPSEnabled, m_MediaFPS);

	return __super::StartStreaming();
}

HRESULT CDirectVobSubFilter::StopStreaming()
{
	InvalidateSubtitle();

	return __super::StopStreaming();
}

HRESULT CDirectVobSubFilter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	m_tPrev = tStart;
	return __super::NewSegment(tStart, tStop, dRate);
}

//

REFERENCE_TIME CDirectVobSubFilter::CalcCurrentTime()
{
	REFERENCE_TIME rt = m_pSubClock ? m_pSubClock->GetTime() : m_tPrev;
	return (rt - 10000i64*m_SubtitleDelay) * m_SubtitleSpeedMul / m_SubtitleSpeedDiv; // no, it won't overflow if we use normal parameters (__int64 is enough for about 2000 hours if we multiply it by the max: 65536 as m_SubtitleSpeedMul)
}

void CDirectVobSubFilter::InitSubPicQueue()
{
	CAutoLock cAutoLock(&m_csQueueLock);

	m_pSubPicQueue = NULL;

	m_pTempPicBuff.Free();
	m_pTempPicBuff.Allocate(4*m_w*m_h);

	const GUID& subtype = m_pInput->CurrentMediaType().subtype;

	BITMAPINFOHEADER bihIn;
	ExtractBIH(&m_pInput->CurrentMediaType(), &bihIn);

	m_spd.type = -1;
	if (subtype == MEDIASUBTYPE_YV12) {
		m_spd.type = MSP_YV12;
	} else if (subtype == MEDIASUBTYPE_I420 || subtype == MEDIASUBTYPE_IYUV) {
		m_spd.type = MSP_IYUV;
	} else if (subtype == MEDIASUBTYPE_YUY2) {
		m_spd.type = MSP_YUY2;
	} else if (subtype == MEDIASUBTYPE_RGB32) {
		m_spd.type = MSP_RGB32;
	} else if (subtype == MEDIASUBTYPE_RGB24) {
		m_spd.type = MSP_RGB24;
	} else if (subtype == MEDIASUBTYPE_RGB565) {
		m_spd.type = MSP_RGB16;
	} else if (subtype == MEDIASUBTYPE_RGB555) {
		m_spd.type = MSP_RGB15;
	}
	m_spd.w = m_w;
	m_spd.h = m_h;
	m_spd.bpp = (m_spd.type == MSP_YV12 || m_spd.type == MSP_IYUV) ? 8 : bihIn.biBitCount;
	m_spd.pitch = m_spd.w*m_spd.bpp>>3;
	m_spd.bits = (void*)m_pTempPicBuff;

	CComPtr<ISubPicAllocator> pSubPicAllocator = new CMemSubPicAllocator(m_spd.type, CSize(m_w, m_h));

	CSize video(bihIn.biWidth, bihIn.biHeight), window = video;
	if (AdjustFrameSize(window)) {
		video += video;
	}
	ASSERT(window == CSize(m_w, m_h));

	pSubPicAllocator->SetCurSize(window);
	pSubPicAllocator->SetCurVidRect(CRect(CPoint((window.cx - video.cx)/2, (window.cy - video.cy)/2), video));

	HRESULT hr = S_OK;

	m_pSubPicQueue = m_uSubPictToBuffer > 0
					 ? (ISubPicQueue*)new CSubPicQueue(m_uSubPictToBuffer, !m_fAnimWhenBuffering, pSubPicAllocator, &hr)
					 : (ISubPicQueue*)new CSubPicQueueNoThread(pSubPicAllocator, &hr);

	if (FAILED(hr)) {
		m_pSubPicQueue = NULL;
	}

	UpdateSubtitle(false);

	if (m_hbm) {
		DeleteObject(m_hbm);
		m_hbm = NULL;
	}
	if (m_hdc) {
		DeleteDC(m_hdc);
		m_hdc = NULL;
	}

	struct {
		BITMAPINFOHEADER bih;
		DWORD mask[3];
	} b = {{sizeof(BITMAPINFOHEADER), m_w, -(int)m_h, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}, 0xFF0000, 0x00FF00, 0x0000FF};
	m_hdc = CreateCompatibleDC(NULL);
	m_hbm = CreateDIBSection(m_hdc, (BITMAPINFO*)&b, DIB_RGB_COLORS, NULL, NULL, 0);

	BITMAP bm;
	GetObject(m_hbm, sizeof(bm), &bm);
	memsetd(bm.bmBits, 0xFF000000, bm.bmHeight*bm.bmWidthBytes);
}

bool CDirectVobSubFilter::AdjustFrameSize(CSize& s)
{
	int horizontal, vertical, resx2, resx2minw, resx2minh;
	get_ExtendPicture(&horizontal, &vertical, &resx2, &resx2minw, &resx2minh);

	bool fRet = (resx2 == 1) || (resx2 == 2 && s.cx*s.cy <= resx2minw*resx2minh);

	if (fRet) {
		s.cx <<= 1;
		s.cy <<= 1;
	}

	int h;
	switch (vertical&0x7f) {
		case 1:
			h = s.cx * 9 / 16;
			if (s.cy < h || !!(vertical&0x80)) {
				s.cy = (h + 3) & ~3;
			}
			break;
		case 2:
			h = s.cx * 3 / 4;
			if (s.cy < h || !!(vertical&0x80)) {
				s.cy = (h + 3) & ~3;
			}
			break;
		case 3:
			h = 480;
			if (s.cy < h || !!(vertical&0x80)) {
				s.cy = (h + 3) & ~3;
			}
			break;
		case 4:
			h = 576;
			if (s.cy < h || !!(vertical&0x80)) {
				s.cy = (h + 3) & ~3;
			}
			break;
	}

	if (horizontal == 1) {
		s.cx = (s.cx + 31) & ~31;
		s.cy = (s.cy + 1) & ~1;
	}

	return(fRet);
}

STDMETHODIMP CDirectVobSubFilter::Count(DWORD* pcStreams)
{
	if (!pcStreams) {
		return E_POINTER;
	}

	*pcStreams = 0;

	int nLangs = 0;
	if (SUCCEEDED(get_LanguageCount(&nLangs))) {
		(*pcStreams) += nLangs;
	}

	(*pcStreams) += 2; // enable ... disable

	(*pcStreams) += 2; // normal flipped

	return S_OK;
}

#define MAXPREFLANGS 5

int CDirectVobSubFilter::FindPreferedLanguage(bool fHideToo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int nLangs;
	get_LanguageCount(&nLangs);

	if (nLangs <= 0) {
		return(0);
	}

	for (ptrdiff_t i = 0; i < MAXPREFLANGS; i++) {
		CString tmp;
		tmp.Format(IDS_RL_LANG, i);

		CString lang = theApp.GetProfileString(ResStr(IDS_R_PREFLANGS), tmp);

		if (!lang.IsEmpty()) {
			for (ptrdiff_t ret = 0; ret < nLangs; ret++) {
				CString l;
				WCHAR* pName = NULL;
				get_LanguageName(ret, &pName);
				l = pName;
				CoTaskMemFree(pName);

				if (!l.CompareNoCase(lang)) {
					return(ret);
				}
			}
		}
	}

	return(0);
}

void CDirectVobSubFilter::UpdatePreferedLanguages(CString l)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString langs[MAXPREFLANGS+1];

	int i = 0, j = 0, k = -1;
	for (; i < MAXPREFLANGS; i++) {
		CString tmp;
		tmp.Format(IDS_RL_LANG, i);

		langs[j] = theApp.GetProfileString(ResStr(IDS_R_PREFLANGS), tmp);

		if (!langs[j].IsEmpty()) {
			if (!langs[j].CompareNoCase(l)) {
				k = j;
			}
			j++;
		}
	}

	if (k == -1) {
		langs[k = j] = l;
		j++;
	}

	// move the selected to the top of the list

	while (k > 0) {
		CString tmp = langs[k];
		langs[k] = langs[k-1];
		langs[k-1] = tmp;
		k--;
	}

	// move "Hide subtitles" to the last position if it wasn't our selection

	CString hidesubs;
	hidesubs.LoadString(IDS_M_HIDESUBTITLES);

	for (k = 1; k < j; k++) {
		if (!langs[k].CompareNoCase(hidesubs)) {
			break;
		}
	}

	while (k < j-1) {
		CString tmp = langs[k];
		langs[k] = langs[k+1];
		langs[k+1] = tmp;
		k++;
	}

	for (i = 0; i < j; i++) {
		CString tmp;
		tmp.Format(IDS_RL_LANG, i);

		theApp.WriteProfileString(ResStr(IDS_R_PREFLANGS), tmp, langs[i]);
	}
}

STDMETHODIMP CDirectVobSubFilter::Enable(long lIndex, DWORD dwFlags)
{
	if (!(dwFlags & AMSTREAMSELECTENABLE_ENABLE)) {
		return E_NOTIMPL;
	}

	int nLangs = 0;
	get_LanguageCount(&nLangs);

	if (!(lIndex >= 0 && lIndex < nLangs+2+2)) {
		return E_INVALIDARG;
	}

	int i = lIndex-1;

	if (i == -1 && !m_fLoading) { // we need this because when loading something stupid media player pushes the first stream it founds, which is "enable" in our case
		put_HideSubtitles(false);
	} else if (i >= 0 && i < nLangs) {
		put_HideSubtitles(false);
		put_SelectedLanguage(i);

		WCHAR* pName = NULL;
		if (SUCCEEDED(get_LanguageName(i, &pName))) {
			UpdatePreferedLanguages(CString(pName));
			if (pName) {
				CoTaskMemFree(pName);
			}
		}
	} else if (i == nLangs && !m_fLoading) {
		put_HideSubtitles(true);
	} else if ((i == nLangs+1 || i == nLangs+2) && !m_fLoading) {
		put_Flip(i == nLangs+2, m_fFlipSubtitles);
	}

	return S_OK;
}

STDMETHODIMP CDirectVobSubFilter::Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int nLangs = 0;
	get_LanguageCount(&nLangs);

	if (!(lIndex >= 0 && lIndex < nLangs+2+2)) {
		return E_INVALIDARG;
	}

	int i = lIndex-1;

	if (ppmt) {
		*ppmt = CreateMediaType(&m_pInput->CurrentMediaType());
	}

	if (pdwFlags) {
		*pdwFlags = 0;

		if (i == -1 && !m_fHideSubtitles
				|| i >= 0 && i < nLangs && i == m_iSelectedLanguage
				|| i == nLangs && m_fHideSubtitles
				|| i == nLangs+1 && !m_fFlipPicture
				|| i == nLangs+2 && m_fFlipPicture) {
			*pdwFlags |= AMSTREAMSELECTINFO_ENABLED;
		}
	}

	if (plcid) {
		*plcid = 0;
	}

	if (pdwGroup) {
		*pdwGroup = 0x648E51;
	}

	if (ppszName) {
		*ppszName = NULL;

		CStringW str;
		if (i == -1) {
			str = ResStr(IDS_M_SHOWSUBTITLES);
		} else if (i >= 0 && i < nLangs) {
			get_LanguageName(i, ppszName);
		} else if (i == nLangs) {
			str = ResStr(IDS_M_HIDESUBTITLES);
		} else if (i == nLangs+1) {
			str = ResStr(IDS_M_ORIGINALPICTURE);
			if (pdwGroup) {
				(*pdwGroup)++;
			}
		} else if (i == nLangs+2) {
			str = ResStr(IDS_M_FLIPPEDPICTURE);
			if (pdwGroup) {
				(*pdwGroup)++;
			}
		}

		if (!str.IsEmpty()) {
			*ppszName = (WCHAR*)CoTaskMemAlloc((str.GetLength()+1)*sizeof(WCHAR));
			if (*ppszName == NULL) {
				return S_FALSE;
			}
			wcscpy(*ppszName, str);
		}
	}

	if (ppObject) {
		*ppObject = NULL;
	}

	if (ppUnk) {
		*ppUnk = NULL;
	}

	return S_OK;
}

STDMETHODIMP CDirectVobSubFilter::GetClassID(CLSID* pClsid)
{
	if (pClsid == NULL) {
		return E_POINTER;
	}
	*pClsid = m_clsid;
	return NOERROR;
}

STDMETHODIMP CDirectVobSubFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems = 7;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID)*pPages->cElems);

	if (pPages->pElems == NULL) {
		return E_OUTOFMEMORY;
	}

	int i = 0;
	pPages->pElems[i++] = __uuidof(CDVSMainPPage);
	pPages->pElems[i++] = __uuidof(CDVSGeneralPPage);
	pPages->pElems[i++] = __uuidof(CDVSMiscPPage);
	pPages->pElems[i++] = __uuidof(CDVSTimingPPage);
	pPages->pElems[i++] = __uuidof(CDVSColorPPage);
	pPages->pElems[i++] = __uuidof(CDVSPathsPPage);
	pPages->pElems[i++] = __uuidof(CDVSAboutPPage);

	return NOERROR;
}

// IDirectVobSub

STDMETHODIMP CDirectVobSubFilter::put_FileName(WCHAR* fn)
{
	HRESULT hr = CDirectVobSub::put_FileName(fn);

	if (hr == S_OK && !Open()) {
		m_FileName.Empty();
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::get_LanguageCount(int* nLangs)
{
	HRESULT hr = CDirectVobSub::get_LanguageCount(nLangs);

	if (hr == NOERROR && nLangs) {
		CAutoLock cAutolock(&m_csQueueLock);

		*nLangs = 0;
		POSITION pos = m_pSubStreams.GetHeadPosition();
		while (pos) {
			(*nLangs) += m_pSubStreams.GetNext(pos)->GetStreamCount();
		}
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::get_LanguageName(int iLanguage, WCHAR** ppName)
{
	HRESULT hr = CDirectVobSub::get_LanguageName(iLanguage, ppName);

	if (!ppName) {
		return E_POINTER;
	}

	if (hr == NOERROR) {
		CAutoLock cAutolock(&m_csQueueLock);

		hr = E_INVALIDARG;

		int i = iLanguage;

		POSITION pos = m_pSubStreams.GetHeadPosition();
		while (i >= 0 && pos) {
			CComPtr<ISubStream> pSubStream = m_pSubStreams.GetNext(pos);

			if (i < pSubStream->GetStreamCount()) {
				pSubStream->GetStreamInfo(i, ppName, NULL);
				hr = NOERROR;
				break;
			}

			i -= pSubStream->GetStreamCount();
		}
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_SelectedLanguage(int iSelected)
{
	HRESULT hr = CDirectVobSub::put_SelectedLanguage(iSelected);

	if (hr == NOERROR) {
		UpdateSubtitle(false);
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_HideSubtitles(bool fHideSubtitles)
{
	HRESULT hr = CDirectVobSub::put_HideSubtitles(fHideSubtitles);

	if (hr == NOERROR) {
		UpdateSubtitle(false);
	}

	return hr;
}

// deprecated
STDMETHODIMP CDirectVobSubFilter::put_PreBuffering(bool fDoPreBuffering)
{
	HRESULT hr = CDirectVobSub::put_PreBuffering(fDoPreBuffering);

	if (hr == NOERROR && m_pInput && m_pInput->IsConnected()) {
		InitSubPicQueue();
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_SubPictToBuffer(unsigned int uSubPictToBuffer)
{
	HRESULT hr = CDirectVobSub::put_SubPictToBuffer(uSubPictToBuffer);

	if (hr == NOERROR && m_pInput && m_pInput->IsConnected()) {
		InitSubPicQueue();
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_AnimWhenBuffering(bool fAnimWhenBuffering)
{
	HRESULT hr = CDirectVobSub::put_AnimWhenBuffering(fAnimWhenBuffering);

	if (hr == NOERROR && m_pInput && m_pInput->IsConnected()) {
		InitSubPicQueue();
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_Placement(bool fOverridePlacement, int xperc, int yperc)
{
	HRESULT hr = CDirectVobSub::put_Placement(fOverridePlacement, xperc, yperc);

	if (hr == NOERROR) {
		UpdateSubtitle(true);
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_VobSubSettings(bool fBuffer, bool fOnlyShowForcedSubs, bool fReserved)
{
	HRESULT hr = CDirectVobSub::put_VobSubSettings(fBuffer, fOnlyShowForcedSubs, fReserved);

	if (hr == NOERROR) {
		//		UpdateSubtitle(false);
		InvalidateSubtitle();
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_TextSettings(void* lf, int lflen, COLORREF color, bool fShadow, bool fOutline, bool fAdvancedRenderer)
{
	HRESULT hr = CDirectVobSub::put_TextSettings(lf, lflen, color, fShadow, fOutline, fAdvancedRenderer);

	if (hr == NOERROR) {
		//		UpdateSubtitle(true);
		InvalidateSubtitle();
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_SubtitleTiming(int delay, int speedmul, int speeddiv)
{
	HRESULT hr = CDirectVobSub::put_SubtitleTiming(delay, speedmul, speeddiv);

	if (hr == NOERROR) {
		InvalidateSubtitle();
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::get_MediaFPS(bool* fEnabled, double* fps)
{
	HRESULT hr = CDirectVobSub::get_MediaFPS(fEnabled, fps);

	CComQIPtr<IMediaSeeking> pMS = m_pGraph;
	double rate;
	if (pMS && SUCCEEDED(pMS->GetRate(&rate))) {
		m_MediaFPS = rate * m_fps;
		if (fps) {
			*fps = m_MediaFPS;
		}
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_MediaFPS(bool fEnabled, double fps)
{
	HRESULT hr = CDirectVobSub::put_MediaFPS(fEnabled, fps);

	CComQIPtr<IMediaSeeking> pMS = m_pGraph;
	if (pMS) {
		if (hr == NOERROR) {
			hr = pMS->SetRate(m_fMediaFPSEnabled ? m_MediaFPS / m_fps : 1.0);
		}

		double dRate;
		if (SUCCEEDED(pMS->GetRate(&dRate))) {
			m_MediaFPS = dRate * m_fps;
		}
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::get_ZoomRect(NORMALIZEDRECT* rect)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDirectVobSubFilter::put_ZoomRect(NORMALIZEDRECT* rect)
{
	return E_NOTIMPL;
}

// IDirectVobSub2

STDMETHODIMP CDirectVobSubFilter::put_TextSettings(STSStyle* pDefStyle)
{
	HRESULT hr = CDirectVobSub::put_TextSettings(pDefStyle);

	if (hr == NOERROR) {
		UpdateSubtitle(true);
	}

	return hr;
}

STDMETHODIMP CDirectVobSubFilter::put_AspectRatioSettings(CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType)
{
	HRESULT hr = CDirectVobSub::put_AspectRatioSettings(ePARCompensationType);

	if (hr == NOERROR) {
		UpdateSubtitle(true);
	}

	return hr;
}

// IDirectVobSubFilterColor

STDMETHODIMP CDirectVobSubFilter::HasConfigDialog(int iSelected)
{
	int nLangs;
	if (FAILED(get_LanguageCount(&nLangs))) {
		return E_FAIL;
	}
	return E_FAIL;
	// TODO: temporally disabled since we don't have a new textsub/vobsub editor dlg for dvs yet
	//	return(nLangs >= 0 && iSelected < nLangs ? S_OK : E_FAIL);
}

STDMETHODIMP CDirectVobSubFilter::ShowConfigDialog(int iSelected, HWND hWndParent)
{
	// TODO: temporally disabled since we don't have a new textsub/vobsub editor dlg for dvs yet
	return(E_FAIL);
}

///////////////////////////////////////////////////////////////////////////

CDirectVobSubFilter2::CDirectVobSubFilter2(LPUNKNOWN punk, HRESULT* phr, const GUID& clsid) :
	CDirectVobSubFilter(punk, phr, clsid)
{
}

HRESULT CDirectVobSubFilter2::CheckConnect(PIN_DIRECTION dir, IPin* pPin)
{
	CPinInfo pi;
	if (FAILED(pPin->QueryPinInfo(&pi))) {
		return E_FAIL;
	}

	if (CComQIPtr<IDirectVobSub>(pi.pFilter)) {
		return E_FAIL;
	}

	if (dir == PINDIR_INPUT) {
		CFilterInfo fi;
		if (SUCCEEDED(pi.pFilter->QueryFilterInfo(&fi))
				&& !_wcsnicmp(fi.achName, L"Overlay Mixer", 13)) {
			return(E_FAIL);
		}
	} else {
	}

	return __super::CheckConnect(dir, pPin);
}

HRESULT CDirectVobSubFilter2::JoinFilterGraph(IFilterGraph* pGraph, LPCWSTR pName)
{
	if (pGraph) {
		BeginEnumFilters(pGraph, pEF, pBF) {
			if (pBF != (IBaseFilter*)this && CComQIPtr<IDirectVobSub>(pBF)) {
				return E_FAIL;
			}
		}
		EndEnumFilters

		// don't look... we will do some serious graph hacking again...
		//
		// we will add dvs2 to the filter graph cache
		// - if the main app has already added some kind of renderer or overlay mixer (anything which accepts video on its input)
		// and
		// - if we have a reason to auto-load (we don't want to make any trouble when there is no need :)
		//
		// This whole workaround is needed because the video stream will always be connected
		// to the pre-added filters first, no matter how high merit we have.

		if (!get_Forced()) {
			BeginEnumFilters(pGraph, pEF, pBF) {
				if (CComQIPtr<IDirectVobSub>(pBF)) {
					continue;
				}

				CComPtr<IPin> pInPin = GetFirstPin(pBF, PINDIR_INPUT);
				CComPtr<IPin> pOutPin = GetFirstPin(pBF, PINDIR_OUTPUT);

				if (!pInPin) {
					continue;
				}

				CComPtr<IPin> pPin;
				if (pInPin && SUCCEEDED(pInPin->ConnectedTo(&pPin))
						|| pOutPin && SUCCEEDED(pOutPin->ConnectedTo(&pPin))) {
					continue;
				}

				if (pOutPin && GetFilterName(pBF) == _T("Overlay Mixer")) {
					continue;
				}

				bool fVideoInputPin = false;

				do {
					BITMAPINFOHEADER bih = {sizeof(BITMAPINFOHEADER), 384, 288, 1, 16, '2YUY', 384*288*2, 0, 0, 0, 0};

					CMediaType cmt;
					cmt.majortype = MEDIATYPE_Video;
					cmt.subtype = MEDIASUBTYPE_YUY2;
					cmt.formattype = FORMAT_VideoInfo;
					cmt.pUnk = NULL;
					cmt.bFixedSizeSamples = TRUE;
					cmt.bTemporalCompression = TRUE;
					cmt.lSampleSize = 384*288*2;
					VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)cmt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
					memset(vih, 0, sizeof(VIDEOINFOHEADER));
					memcpy(&vih->bmiHeader, &bih, sizeof(bih));
					vih->AvgTimePerFrame = 400000;

					if (SUCCEEDED(pInPin->QueryAccept(&cmt))) {
						fVideoInputPin = true;
						break;
					}

					VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)cmt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
					memset(vih2, 0, sizeof(VIDEOINFOHEADER2));
					memcpy(&vih2->bmiHeader, &bih, sizeof(bih));
					vih2->AvgTimePerFrame = 400000;
					vih2->dwPictAspectRatioX = 384;
					vih2->dwPictAspectRatioY = 288;

					if (SUCCEEDED(pInPin->QueryAccept(&cmt))) {
						fVideoInputPin = true;
						break;
					}
				} while (false);

				if (fVideoInputPin) {
					CComPtr<IBaseFilter> pDVS;
					if (ShouldWeAutoload(pGraph) && SUCCEEDED(pDVS.CoCreateInstance(__uuidof(CDirectVobSubFilter2)))) {
						CComQIPtr<IDirectVobSub2>(pDVS)->put_Forced(true);
						CComQIPtr<IGraphConfig>(pGraph)->AddFilterToCache(pDVS);
					}

					break;
				}
			}
			EndEnumFilters
		}
	} else {
	}

	return __super::JoinFilterGraph(pGraph, pName);
}

HRESULT CDirectVobSubFilter2::CheckInputType(const CMediaType* mtIn)
{
	HRESULT hr = __super::CheckInputType(mtIn);

	if (FAILED(hr) || m_pInput->IsConnected()) {
		return hr;
	}

	if (!ShouldWeAutoload(m_pGraph)) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	GetRidOfInternalScriptRenderer();

	return NOERROR;
}

bool CDirectVobSubFilter2::ShouldWeAutoload(IFilterGraph* pGraph)
{
	TCHAR blacklistedapps[][32] = {
		_T("WM8EUTIL."), // wmp8 encoder's dummy renderer releases the outputted media sample after calling Receive on its input pin (yes, even when dvobsub isn't registered at all)
		_T("explorer."), // as some users reported thumbnail preview loads dvobsub, I've never experienced this yet...
		_T("producer."), // this is real's producer
		_T("GoogleDesktopIndex."), // Google Desktop
		_T("GoogleDesktopDisplay."), // Google Desktop
		_T("GoogleDesktopCrawl."), // Google Desktop
	};

	for (ptrdiff_t i = 0; i < countof(blacklistedapps); i++) {
		if (theApp.m_AppName.Find(blacklistedapps[i]) >= 0) {
			return false;
		}
	}

	int level;
	bool m_fExternalLoad, m_fWebLoad, m_fEmbeddedLoad;
	get_LoadSettings(&level, &m_fExternalLoad, &m_fWebLoad, &m_fEmbeddedLoad);

	if (level < 0 || level >= 2) {
		return false;
	}

	bool fRet = false;

	if (level == 1) {
		fRet = m_fExternalLoad = m_fWebLoad = m_fEmbeddedLoad = true;
	}

	// find text stream on known splitters

	if (!fRet && m_fEmbeddedLoad) {
		CComPtr<IBaseFilter> pBF;
		if ((pBF = FindFilter(CLSID_OggSplitter, pGraph)) || (pBF = FindFilter(CLSID_AviSplitter, pGraph))
				|| (pBF = FindFilter(L"{34293064-02F2-41D5-9D75-CC5967ACA1AB}", pGraph)) // matroska demux
				|| (pBF = FindFilter(L"{0A68C3B5-9164-4a54-AFAF-995B2FF0E0D4}", pGraph)) // matroska source
				|| (pBF = FindFilter(L"{149D2E01-C32E-4939-80F6-C07B81015A7A}", pGraph)) // matroska splitter
				|| (pBF = FindFilter(L"{55DA30FC-F16B-49fc-BAA5-AE59FC65F82D}", pGraph)) // Haali's matroska splitter
				|| (pBF = FindFilter(L"{564FD788-86C9-4444-971E-CC4A243DA150}", pGraph)) // Haali's matroska splitter (AR)
				|| (pBF = FindFilter(L"{171252A0-8820-4AFE-9DF8-5C92B2D66B04}", pGraph)) // LAV Splitter
				|| (pBF = FindFilter(L"{B98D13E7-55DB-4385-A33D-09FD1BA26338}", pGraph)) // LAV Splitter Source
				|| (pBF = FindFilter(L"{E436EBB5-524F-11CE-9F53-0020AF0BA770}", pGraph)) // Solveig matroska splitter
				|| (pBF = FindFilter(L"{52B63861-DC93-11CE-A099-00AA00479A58}", pGraph)) // 3ivx splitter
				|| (pBF = FindFilter(L"{6D3688CE-3E9D-42F4-92CA-8A11119D25CD}", pGraph)) // our ogg source
				|| (pBF = FindFilter(L"{9FF48807-E133-40AA-826F-9B2959E5232D}", pGraph)) // our ogg splitter
				|| (pBF = FindFilter(L"{803E8280-F3CE-4201-982C-8CD8FB512004}", pGraph)) // dsm source
				|| (pBF = FindFilter(L"{0912B4DD-A30A-4568-B590-7179EBB420EC}", pGraph)) // dsm splitter
				|| (pBF = FindFilter(L"{3CCC052E-BDEE-408a-BEA7-90914EF2964B}", pGraph)) // mp4 source
				|| (pBF = FindFilter(L"{61F47056-E400-43d3-AF1E-AB7DFFD4C4AD}", pGraph))) { // mp4 splitter
			BeginEnumPins(pBF, pEP, pPin) {
				BeginEnumMediaTypes(pPin, pEM, pmt) {
					if (pmt->majortype == MEDIATYPE_Text || pmt->majortype == MEDIATYPE_Subtitle) {
						fRet = true;
						break;
					}
				}
				EndEnumMediaTypes(pmt)
				if (fRet) {
					break;
				}
			}
			EndEnumFilters
		}
	}

	// find file name

	CStringW fn;

	BeginEnumFilters(pGraph, pEF, pBF) {
		if (CComQIPtr<IFileSourceFilter> pFSF = pBF) {
			LPOLESTR fnw = NULL;
			if (!pFSF || FAILED(pFSF->GetCurFile(&fnw, NULL)) || !fnw) {
				continue;
			}
			fn = CString(fnw);
			CoTaskMemFree(fnw);
			break;
		}
	}
	EndEnumFilters

	if ((m_fExternalLoad || m_fWebLoad) && (m_fWebLoad || !(wcsstr(fn, L"http://") || wcsstr(fn, L"mms://")))) {
		bool fTemp = m_fHideSubtitles;
		fRet = !fn.IsEmpty() && SUCCEEDED(put_FileName((LPWSTR)(LPCWSTR)fn))
			   || SUCCEEDED(put_FileName(L"c:\\tmp.srt"))
			   || fRet;
		if (fTemp) {
			m_fHideSubtitles = true;
		}
	}

	return(fRet);
}

void CDirectVobSubFilter2::GetRidOfInternalScriptRenderer()
{
	while (CComPtr<IBaseFilter> pBF = FindFilter(L"{48025243-2D39-11CE-875D-00608CB78066}", m_pGraph)) {
		BeginEnumPins(pBF, pEP, pPin) {
			PIN_DIRECTION dir;
			CComPtr<IPin> pPinTo;

			if (SUCCEEDED(pPin->QueryDirection(&dir)) && dir == PINDIR_INPUT
					&& SUCCEEDED(pPin->ConnectedTo(&pPinTo))) {
				m_pGraph->Disconnect(pPinTo);
				m_pGraph->Disconnect(pPin);
				m_pGraph->ConnectDirect(pPinTo, GetPin(2 + m_pTextInput.GetCount()-1), NULL);
			}
		}
		EndEnumPins

		if (FAILED(m_pGraph->RemoveFilter(pBF))) {
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

bool CDirectVobSubFilter::Open()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CAutoLock cAutolock(&m_csQueueLock);

	m_pSubStreams.RemoveAll();

	m_frd.files.RemoveAll();

	CAtlArray<CString> paths;

	for (ptrdiff_t i = 0; i < 10; i++) {
		CString tmp;
		tmp.Format(IDS_RP_PATH, i);
		CString path = theApp.GetProfileString(ResStr(IDS_R_DEFTEXTPATHES), tmp);
		if (!path.IsEmpty()) {
			paths.Add(path);
		}
	}

	CAtlArray<SubFile> ret;
	GetSubFileNames(m_FileName, paths, ret);

	for (size_t i = 0; i < ret.GetCount(); i++) {
		if (m_frd.files.Find(ret[i].fn)) {
			continue;
		}

		CComPtr<ISubStream> pSubStream;

		if (!pSubStream) {
			CAutoPtr<CVobSubFile> pVSF(new CVobSubFile(&m_csSubLock));
			if (pVSF && pVSF->Open(ret[i].fn) && pVSF->GetStreamCount() > 0) {
				pSubStream = pVSF.Detach();
				m_frd.files.AddTail(ret[i].fn.Left(ret[i].fn.GetLength()-4) + _T(".sub"));
			}
		}

		if (!pSubStream) {
			CAutoPtr<CRenderedTextSubtitle> pRTS(new CRenderedTextSubtitle(&m_csSubLock));
			if (pRTS && pRTS->Open(ret[i].fn, DEFAULT_CHARSET) && pRTS->GetStreamCount() > 0) {
				pSubStream = pRTS.Detach();
				m_frd.files.AddTail(ret[i].fn + _T(".style"));
			}
		}

		if (pSubStream) {
			m_pSubStreams.AddTail(pSubStream);
			m_frd.files.AddTail(ret[i].fn);
		}
	}

	for (size_t i = 0; i < m_pTextInput.GetCount(); i++) {
		if (m_pTextInput[i]->IsConnected()) {
			m_pSubStreams.AddTail(m_pTextInput[i]->GetSubStream());
		}
	}

	if (S_FALSE == put_SelectedLanguage(FindPreferedLanguage())) {
		UpdateSubtitle(false);    // make sure pSubPicProvider of our queue gets updated even if the stream number hasn't changed
	}

	m_frd.RefreshEvent.Set();

	return(m_pSubStreams.GetCount() > 0);
}

void CDirectVobSubFilter::UpdateSubtitle(bool fApplyDefStyle)
{
	CAutoLock cAutolock(&m_csQueueLock);

	if (!m_pSubPicQueue) {
		return;
	}

	InvalidateSubtitle();

	CComPtr<ISubStream> pSubStream;

	if (!m_fHideSubtitles) {
		int i = m_iSelectedLanguage;

		for (POSITION pos = m_pSubStreams.GetHeadPosition(); i >= 0 && pos; pSubStream = NULL) {
			pSubStream = m_pSubStreams.GetNext(pos);

			if (i < pSubStream->GetStreamCount()) {
				CAutoLock cAutoLock(&m_csSubLock);
				pSubStream->SetStream(i);
				break;
			}

			i -= pSubStream->GetStreamCount();
		}
	}

	SetSubtitle(pSubStream, fApplyDefStyle);
}

void CDirectVobSubFilter::SetSubtitle(ISubStream* pSubStream, bool fApplyDefStyle)
{
	CAutoLock cAutolock(&m_csQueueLock);

	if (pSubStream) {
		CAutoLock cAutolock(&m_csSubLock);

		CLSID clsid;
		pSubStream->GetClassID(&clsid);

		if (clsid == __uuidof(CVobSubFile)) {
			CVobSubSettings* pVSS = (CVobSubFile*)(ISubStream*)pSubStream;

			if (fApplyDefStyle) {
				pVSS->SetAlignment(m_fOverridePlacement, m_PlacementXperc, m_PlacementYperc, 1, 1);
				pVSS->m_fOnlyShowForcedSubs = m_fOnlyShowForcedVobSubs;
			}
		} else if (clsid == __uuidof(CVobSubStream)) {
			CVobSubSettings* pVSS = (CVobSubStream*)(ISubStream*)pSubStream;

			if (fApplyDefStyle) {
				pVSS->SetAlignment(m_fOverridePlacement, m_PlacementXperc, m_PlacementYperc, 1, 1);
				pVSS->m_fOnlyShowForcedSubs = m_fOnlyShowForcedVobSubs;
			}
		} else if (clsid == __uuidof(CRenderedTextSubtitle)) {
			CRenderedTextSubtitle* pRTS = (CRenderedTextSubtitle*)(ISubStream*)pSubStream;

			if (fApplyDefStyle || pRTS->m_fUsingAutoGeneratedDefaultStyle) {
				STSStyle s = m_defStyle;

				if (m_fOverridePlacement) {
					s.scrAlignment = 2;
					int w = pRTS->m_dstScreenSize.cx;
					int h = pRTS->m_dstScreenSize.cy;
					int mw = w - s.marginRect.left - s.marginRect.right;
					s.marginRect.bottom = h - MulDiv(h, m_PlacementYperc, 100);
					s.marginRect.left = MulDiv(w, m_PlacementXperc, 100) - mw/2;
					s.marginRect.right = w - (s.marginRect.left + mw);
				}

				pRTS->SetDefaultStyle(s);
			}

			pRTS->m_ePARCompensationType = m_ePARCompensationType;
			if (m_CurrentVIH2.dwPictAspectRatioX != 0 && m_CurrentVIH2.dwPictAspectRatioY != 0&& m_CurrentVIH2.bmiHeader.biWidth != 0 && m_CurrentVIH2.bmiHeader.biHeight != 0) {
				pRTS->m_dPARCompensation = ((double)abs(m_CurrentVIH2.bmiHeader.biWidth) / (double)abs(m_CurrentVIH2.bmiHeader.biHeight)) /
										   ((double)abs((long)m_CurrentVIH2.dwPictAspectRatioX) / (double)abs((long)m_CurrentVIH2.dwPictAspectRatioY));

			} else {
				pRTS->m_dPARCompensation = 1.00;
			}

			pRTS->Deinit();
		}
	}

	if (!fApplyDefStyle) {
		int i = 0;

		POSITION pos = m_pSubStreams.GetHeadPosition();
		while (pos) {
			CComPtr<ISubStream> pSubStream2 = m_pSubStreams.GetNext(pos);

			if (pSubStream == pSubStream2) {
				m_iSelectedLanguage = i + pSubStream2->GetStream();
				break;
			}

			i += pSubStream2->GetStreamCount();
		}
	}

	m_nSubtitleId = (DWORD_PTR)pSubStream;

	if (m_pSubPicQueue) {
		m_pSubPicQueue->SetSubPicProvider(CComQIPtr<ISubPicProvider>(pSubStream));
	}
}

void CDirectVobSubFilter::InvalidateSubtitle(REFERENCE_TIME rtInvalidate, DWORD_PTR nSubtitleId)
{
	CAutoLock cAutolock(&m_csQueueLock);

	if (m_pSubPicQueue) {
		if (nSubtitleId == -1 || nSubtitleId == m_nSubtitleId) {
			m_pSubPicQueue->Invalidate(rtInvalidate);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

void CDirectVobSubFilter::AddSubStream(ISubStream* pSubStream)
{
	CAutoLock cAutoLock(&m_csQueueLock);

	POSITION pos = m_pSubStreams.Find(pSubStream);
	if (!pos) {
		m_pSubStreams.AddTail(pSubStream);
	}

	int len = m_pTextInput.GetCount();
	for (size_t i = 0; i < m_pTextInput.GetCount(); i++)
		if (m_pTextInput[i]->IsConnected()) {
			len--;
		}

	if (len == 0) {
		HRESULT hr = S_OK;
		m_pTextInput.Add(new CTextInputPin(this, m_pLock, &m_csSubLock, &hr));
	}
}

void CDirectVobSubFilter::RemoveSubStream(ISubStream* pSubStream)
{
	CAutoLock cAutoLock(&m_csQueueLock);

	POSITION pos = m_pSubStreams.Find(pSubStream);
	if (pos) {
		m_pSubStreams.RemoveAt(pos);
	}
}

void CDirectVobSubFilter::Post_EC_OLE_EVENT(CString str, DWORD_PTR nSubtitleId)
{
	if (nSubtitleId != -1 && nSubtitleId != m_nSubtitleId) {
		return;
	}

	CComQIPtr<IMediaEventSink> pMES = m_pGraph;
	if (!pMES) {
		return;
	}

	CComBSTR bstr1("Text"), bstr2(" ");

	str.Trim();
	if (!str.IsEmpty()) {
		bstr2 = CStringA(str);
	}

	pMES->Notify(EC_OLE_EVENT, (LONG_PTR)bstr1.Detach(), (LONG_PTR)bstr2.Detach());
}

////////////////////////////////////////////////////////////////

void CDirectVobSubFilter::SetupFRD(CStringArray& paths, CAtlArray<HANDLE>& handles)
{
	CAutoLock cAutolock(&m_csSubLock);

	for (size_t i = 2; i < handles.GetCount(); i++) {
		FindCloseChangeNotification(handles[i]);
	}

	paths.RemoveAll();
	handles.RemoveAll();

	handles.Add(m_frd.EndThreadEvent);
	handles.Add(m_frd.RefreshEvent);

	m_frd.mtime.SetCount(m_frd.files.GetCount());

	POSITION pos = m_frd.files.GetHeadPosition();
	for (ptrdiff_t i = 0; pos; i++) {
		CString fn = m_frd.files.GetNext(pos);

		CFileStatus status;
		if (CFileGetStatus(fn, status)) {
			m_frd.mtime[i] = status.m_mtime;
		}

		fn.Replace('\\', '/');
		fn = fn.Left(fn.ReverseFind('/')+1);

		bool fFound = false;

		for (ptrdiff_t j = 0; !fFound && j < paths.GetCount(); j++) {
			if (paths[j] == fn) {
				fFound = true;
			}
		}

		if (!fFound) {
			paths.Add(fn);

			HANDLE h = FindFirstChangeNotification(fn, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
			if (h != INVALID_HANDLE_VALUE) {
				handles.Add(h);
			}
		}
	}
}

DWORD CDirectVobSubFilter::ThreadProc()
{
	SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST/*THREAD_PRIORITY_BELOW_NORMAL*/);

	CStringArray paths;
	CAtlArray<HANDLE> handles;

	SetupFRD(paths, handles);

	while (1) {
		DWORD idx = WaitForMultipleObjects(handles.GetCount(), handles.GetData(), FALSE, INFINITE);

		if (idx == (WAIT_OBJECT_0 + 0)) { // m_frd.hEndThreadEvent
			break;
		}
		if (idx == (WAIT_OBJECT_0 + 1)) { // m_frd.hRefreshEvent
			SetupFRD(paths, handles);
		} else if (idx >= (WAIT_OBJECT_0 + 2) && idx < (WAIT_OBJECT_0 + handles.GetCount())) {
			bool fLocked = true;
			IsSubtitleReloaderLocked(&fLocked);
			if (fLocked) {
				continue;
			}

			if (FindNextChangeNotification(handles[idx - WAIT_OBJECT_0]) == FALSE) {
				break;
			}

			int j = 0;

			POSITION pos = m_frd.files.GetHeadPosition();
			for (ptrdiff_t i = 0; pos && j == 0; i++) {
				CString fn = m_frd.files.GetNext(pos);

				CFileStatus status;
				if (CFileGetStatus(fn, status) && m_frd.mtime[i] != status.m_mtime) {
					for (j = 0; j < 10; j++) {
						if (FILE* f = _tfopen(fn, _T("rb+"))) {
							fclose(f);
							j = 0;
							break;
						} else {
							Sleep(100);
							j++;
						}
					}
				}
			}

			if (j > 0) {
				SetupFRD(paths, handles);
			} else {
				Sleep(500);

				POSITION pos = m_frd.files.GetHeadPosition();
				for (ptrdiff_t i = 0; pos; i++) {
					CFileStatus status;
					if (CFileGetStatus(m_frd.files.GetNext(pos), status)
							&& m_frd.mtime[i] != status.m_mtime) {
						Open();
						SetupFRD(paths, handles);
						break;
					}
				}
			}
		} else {
			break;
		}
	}

	for (size_t i = 2; i < handles.GetCount(); i++) {
		FindCloseChangeNotification(handles[i]);
	}

	return 0;
}
