/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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
#include <math.h>
#include <atlbase.h>
#include <ks.h>
#include <ksmedia.h>
#include "libmpeg2.h"
#include "Mpeg2DecFilter.h"

#include <xmmintrin.h>
#include <emmintrin.h>

#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/MediaTypes.h"
#include "../../../DSUtil/GolombBuffer.h"

#include <InitGuid.h>
#include <moreuuids.h>
#include <IFilterVersion.h>

#define EPSILON 1e-4

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_VIDEO},
	{&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_MPEG2_VIDEO},
	{&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_MPEG2_VIDEO},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG2_VIDEO},
#ifndef MPEG2ONLY
	{&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Packet},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Payload},
#endif
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_IYUV},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
#ifdef MPEG2ONLY
	{&__uuidof(CMpeg2DecFilter), L"MPC - MPEG-2 Video Decoder (Gabest)", 0x00600001, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
#else
	{&__uuidof(CMpeg2DecFilter), L"MPC - MPEG Video Decoder (Gabest)", 0x00600001, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
#endif
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CMpeg2DecFilter>, NULL, &sudFilter[0]},
	{L"CMpeg2DecPropertyPage", &__uuidof(CMpeg2DecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMpeg2DecSettingsWnd> >},
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

//

#include <detours/detours.h>

BOOL (__stdcall * Real_IsDebuggerPresent)(void)
= IsDebuggerPresent;

LONG (__stdcall * Real_ChangeDisplaySettingsExA)(LPCSTR a0,
		LPDEVMODEA a1,
		HWND a2,
		DWORD a3,
		LPVOID a4)
= ChangeDisplaySettingsExA;

LONG (__stdcall * Real_ChangeDisplaySettingsExW)(LPCWSTR a0,
		LPDEVMODEW a1,
		HWND a2,
		DWORD a3,
		LPVOID a4)
= ChangeDisplaySettingsExW;


BOOL WINAPI Mine_IsDebuggerPresent()
{
	TRACE(_T("Oops, somebody was trying to be naughty! (called IsDebuggerPresent)\n"));
	return FALSE;
}

LONG WINAPI Mine_ChangeDisplaySettingsEx(LONG ret, DWORD dwFlags, LPVOID lParam)
{
	if(dwFlags&CDS_VIDEOPARAMETERS) {
		VIDEOPARAMETERS* vp = (VIDEOPARAMETERS*)lParam;

		if(vp->Guid == GUIDFromCString(_T("{02C62061-1097-11d1-920F-00A024DF156E}"))
				&& (vp->dwFlags&VP_FLAGS_COPYPROTECT)) {
			if(vp->dwCommand == VP_COMMAND_GET) {
				if((vp->dwTVStandard&VP_TV_STANDARD_WIN_VGA) && vp->dwTVStandard != VP_TV_STANDARD_WIN_VGA) {
					TRACE(_T("Ooops, tv-out enabled? macrovision checks suck..."));
					vp->dwTVStandard = VP_TV_STANDARD_WIN_VGA;
				}
			} else if(vp->dwCommand == VP_COMMAND_SET) {
				TRACE(_T("Ooops, as I already told ya, no need for any macrovision bs here"));
				return 0;
			}
		}
	}

	return ret;
}
LONG WINAPI Mine_ChangeDisplaySettingsExA(LPCSTR lpszDeviceName, LPDEVMODEA lpDevMode, HWND hwnd, DWORD dwFlags, LPVOID lParam)
{
	return Mine_ChangeDisplaySettingsEx(Real_ChangeDisplaySettingsExA(lpszDeviceName, lpDevMode, hwnd, dwFlags, lParam), dwFlags, lParam);
}
LONG WINAPI Mine_ChangeDisplaySettingsExW(LPCWSTR lpszDeviceName, LPDEVMODEW lpDevMode, HWND hwnd, DWORD dwFlags, LPVOID lParam)
{
	return Mine_ChangeDisplaySettingsEx(Real_ChangeDisplaySettingsExW(lpszDeviceName, lpDevMode, hwnd, dwFlags, lParam), dwFlags, lParam);
}

//

#include "../../FilterApp.h"

class CMpeg2DecFilterApp : public CFilterApp
{
public:
	BOOL InitInstance() {
		long		lError;

		if(!__super::InitInstance()) {
			return FALSE;
		}

		DetourRestoreAfterWith();
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)Real_IsDebuggerPresent, (PVOID)Mine_IsDebuggerPresent);
		DetourAttach(&(PVOID&)Real_ChangeDisplaySettingsExA, (PVOID)Mine_ChangeDisplaySettingsExA);
		DetourAttach(&(PVOID&)Real_ChangeDisplaySettingsExW, (PVOID)Mine_ChangeDisplaySettingsExW);

		lError = DetourTransactionCommit();
		ASSERT (lError == NOERROR);

		return TRUE;
	}
};

CMpeg2DecFilterApp theApp;

#endif

//
// CMpeg2DecFilter
//

CMpeg2DecFilter::CMpeg2DecFilter(LPUNKNOWN lpunk, HRESULT* phr)
	: CBaseVideoFilter(NAME("CMpeg2DecFilter"), lpunk, phr, __uuidof(this), 1)
	, m_fWaitForKeyFrame(true)
	, m_fInitializedBuffer(true)
{
	delete m_pInput;
	//	delete m_pOutput;

	if(FAILED(*phr)) {
		return;
	}

	m_pInput = DNew CMpeg2DecInputPin(this, phr, L"Video");
	if(!m_pInput) {
		*phr = E_OUTOFMEMORY;
	}
	if(FAILED(*phr)) {
		return;
	}

	//	m_pOutput = DNew CMpeg2DecOutputPin(this, phr, L"Output");
	//	if(!m_pOutput) *phr = E_OUTOFMEMORY;
	//	if(FAILED(*phr)) return;

	m_pSubpicInput = DNew CSubpicInputPin(this, phr);
	if(!m_pSubpicInput) {
		*phr = E_OUTOFMEMORY;
	}
	if(FAILED(*phr)) {
		return;
	}

	m_pClosedCaptionOutput = DNew CClosedCaptionOutputPin(this, m_pLock, phr);
	if(!m_pClosedCaptionOutput) {
		*phr = E_OUTOFMEMORY;
	}
	if(FAILED(*phr)) {
		return;
	}

	SetDeinterlaceMethod(DIAuto);
	SetBrightness(0.0f);
	SetContrast(1.0f);
	SetHue(0.0f);
	SetSaturation(1.0f);
	EnableForcedSubtitles(true);
	EnablePlanarYUV(true);
	EnableInterlaced(false);
	EnableReadARFromStream(true);

#ifdef REGISTER_FILTER
	CRegKey key;
	if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Video Decoder"), KEY_READ)) {
		DWORD dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DeinterlaceMethod"), dw)) {
			SetDeinterlaceMethod((ditype)dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Brightness"), dw)) {
			SetBrightness(*(float*)&dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Contrast"), dw)) {
			SetContrast(*(float*)&dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Hue"), dw)) {
			SetHue(*(float*)&dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Saturation"), dw)) {
			SetSaturation(*(float*)&dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("ForcedSubtitles"), dw)) {
			EnableForcedSubtitles(!!dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("PlanarYUV"), dw)) {
			EnablePlanarYUV(!!dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Interlaced"), dw)) {
			EnableInterlaced(!!dw);
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("ReadARFromStream"), dw)) {
			EnableReadARFromStream(!!dw);
		}
	}
#else
	DWORD dw;		
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("DeinterlaceMethod"), m_ditype);
	SetDeinterlaceMethod((ditype)dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Brightness"), *(DWORD*)&m_bright);
	SetBrightness(*(float*)&dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Contrast"), *(DWORD*)&m_cont);
	SetContrast(*(float*)&dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Hue"), *(DWORD*)&m_hue);
	SetHue(*(float*)&dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Saturation"), *(DWORD*)&m_sat);
	SetSaturation(*(float*)&dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("ForcedSubtitles"), m_fForcedSubs);
	EnableForcedSubtitles(!!dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("PlanarYUV"), m_fPlanarYUV);
	EnablePlanarYUV(!!dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Interlaced"), m_fInterlaced);
	EnableInterlaced(!!dw);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Video Decoder"), _T("ReadARFromStream"), m_bReadARFromStream);
	EnableReadARFromStream(!!dw);

#endif

	m_rate.Rate = 10000;
	m_rate.StartTime = 0;

	m_par.SetSize(1,1);
}

CMpeg2DecFilter::~CMpeg2DecFilter()
{
	delete m_pSubpicInput;
	delete m_pClosedCaptionOutput;
}

STDMETHODIMP CMpeg2DecFilter::Apply()
{
#ifdef REGISTER_FILTER
	CRegKey key;
	if(ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Video Decoder"))) {
		key.SetDWORDValue(_T("DeinterlaceMethod"), m_ditype);
		key.SetDWORDValue(_T("Brightness"), *(DWORD*)&m_bright);
		key.SetDWORDValue(_T("Contrast"), *(DWORD*)&m_cont);
		key.SetDWORDValue(_T("Hue"), *(DWORD*)&m_hue);
		key.SetDWORDValue(_T("Saturation"), *(DWORD*)&m_sat);
		key.SetDWORDValue(_T("ForcedSubtitles"), m_fForcedSubs);
		key.SetDWORDValue(_T("PlanarYUV"), m_fPlanarYUV);
		key.SetDWORDValue(_T("Interlaced"), m_fInterlaced);
		key.SetDWORDValue(_T("ReadARFromStream"), m_bReadARFromStream);
	}
#else
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("DeinterlaceMethod"), m_ditype);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Brightness"), *(DWORD*)&m_bright);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Contrast"), *(DWORD*)&m_cont);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Hue"), *(DWORD*)&m_hue);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Saturation"), *(DWORD*)&m_sat);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("ForcedSubtitles"), m_fForcedSubs);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("PlanarYUV"), m_fPlanarYUV);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("Interlaced"), m_fInterlaced);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Video Decoder"), _T("ReadARFromStream"), m_bReadARFromStream);
#endif

	return S_OK;
}

void CMpeg2DecFilter::GetOutputSize(int& w, int& h, int& arx, int& ary, int &RealWidth, int &RealHeight)
{
	w = m_dec->m_info.m_sequence->picture_width;
	h = m_dec->m_info.m_sequence->picture_height;
}

STDMETHODIMP CMpeg2DecFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IMpeg2DecFilter)
		QI(ISpecifyPropertyPages)
		QI(ISpecifyPropertyPages2)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

int CMpeg2DecFilter::GetPinCount()
{
	return 4;
}

CBasePin* CMpeg2DecFilter::GetPin(int n)
{
	switch(n) {
		case 0:
			return m_pInput;
		case 1:
			return m_pOutput;
		case 2:
			return m_pSubpicInput;
		case 3:
			return m_pClosedCaptionOutput;
	}
	return NULL;
}

HRESULT CMpeg2DecFilter::EndOfStream()
{
	CAutoLock cAutoLock(&m_csReceive);
	m_pClosedCaptionOutput->EndOfStream();
	return __super::EndOfStream();
}

HRESULT CMpeg2DecFilter::BeginFlush()
{
	m_pClosedCaptionOutput->DeliverBeginFlush();
	return __super::BeginFlush();
}

HRESULT CMpeg2DecFilter::EndFlush()
{
	m_pClosedCaptionOutput->DeliverEndFlush();
	return __super::EndFlush();
}

HRESULT CMpeg2DecFilter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);
	m_pClosedCaptionOutput->DeliverNewSegment(tStart, tStop, dRate);
	m_fDropFrames = false;
	return __super::NewSegment(tStart, tStop, dRate);
}

void CMpeg2DecFilter::InputTypeChanged()
{
	CAutoLock cAutoLock(&m_csReceive);

	TRACE(_T("ResetMpeg2Decoder()\n"));

	for(int i = 0; i < countof(m_dec->m_pictures); i++) {
		m_dec->m_pictures[i].rtStart = m_dec->m_pictures[i].rtStop = _I64_MIN+1;
		m_dec->m_pictures[i].fDelivered = false;
		m_dec->m_pictures[i].flags &= ~PIC_MASK_CODING_TYPE;
	}

	const CMediaType& mt = m_pInput->CurrentMediaType();

	BYTE* pSequenceHeader = NULL;
	DWORD cbSequenceHeader = 0;

	if(mt.formattype == FORMAT_MPEGVideo) {
		pSequenceHeader = ((MPEG1VIDEOINFO*)mt.Format())->bSequenceHeader;
		cbSequenceHeader = ((MPEG1VIDEOINFO*)mt.Format())->cbSequenceHeader;
	} else if(mt.formattype == FORMAT_MPEG2_VIDEO) {
		pSequenceHeader = (BYTE*)((MPEG2VIDEOINFO*)mt.Format())->dwSequenceHeader;
		cbSequenceHeader = ((MPEG2VIDEOINFO*)mt.Format())->cbSequenceHeader;
	}

	m_dec->mpeg2_close();
	m_dec->mpeg2_init();

	m_dec->mpeg2_buffer(pSequenceHeader, pSequenceHeader + cbSequenceHeader);

	m_fWaitForKeyFrame = true;

	m_fFilm = false;
	m_fb.flags = 0;
}

void CMpeg2DecFilter::SetDeinterlaceMethod()
{
	ASSERT(m_dec->m_info.m_sequence);
	ASSERT(m_dec->m_info.m_display_picture);

	DWORD seqflags = m_dec->m_info.m_sequence->flags;
	DWORD oldflags = m_fb.flags;
	DWORD newflags = m_dec->m_info.m_display_picture->flags;

	if(!(seqflags & SEQ_FLAG_PROGRESSIVE_SEQUENCE)
			&& !(oldflags & PIC_FLAG_REPEAT_FIRST_FIELD)
			&& (newflags & PIC_FLAG_PROGRESSIVE_FRAME)) {
		if(!m_fFilm && (newflags & PIC_FLAG_REPEAT_FIRST_FIELD)) {
			TRACE(_T("m_fFilm = true\n"));
			m_fFilm = true;
		} else if(m_fFilm && !(newflags & PIC_FLAG_REPEAT_FIRST_FIELD)) {
			TRACE(_T("m_fFilm = false\n"));
			m_fFilm = false;
		}
	}

	const CMediaType& mt = m_pOutput->CurrentMediaType();

	if(mt.formattype == FORMAT_VideoInfo2 && (((VIDEOINFOHEADER2*)mt.pbFormat)->dwInterlaceFlags & AMINTERLACE_IsInterlaced)) {
		m_fb.di = DIWeave;
	} else {
		m_fb.di = GetDeinterlaceMethod();

		if(m_fb.di == DIAuto || m_fb.di != DIWeave && m_fb.di != DIBlend && m_fb.di != DIBob && m_fb.di != DIFieldShift && m_fb.di != DIELA) {
			if(seqflags & SEQ_FLAG_PROGRESSIVE_SEQUENCE) {
				m_fb.di = DIWeave;    // hurray!
			} else if(m_fFilm) {
				m_fb.di = DIWeave;    // we are lucky
			} else if(!(m_fb.flags & PIC_FLAG_PROGRESSIVE_FRAME)) {
				m_fb.di = DIBlend;    // ok, clear thing
			} else
				// big trouble here, the progressive_frame bit is not reliable :'(
				// frames without temporal field diffs can be only detected when ntsc
				// uses the repeat field flag (signaled with m_fFilm), if it's not set
				// or we have pal then we might end up blending the fields unnecessarily...
			{
				m_fb.di = DIBlend;
			}
		}
	}

	m_fb.flags = newflags;
}

void CMpeg2DecFilter::SetTypeSpecificFlags(IMediaSample* pMS)
{
	if(CComQIPtr<IMediaSample2> pMS2 = pMS) {
		AM_SAMPLE2_PROPERTIES props;
		if(SUCCEEDED(pMS2->GetProperties(sizeof(props), (BYTE*)&props))) {
			props.dwTypeSpecificFlags &= ~0x7f;

			const CMediaType& mt = m_pOutput->CurrentMediaType();
			if(mt.formattype == FORMAT_VideoInfo2 && (((VIDEOINFOHEADER2*)mt.pbFormat)->dwInterlaceFlags & AMINTERLACE_IsInterlaced)) {
				// props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_WEAVE;

				if(m_dec->m_info.m_sequence->flags & SEQ_FLAG_PROGRESSIVE_SEQUENCE) {
					props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_WEAVE;
				}

				if(m_fb.flags & PIC_FLAG_TOP_FIELD_FIRST) {
					props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_FIELD1FIRST;
				}
				if(m_fb.flags & PIC_FLAG_REPEAT_FIRST_FIELD) {
					props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_REPEAT_FIELD;
				}

				if((m_fb.flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I) {
					props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_I_SAMPLE;
				}
				if((m_fb.flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_P) {
					props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_P_SAMPLE;
				}
				if((m_fb.flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_B) {
					props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_B_SAMPLE;
				}
			}

			pMS2->SetProperties(sizeof(props), (BYTE*)&props);
		}
	}
}

HRESULT CMpeg2DecFilter::Transform(IMediaSample* pIn)
{
	HRESULT hr;

	BYTE* pDataIn = NULL;
	if(FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}

	long len = pIn->GetActualDataLength();

	(static_cast<CDeCSSInputPin*>(m_pInput))->StripPacket(pDataIn, len);

	if(pIn->IsDiscontinuity() == S_OK) {
		InputTypeChanged();
	}

	REFERENCE_TIME rtStart = _I64_MIN, rtStop = _I64_MIN;
	hr = pIn->GetTime(&rtStart, &rtStop);
	if(FAILED(hr)) {
		rtStart = rtStop = _I64_MIN;
	}

	while(len >= 0) {
		mpeg2_state_t state = m_dec->mpeg2_parse();
#ifndef _WIN64
		__asm emms; // this one is missing somewhere in the precompiled mmx obj files
#endif
		switch(state) {
			case STATE_BUFFER:
				if(len > 0) {
					m_dec->mpeg2_buffer(pDataIn, pDataIn + len);
					len = 0;
				} else {
					len = -1;
				}
				break;
			case STATE_INVALID:
				TRACE(_T("*** STATE_INVALID\n"));
				break;
			case STATE_GOP:
				m_pClosedCaptionOutput->Deliver(m_dec->m_info.m_user_data, m_dec->m_info.m_user_data_len);
				break;
			case STATE_SEQUENCE:
				m_AvgTimePerFrame = m_dec->m_info.m_sequence->frame_period
									? 10i64 * m_dec->m_info.m_sequence->frame_period / 27
									: ((VIDEOINFOHEADER*)m_pInput->CurrentMediaType().Format())->AvgTimePerFrame;
				break;
			case STATE_PICTURE:
				m_dec->m_picture->rtStart = rtStart;
				rtStart = _I64_MIN;
				m_dec->m_picture->fDelivered = false;
				m_dec->mpeg2_skip(m_fDropFrames && (m_dec->m_picture->flags&PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_B);
				break;
			case STATE_SLICE:
			case STATE_END: {
				mpeg2_picture_t* picture = m_dec->m_info.m_display_picture;
				mpeg2_fbuf_t* fbuf = m_dec->m_info.m_display_fbuf;

				if(picture && !(picture->flags&PIC_FLAG_SKIP) && fbuf) {
					ASSERT(!picture->fDelivered);

					picture->fDelivered = true;

					// frame buffer

					int w = m_dec->m_info.m_sequence->picture_width;
					int h = m_dec->m_info.m_sequence->picture_height;
					int pitch = (m_dec->m_info.m_sequence->width + 31) & ~31;

					if(m_fb.w != w || m_fb.h != h || m_fb.pitch != pitch) {
						m_fb.Alloc(w, h, pitch);
						m_fInitializedBuffer = false;
					}

					// start - end

					m_fb.rtStart = picture->rtStart;
					if(m_fb.rtStart == _I64_MIN) {
						m_fb.rtStart = m_fb.rtStop;
					}
					m_fb.rtStop = m_fb.rtStart + m_AvgTimePerFrame * picture->nb_fields / (m_dec->m_info.m_display_picture_2nd ? 1 : 2);

					REFERENCE_TIME rtStart = m_fb.rtStart;
					REFERENCE_TIME rtStop = m_fb.rtStop;
					UNUSED_ALWAYS(rtStart);
					UNUSED_ALWAYS(rtStop);

					//

					SetDeinterlaceMethod();
					UpdateAspectRatio();

					hr = DeliverFast();
					if(hr != S_OK) {
						hr = DeliverNormal();
					}
					if(hr != S_OK) {
						return hr;
					}
					if(hr == S_OK && !m_fWaitForKeyFrame) {
						m_fInitializedBuffer = true;
					}
				}
			}
			break;
			default:
				break;
		}
	}

	return S_OK;
}

bool CMpeg2DecFilter::IsVideoInterlaced()
{
	return IsInterlacedEnabled();
}

void CMpeg2DecFilter::UpdateAspectRatio()
{
	if (m_bReadARFromStream && ((unsigned int)m_par.cx != m_dec->m_info.m_sequence->pixel_width || (unsigned int)m_par.cy != m_dec->m_info.m_sequence->pixel_height)) {
		m_par.cx = m_dec->m_info.m_sequence->pixel_width;
		m_par.cy = m_dec->m_info.m_sequence->pixel_height;
		CSize dar(m_dec->m_info.m_sequence->picture_width * m_par.cx,
				  m_dec->m_info.m_sequence->picture_height * m_par.cy);
		int lnko = LNKO(dar.cx, dar.cy);
		if(lnko > 1) {
			dar.cx /= lnko, dar.cy /= lnko;
		}
		SetAspect(dar);
	}
}

HRESULT CMpeg2DecFilter::DeliverFast()
{
	HRESULT hr;

	CAutoLock cAutoLock(&m_csReceive);

	mpeg2_fbuf_t* fbuf = m_dec->m_info.m_display_fbuf;
	if(!fbuf) {
		return S_FALSE;
	}

	{

		CAutoLock cAutoLock2(&m_csProps);

		if(GetCLSID(m_pInput->GetConnected()) == CLSID_DVDNavigator
				|| m_pSubpicInput->HasAnythingToRender(m_fb.rtStart)
				|| fabs(m_bright) > EPSILON || fabs(m_cont-1.0) > EPSILON
				|| fabs(m_hue) > EPSILON || fabs(m_sat-1.0) > EPSILON) {
			return S_FALSE;
		}
	}

	if((m_fb.flags & PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I) {
		m_fWaitForKeyFrame = false;
	}

	if(m_fb.rtStart < 0 || m_fWaitForKeyFrame) {
		return S_OK;
	}

	const CMediaType& mt = m_pOutput->CurrentMediaType();

	if(mt.subtype != MEDIASUBTYPE_I420 && mt.subtype != MEDIASUBTYPE_IYUV && mt.subtype != MEDIASUBTYPE_YV12) {
		return S_FALSE;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if(FAILED(hr = GetDeliveryBuffer(m_fb.w, m_fb.h, &pOut))
			|| FAILED(hr = pOut->GetPointer(&pDataOut))) {
		return hr;
	}

	if(mt.subtype != MEDIASUBTYPE_I420 && mt.subtype != MEDIASUBTYPE_IYUV && mt.subtype != MEDIASUBTYPE_YV12) {
		return S_FALSE;
	}

	BITMAPINFOHEADER bihOut;
	ExtractBIH(&mt, &bihOut);

	int w = bihOut.biWidth;
	int h = abs(bihOut.biHeight);
	int srcpitch = m_dec->m_info.m_sequence->width; // TODO (..+7)&~7; ?
	int dstpitch = bihOut.biWidth;

	BYTE* y = pDataOut;
	BYTE* u = y + dstpitch*h;
	BYTE* v = y + dstpitch*h*5/4;

	if(bihOut.biCompression == '21VY') {
		BYTE* tmp = u;
		u = v;
		v = tmp;
	}

	if(m_fb.di == DIWeave) {
		BitBltFromI420ToI420(w, h, y, u, v, dstpitch, fbuf->buf[0], fbuf->buf[1], fbuf->buf[2], srcpitch);
	} else if(m_fb.di == DIBlend) {
		DeinterlaceBlend(y, fbuf->buf[0], w, h, dstpitch, srcpitch);
		DeinterlaceBlend(u, fbuf->buf[1], w/2, h/2, dstpitch/2, srcpitch/2);
		DeinterlaceBlend(v, fbuf->buf[2], w/2, h/2, dstpitch/2, srcpitch/2);
	} else { // TODO
		return S_FALSE;
	}

	if(h == 1088) {
		memset(y + dstpitch*(h-8), 0xff, dstpitch*8);
		memset(u + dstpitch*(h-8)/4, 0x80, dstpitch*8/4);
		memset(v + dstpitch*(h-8)/4, 0x80, dstpitch*8/4);
	}

	if(CMpeg2DecInputPin* pPin = dynamic_cast<CMpeg2DecInputPin*>(m_pInput)) {
		CAutoLock cAutoLock(&pPin->m_csRateLock);

		if(m_rate.Rate != pPin->m_ratechange.Rate) {
			m_rate.Rate = pPin->m_ratechange.Rate;
			m_rate.StartTime = m_fb.rtStart;
		}
	}

	REFERENCE_TIME rtStart = m_fb.rtStart;
	REFERENCE_TIME rtStop = m_fb.rtStop;

	rtStart = m_rate.StartTime + (rtStart - m_rate.StartTime) * m_rate.Rate / 10000;
	rtStop = m_rate.StartTime + (rtStop - m_rate.StartTime) * m_rate.Rate / 10000;

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	//

	SetTypeSpecificFlags(pOut);

	//

	return m_pOutput->Deliver(pOut);
}

HRESULT CMpeg2DecFilter::DeliverNormal()
{
	HRESULT hr = S_OK;

	CAutoLock cAutoLock(&m_csReceive);

	mpeg2_fbuf_t* fbuf = m_dec->m_info.m_display_fbuf;
	if(!fbuf) {
		return S_FALSE;
	}

	int w = m_fb.w;
	int h = m_fb.h;
	int spitch = m_dec->m_info.m_sequence->width; // TODO (..+7)&~7; ?
	int dpitch = m_fb.pitch;

	REFERENCE_TIME rtStart = m_fb.rtStart;
	REFERENCE_TIME rtStop = m_fb.rtStop;

	bool tff = !!(m_fb.flags&PIC_FLAG_TOP_FIELD_FIRST);

	// deinterlace

	if(m_fb.di == DIWeave) {
		BitBltFromI420ToI420(w, h, m_fb.buf[0], m_fb.buf[1], m_fb.buf[2], dpitch, fbuf->buf[0], fbuf->buf[1], fbuf->buf[2], spitch);
	} else if(m_fb.di == DIBlend) {
		DeinterlaceBlend(m_fb.buf[0], fbuf->buf[0], w, h, dpitch, spitch);
		DeinterlaceBlend(m_fb.buf[1], fbuf->buf[1], w/2, h/2, dpitch/2, spitch/2);
		DeinterlaceBlend(m_fb.buf[2], fbuf->buf[2], w/2, h/2, dpitch/2, spitch/2);
	} else if(m_fb.di == DIBob) {
		DeinterlaceBob(m_fb.buf[0], fbuf->buf[0], w, h, dpitch, spitch, tff);
		DeinterlaceBob(m_fb.buf[1], fbuf->buf[1], w/2, h/2, dpitch/2, spitch/2, tff);
		DeinterlaceBob(m_fb.buf[2], fbuf->buf[2], w/2, h/2, dpitch/2, spitch/2, tff);

		m_fb.rtStart = rtStart;
		m_fb.rtStop = (rtStart + rtStop) / 2;
	} else if(m_fb.di == DIFieldShift) {
		int soffset = tff ? 0 : spitch;
		int doffset = tff ? 0 : dpitch;
		BitBltFromRGBToRGB(w, h/2, m_fb.buf[0] + doffset, dpitch*2, 8, fbuf->buf[0] + soffset, spitch*2, 8);
		BitBltFromRGBToRGB(w/2, h/4, m_fb.buf[1] + doffset/2, dpitch, 8, fbuf->buf[1] + soffset/2, spitch, 8);
		BitBltFromRGBToRGB(w/2, h/4, m_fb.buf[2] + doffset/2, dpitch, 8, fbuf->buf[2] + soffset/2, spitch, 8);
	} else if(m_fb.di == DIELA) {
		DeinterlaceELA(m_fb.buf[0], fbuf->buf[0], w, h, dpitch, spitch, tff);
		DeinterlaceELA(m_fb.buf[1], fbuf->buf[1], w/2, h/2, dpitch/2, spitch/2, tff);
		DeinterlaceELA(m_fb.buf[2], fbuf->buf[2], w/2, h/2, dpitch/2, spitch/2, tff);
	}

	// postproc

	ApplyBrContHueSat(m_fb.buf[0], m_fb.buf[1], m_fb.buf[2], w, h, dpitch);

	// deliver

	if (m_fb.di == DIWeave || m_fInitializedBuffer) {
		hr = Deliver(false);
		if(FAILED(hr)) {
			return hr;
		}
	}

	if(m_fb.di == DIBob) {
		DeinterlaceBob(m_fb.buf[0], fbuf->buf[0], w, h, dpitch, spitch, !tff);
		DeinterlaceBob(m_fb.buf[1], fbuf->buf[1], w/2, h/2, dpitch/2, spitch/2, !tff);
		DeinterlaceBob(m_fb.buf[2], fbuf->buf[2], w/2, h/2, dpitch/2, spitch/2, !tff);

		m_fb.rtStart = (rtStart + rtStop) / 2;
		m_fb.rtStop = rtStop;

		// postproc

		ApplyBrContHueSat(m_fb.buf[0], m_fb.buf[1], m_fb.buf[2], w, h, dpitch);

		// deliver

		hr = Deliver(false);
	} else if(m_fb.di == DIFieldShift) {
		int soffset = !tff ? 0 : spitch;
		int doffset = !tff ? 0 : dpitch;
		BitBltFromRGBToRGB(w, h/2, m_fb.buf[0] + doffset, dpitch*2, 8, fbuf->buf[0] + soffset, spitch*2, 8);
		BitBltFromRGBToRGB(w/2, h/4, m_fb.buf[1] + doffset/2, dpitch, 8, fbuf->buf[1] + soffset/2, spitch, 8);
		BitBltFromRGBToRGB(w/2, h/4, m_fb.buf[2] + doffset/2, dpitch, 8, fbuf->buf[2] + soffset/2, spitch, 8);
	} else if(m_fb.di == DIELA) {
		DeinterlaceELA(m_fb.buf[0], fbuf->buf[0], w, h, dpitch, spitch, !tff);
		DeinterlaceELA(m_fb.buf[1], fbuf->buf[1], w/2, h/2, dpitch/2, spitch/2, !tff);
		DeinterlaceELA(m_fb.buf[2], fbuf->buf[2], w/2, h/2, dpitch/2, spitch/2, !tff);
	}

	if (!m_fInitializedBuffer) {
		hr = Deliver(false);
	}

	return hr;
}

HRESULT CMpeg2DecFilter::Deliver(bool fRepeatLast)
{
	CAutoLock cAutoLock(&m_csReceive);

	if((m_fb.flags&PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I) {
		m_fWaitForKeyFrame = false;
	}

	if(m_fb.rtStart < 0 || m_fWaitForKeyFrame) {
		return S_OK;
	}

	HRESULT hr;

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if(FAILED(hr = GetDeliveryBuffer(m_fb.w, m_fb.h, &pOut))
			|| FAILED(hr = pOut->GetPointer(&pDataOut))) {
		return hr;
	}

	if(m_fb.h == 1088) {
		memset(m_fb.buf[0] + m_fb.w*(m_fb.h-8), 0xff, m_fb.pitch*8);
		memset(m_fb.buf[1] + m_fb.w*(m_fb.h-8)/4, 0x80, m_fb.pitch*8/4);
		memset(m_fb.buf[2] + m_fb.w*(m_fb.h-8)/4, 0x80, m_fb.pitch*8/4);
	}

	BYTE** buf = &m_fb.buf[0];

	if(m_pSubpicInput->HasAnythingToRender(m_fb.rtStart)) {
		BitBltFromI420ToI420(m_fb.w, m_fb.h,
							 m_fb.buf[3], m_fb.buf[4], m_fb.buf[5], m_fb.pitch,
							 m_fb.buf[0], m_fb.buf[1], m_fb.buf[2], m_fb.pitch);

		buf = &m_fb.buf[3];

		m_pSubpicInput->RenderSubpics(m_fb.rtStart, buf, m_fb.pitch, m_fb.h);
	}

	CopyBuffer(pDataOut, buf, (m_fb.w+7)&~7, m_fb.h, m_fb.pitch, MEDIASUBTYPE_I420, !(m_fb.flags & PIC_FLAG_PROGRESSIVE_FRAME));

	//

	if(CMpeg2DecInputPin* pPin = dynamic_cast<CMpeg2DecInputPin*>(m_pInput)) {
		CAutoLock cAutoLock(&pPin->m_csRateLock);

		if(m_rate.Rate != pPin->m_ratechange.Rate) {
			m_rate.Rate = pPin->m_ratechange.Rate;
			m_rate.StartTime = m_fb.rtStart;
		}
	}

	REFERENCE_TIME rtStart = m_fb.rtStart;
	REFERENCE_TIME rtStop = m_fb.rtStop;

	rtStart = m_rate.StartTime + (rtStart - m_rate.StartTime) * m_rate.Rate / 10000;
	rtStop = m_rate.StartTime + (rtStop - m_rate.StartTime) * m_rate.Rate / 10000;

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	//

	SetTypeSpecificFlags(pOut);

	//

	hr = m_pOutput->Deliver(pOut);

	return hr;
}

HRESULT CMpeg2DecFilter::CheckConnect(PIN_DIRECTION dir, IPin* pPin)
{
	if(dir == PINDIR_OUTPUT) {
		if(GetCLSID(m_pInput->GetConnected()) == CLSID_DVDNavigator) {
			// one of these needed for dynamic format changes

			CLSID clsid = GetCLSID(pPin);

			DWORD ver = 0;
			if(CComQIPtr<IFilterVersion> pFV = GetFilterFromPin(pPin)) {
				ver = pFV->GetFilterVersion();
			}

			if(clsid != CLSID_OverlayMixer
					/*&& clsid != CLSID_OverlayMixer2*/
					&& clsid != CLSID_VideoMixingRenderer
					&& clsid != CLSID_VideoMixingRenderer9
					&& clsid != GUIDFromCString(_T("{FA10746C-9B63-4b6c-BC49-FC300EA5F256}")) // EVR
					&& clsid != GUIDFromCString(_T("{04FE9017-F873-410E-871E-AB91661A4EF7}")) // ffdshow
					&& (clsid != GUIDFromCString(_T("{93A22E7A-5091-45ef-BA61-6DA26156A5D0}")) || ver < 0x0234) // dvobsub
					&& (clsid != GUIDFromCString(_T("{9852A670-F845-491b-9BE6-EBD841B8A613}")) || ver < 0x0234) // dvobsub auto
					&& clsid != CLSID_madVR
					&& clsid != CLSID_DXR) { // Haali's video renderer
				return E_FAIL;
			}
		}
	}

	return __super::CheckConnect(dir, pPin);
}

int NextMpegStartCode(CGolombBuffer& gb, BYTE& code, __int64 len)
{
	gb.BitByteAlign();
	DWORD dw = (DWORD)-1;
	do {
		if(len-- == 0) {
			return false;
		}
		dw = (dw << 8) | (BYTE)gb.BitRead(8);
	} while((dw&0xffffff00) != 0x00000100);
	code = (BYTE)(dw&0xff);
	return true;
}

HRESULT CMpeg2DecFilter::CheckInputType(const CMediaType* mtIn)
{
	if(mtIn->formattype == FORMAT_MPEG2_VIDEO && mtIn->pbFormat) {
		MPEG2VIDEOINFO* vih = (MPEG2VIDEOINFO*)mtIn->pbFormat;
		if(vih->cbSequenceHeader > 0 && (vih->dwSequenceHeader[0] & 0x00ffffff) != 0x00010000) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
		BYTE* pSequenceHeader = (BYTE*)vih->dwSequenceHeader;
		DWORD cbSequenceHeader = vih->cbSequenceHeader;

		BYTE id = 0;
		CGolombBuffer gb(pSequenceHeader, cbSequenceHeader);
		while(gb.GetPos() < gb.GetSize() && id != 0xb5) {
			if(!NextMpegStartCode(gb, id, cbSequenceHeader)) {
				break;
			}
		}
		if(id == 0xb5) {
			gb.BitRead(4); // startcodeid
			gb.BitRead(1); // profile_levelescape
			gb.BitRead(3); // profile
			gb.BitRead(4); // level
			gb.BitRead(1); // interlaced
			BYTE chroma = gb.BitRead(2);
			if(chroma >= 2) {// current support only 4:2:0 profile
				return VFW_E_TYPE_NOT_ACCEPTED;	
			}
		}
	}

	return (mtIn->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && mtIn->subtype == MEDIASUBTYPE_MPEG2_VIDEO
			|| mtIn->majortype == MEDIATYPE_MPEG2_PACK && mtIn->subtype == MEDIASUBTYPE_MPEG2_VIDEO
			|| mtIn->majortype == MEDIATYPE_MPEG2_PES && mtIn->subtype == MEDIASUBTYPE_MPEG2_VIDEO
			|| mtIn->majortype == MEDIATYPE_Video && mtIn->subtype == MEDIASUBTYPE_MPEG2_VIDEO
			|| mtIn->majortype == MEDIATYPE_Video && mtIn->subtype == MEDIASUBTYPE_MPEG1Packet
			|| mtIn->majortype == MEDIATYPE_Video && mtIn->subtype == MEDIASUBTYPE_MPEG1Payload)
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMpeg2DecFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	bool fPlanarYUV = mtOut->subtype == MEDIASUBTYPE_YV12
					  || mtOut->subtype == MEDIASUBTYPE_I420
					  || mtOut->subtype == MEDIASUBTYPE_IYUV;

	return SUCCEEDED(__super::CheckTransform(mtIn, mtOut))
		   && (!fPlanarYUV || IsPlanarYUVEnabled())
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

DWORD g_clock;

HRESULT CMpeg2DecFilter::StartStreaming()
{
	HRESULT hr = __super::StartStreaming();
	if(FAILED(hr)) {
		return hr;
	}

	m_dec.Attach(DNew CMpeg2Dec());
	if(!m_dec) {
		return E_OUTOFMEMORY;
	}

	InputTypeChanged();

	//	g_clock = clock();

	return S_OK;
}

HRESULT CMpeg2DecFilter::StopStreaming()
{
	/*
		CString str;
		str.Format(_T("%d"), clock()-g_clock);
		AfxMessageBox(str);
	*/
	m_dec.Free();

	return __super::StopStreaming();
}

HRESULT CMpeg2DecFilter::AlterQuality(Quality q)
{
	if(q.Late > 100*10000i64) {
		m_fDropFrames = true;
	} else if(q.Late <= 0) {
		m_fDropFrames = false;
	}

	//	TRACE(_T("CMpeg2DecFilter::AlterQuality: Type=%d, Proportion=%d, Late=%I64d, TimeStamp=%I64d\n"), q.Type, q.Proportion, q.Late, q.TimeStamp);
	return S_OK;
}

// ISpecifyPropertyPages2

STDMETHODIMP CMpeg2DecFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	pPages->pElems[0] = __uuidof(CMpeg2DecSettingsWnd);

	return S_OK;
}

STDMETHODIMP CMpeg2DecFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
	CheckPointer(ppPage, E_POINTER);

	if(*ppPage != NULL) {
		return E_INVALIDARG;
	}

	HRESULT hr;

	if(guid == __uuidof(CMpeg2DecSettingsWnd)) {
		(*ppPage = DNew CInternalPropertyPageTempl<CMpeg2DecSettingsWnd>(NULL, &hr))->AddRef();
	}

	return *ppPage ? S_OK : E_FAIL;
}

// IMpeg2DecFilter

STDMETHODIMP CMpeg2DecFilter::SetDeinterlaceMethod(ditype di)
{
	CAutoLock cAutoLock(&m_csProps);
	m_ditype = di;
	return S_OK;
}

STDMETHODIMP_(ditype) CMpeg2DecFilter::GetDeinterlaceMethod()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_ditype;
}

void CMpeg2DecFilter::CalcBrCont(BYTE* YTbl, float bright, float cont)
{
	int Cont = (int)(cont * 512);
	int Bright = (int)bright;

	for(int i = 0; i < 256; i++) {
		int y = ((Cont * (i - 16)) >> 9) + Bright + 16;
		YTbl[i] = min(max(y, 0), 255);
		//		YTbl[i] = min(max(y, 16), 235);
	}
}

void CMpeg2DecFilter::CalcHueSat(BYTE* UTbl, BYTE* VTbl, float hue, float sat)
{
	int Sat = (int)(sat * 512);
	double Hue = (hue * 3.1415926) / 180.0;
	int Sin = (int)(sin(Hue) * 4096);
	int Cos = (int)(cos(Hue) * 4096);

	for(int y = 0; y < 256; y++) {
		for(int x = 0; x < 256; x++) {
			int u = x - 128;
			int v = y - 128;
			int ux = (u * Cos + v * Sin) >> 12;
			v = (v * Cos - u * Sin) >> 12;
			u = ((ux * Sat) >> 9) + 128;
			v = ((v * Sat) >> 9) + 128;
			u = min(max(u, 16), 235);
			v = min(max(v, 16), 235);
			UTbl[(y << 8) | x] = u;
			VTbl[(y << 8) | x] = v;
		}
	}
}

void CMpeg2DecFilter::ApplyBrContHueSat(BYTE* srcy, BYTE* srcu, BYTE* srcv, int w, int h, int pitch)
{
	CAutoLock cAutoLock(&m_csProps);

	if(fabs(m_bright) > EPSILON || fabs(m_cont-1.0) > EPSILON) {
		int size = pitch*h;

		if((g_cpuid.m_flags&CCpuID::sse2) && ((DWORD_PTR)srcy & 15) == 0) {
			short Cont = (short)(min(max(m_cont, 0) * 512, (1<<16)-1));
			short Bright = (short)(m_bright + 16);

			__m128i bc = _mm_set_epi16(Bright, Cont, Bright, Cont, Bright, Cont, Bright, Cont);

			__m128i zero = _mm_setzero_si128();
			__m128i _16 = _mm_set1_epi16(16);
			__m128i _512 = _mm_set1_epi16(512);

			for(int i = 0, j = size>>4; i < j; i++) {
				__m128i r = _mm_load_si128((__m128i*)&srcy[i*16]);

				__m128i rl = _mm_unpacklo_epi8(r, zero);
				__m128i rh = _mm_unpackhi_epi8(r, zero);

				rl = _mm_subs_epi16(rl, _16);
				rh = _mm_subs_epi16(rh, _16);

				__m128i rll = _mm_unpacklo_epi16(rl, _512);
				__m128i rlh = _mm_unpackhi_epi16(rl, _512);
				__m128i rhl = _mm_unpacklo_epi16(rh, _512);
				__m128i rhh = _mm_unpackhi_epi16(rh, _512);

				rll = _mm_madd_epi16(rll, bc);
				rlh = _mm_madd_epi16(rlh, bc);
				rhl = _mm_madd_epi16(rhl, bc);
				rhh = _mm_madd_epi16(rhh, bc);

				rll = _mm_srai_epi32(rll, 9);
				rlh = _mm_srai_epi32(rlh, 9);
				rhl = _mm_srai_epi32(rhl, 9);
				rhh = _mm_srai_epi32(rhh, 9);

				rl = _mm_packs_epi32(rll, rlh);
				rh = _mm_packs_epi32(rhl, rhh);

				r = _mm_packus_epi16(rl, rh);

				_mm_store_si128((__m128i*)&srcy[i*16], r);
			}

			srcy += size>>4;
			size &= 15;
		}

		for(; size > 0; size--) {
			*srcy++ = m_YTbl[*srcy];
		}
	}

	pitch /= 2;
	w /= 2;
	h /= 2;

	if(fabs(m_hue) > EPSILON || fabs(m_sat-1.0) > EPSILON) {
		for(int size = pitch*h; size > 0; size--) {
			WORD uv = (*srcv<<8)|*srcu;
			*srcu++ = m_UTbl[uv];
			*srcv++ = m_VTbl[uv];
		}
	}
}

STDMETHODIMP CMpeg2DecFilter::SetBrightness(float bright)
{
	CAutoLock cAutoLock(&m_csProps);
	CalcBrCont(m_YTbl, m_bright = bright, m_cont);
	return S_OK;
}

STDMETHODIMP CMpeg2DecFilter::SetContrast(float cont)
{
	CAutoLock cAutoLock(&m_csProps);
	CalcBrCont(m_YTbl, m_bright, m_cont = cont);
	return S_OK;
}

STDMETHODIMP CMpeg2DecFilter::SetHue(float hue)
{
	CAutoLock cAutoLock(&m_csProps);
	CalcHueSat(m_UTbl, m_VTbl, m_hue = hue, m_sat);
	return S_OK;
}

STDMETHODIMP CMpeg2DecFilter::SetSaturation(float sat)
{
	CAutoLock cAutoLock(&m_csProps);
	CalcHueSat(m_UTbl, m_VTbl, m_hue, m_sat = sat);
	return S_OK;
}

STDMETHODIMP_(float) CMpeg2DecFilter::GetBrightness()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_bright;
}

STDMETHODIMP_(float) CMpeg2DecFilter::GetContrast()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_cont;
}

STDMETHODIMP_(float) CMpeg2DecFilter::GetHue()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_hue;
}

STDMETHODIMP_(float) CMpeg2DecFilter::GetSaturation()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_sat;
}

STDMETHODIMP CMpeg2DecFilter::EnableForcedSubtitles(bool fEnable)
{
	CAutoLock cAutoLock(&m_csProps);
	m_fForcedSubs = fEnable;
	return S_OK;
}

STDMETHODIMP_(bool) CMpeg2DecFilter::IsForcedSubtitlesEnabled()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_fForcedSubs;
}

STDMETHODIMP CMpeg2DecFilter::EnablePlanarYUV(bool fEnable)
{
	CAutoLock cAutoLock(&m_csProps);
	m_fPlanarYUV = fEnable;
	return S_OK;
}

STDMETHODIMP_(bool) CMpeg2DecFilter::IsPlanarYUVEnabled()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_fPlanarYUV;
}

STDMETHODIMP CMpeg2DecFilter::EnableInterlaced(bool fEnable)
{
	CAutoLock cAutoLock(&m_csProps);
	m_fInterlaced = fEnable;
	return S_OK;
}

STDMETHODIMP_(bool) CMpeg2DecFilter::IsInterlacedEnabled()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_fInterlaced;
}

STDMETHODIMP CMpeg2DecFilter::EnableReadARFromStream(bool fEnable)
{
	CAutoLock cAutoLock(&m_csProps);
	m_bReadARFromStream = fEnable;
	return S_OK;
}

STDMETHODIMP_(bool) CMpeg2DecFilter::IsReadARFromStreamEnabled()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_bReadARFromStream;
}

//
// CMpeg2DecInputPin
//

CMpeg2DecInputPin::CMpeg2DecInputPin(CTransformFilter* pFilter, HRESULT* phr, LPWSTR pName)
	: CDeCSSInputPin(NAME("CMpeg2DecInputPin"), pFilter, phr, pName)
{
	m_CorrectTS = 0;
	m_ratechange.Rate = 10000;
	m_ratechange.StartTime = _I64_MAX;
}

// IKsPropertySet

STDMETHODIMP CMpeg2DecInputPin::Set(REFGUID PropSet, ULONG Id, LPVOID pInstanceData, ULONG InstanceLength, LPVOID pPropertyData, ULONG DataLength)
{
	if(PropSet != AM_KSPROPSETID_TSRateChange /*&& PropSet != AM_KSPROPSETID_DVD_RateChange*/) {
		return __super::Set(PropSet, Id, pInstanceData, InstanceLength, pPropertyData, DataLength);
	}

	if(PropSet == AM_KSPROPSETID_TSRateChange)
		switch(Id) {
			case AM_RATE_SimpleRateChange: {
				AM_SimpleRateChange* p = (AM_SimpleRateChange*)pPropertyData;
				if(!m_CorrectTS) {
					return E_PROP_ID_UNSUPPORTED;
				}
				CAutoLock cAutoLock(&m_csRateLock);
				m_ratechange = *p;
				DbgLog((LOG_TRACE, 0, _T("StartTime=%I64d, Rate=%d"), p->StartTime, p->Rate));
			}
			break;
			case AM_RATE_UseRateVersion: {
				WORD* p = (WORD*)pPropertyData;
				if(*p > 0x0101) {
					return E_PROP_ID_UNSUPPORTED;
				}
			}
			break;
			case AM_RATE_CorrectTS: {
				LONG* p = (LONG*)pPropertyData;
				m_CorrectTS = *p;
			}
			break;
			default:
				return E_PROP_ID_UNSUPPORTED;
		}
	/*
		if(PropSet == AM_KSPROPSETID_DVD_RateChange)
		switch(Id)
		{
		case AM_RATE_ChangeRate:
			{
				AM_DVD_ChangeRate* p = (AM_DVD_ChangeRate*)pPropertyData;
			}
			break;
		default:
			return E_PROP_ID_UNSUPPORTED;
		}
	*/
	return S_OK;
}

STDMETHODIMP CMpeg2DecInputPin::Get(REFGUID PropSet, ULONG Id, LPVOID pInstanceData, ULONG InstanceLength, LPVOID pPropertyData, ULONG DataLength, ULONG* pBytesReturned)
{
	if(PropSet != AM_KSPROPSETID_TSRateChange /*&& PropSet != AM_KSPROPSETID_DVD_RateChange*/) {
		return __super::Get(PropSet, Id, pInstanceData, InstanceLength, pPropertyData, DataLength, pBytesReturned);
	}

	if(PropSet == AM_KSPROPSETID_TSRateChange)
		switch(Id) {
			case AM_RATE_SimpleRateChange: {
				AM_SimpleRateChange* p = (AM_SimpleRateChange*)pPropertyData;
				UNUSED_ALWAYS(p);
				return E_PROP_ID_UNSUPPORTED;
			}
			break;
			case AM_RATE_MaxFullDataRate: {
				AM_MaxFullDataRate* p = (AM_MaxFullDataRate*)pPropertyData;
				UNUSED_ALWAYS(p);
				*p = 8*10000;
				*pBytesReturned = sizeof(AM_MaxFullDataRate);
			}
			break;
			case AM_RATE_QueryFullFrameRate: {
				AM_QueryRate* p = (AM_QueryRate*)pPropertyData;
				p->lMaxForwardFullFrame = 8*10000;
				p->lMaxReverseFullFrame = 8*10000;
				*pBytesReturned = sizeof(AM_QueryRate);
			}
			break;
			case AM_RATE_QueryLastRateSegPTS: {
				//REFERENCE_TIME* p = (REFERENCE_TIME*)pPropertyData;
				return E_PROP_ID_UNSUPPORTED;
			}
			break;
			default:
				return E_PROP_ID_UNSUPPORTED;
		}
	/*
		if(PropSet == AM_KSPROPSETID_DVD_RateChange)
		switch(Id)
		{
		case AM_RATE_FullDataRateMax:
			{
				AM_MaxFullDataRate* p = (AM_MaxFullDataRate*)pPropertyData;
			}
			break;
		case AM_RATE_ReverseDecode:
			{
				LONG* p = (LONG*)pPropertyData;
			}
			break;
		case AM_RATE_DecoderPosition:
			{
				AM_DVD_DecoderPosition* p = (AM_DVD_DecoderPosition*)pPropertyData;
			}
			break;
		case AM_RATE_DecoderVersion:
			{
				LONG* p = (LONG*)pPropertyData;
			}
			break;
		default:
			return E_PROP_ID_UNSUPPORTED;
		}
	*/
	return S_OK;
}

STDMETHODIMP CMpeg2DecInputPin::QuerySupported(REFGUID PropSet, ULONG Id, ULONG* pTypeSupport)
{
	if(PropSet != AM_KSPROPSETID_TSRateChange /*&& PropSet != AM_KSPROPSETID_DVD_RateChange*/) {
		return __super::QuerySupported(PropSet, Id, pTypeSupport);
	}

	if(PropSet == AM_KSPROPSETID_TSRateChange)
		switch(Id) {
			case AM_RATE_SimpleRateChange:
				*pTypeSupport = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
				break;
			case AM_RATE_MaxFullDataRate:
				*pTypeSupport = KSPROPERTY_SUPPORT_GET;
				break;
			case AM_RATE_UseRateVersion:
				*pTypeSupport = KSPROPERTY_SUPPORT_SET;
				break;
			case AM_RATE_QueryFullFrameRate:
				*pTypeSupport = KSPROPERTY_SUPPORT_GET;
				break;
			case AM_RATE_QueryLastRateSegPTS:
				*pTypeSupport = KSPROPERTY_SUPPORT_GET;
				break;
			case AM_RATE_CorrectTS:
				*pTypeSupport = KSPROPERTY_SUPPORT_SET;
				break;
			default:
				return E_PROP_ID_UNSUPPORTED;
		}
	/*
		if(PropSet == AM_KSPROPSETID_DVD_RateChange)
		switch(Id)
		{
		case AM_RATE_ChangeRate:
			*pTypeSupport = KSPROPERTY_SUPPORT_SET;
			break;
		case AM_RATE_FullDataRateMax:
			*pTypeSupport = KSPROPERTY_SUPPORT_GET;
			break;
		case AM_RATE_ReverseDecode:
			*pTypeSupport = KSPROPERTY_SUPPORT_GET;
			break;
		case AM_RATE_DecoderPosition:
			*pTypeSupport = KSPROPERTY_SUPPORT_GET;
			break;
		case AM_RATE_DecoderVersion:
			*pTypeSupport = KSPROPERTY_SUPPORT_GET;
			break;
		default:
			return E_PROP_ID_UNSUPPORTED;
		}
	*/
	return S_OK;
}

//
// CMpeg2DecOutputPin
//

CMpeg2DecOutputPin::CMpeg2DecOutputPin(CBaseVideoFilter* pFilter, HRESULT* phr, LPWSTR pName)
	: CBaseVideoOutputPin(NAME("CMpeg2DecOutputPin"), pFilter, phr, pName)
{
}

HRESULT CMpeg2DecOutputPin::Active()
{
	CAutoLock cAutoLock(m_pLock);

	// TODO

	if(m_Connected && !m_pOutputQueue) {
		HRESULT hr = NOERROR;

		m_pOutputQueue.Attach(DNew COutputQueue(m_Connected, &hr));
		if(!m_pOutputQueue) {
			hr = E_OUTOFMEMORY;
		}

		if(FAILED(hr)) {
			m_pOutputQueue.Free();
			return hr;
		}
	}

	return __super::Active();
}

HRESULT CMpeg2DecOutputPin::Inactive()
{
	CAutoLock cAutoLock(m_pLock);
	m_pOutputQueue.Free();
	return __super::Inactive();
}

HRESULT CMpeg2DecOutputPin::Deliver(IMediaSample* pMediaSample)
{
	if(!m_pOutputQueue) {
		return NOERROR;
	}
	pMediaSample->AddRef();
	return m_pOutputQueue->Receive(pMediaSample);
}

#define CallQueue(call) \
	if(!m_pOutputQueue) return NOERROR; \
	m_pOutputQueue->##call; \
	return NOERROR; \
 
HRESULT CMpeg2DecOutputPin::DeliverEndOfStream()
{
	CallQueue(EOS());
}

HRESULT CMpeg2DecOutputPin::DeliverBeginFlush()
{
	CallQueue(BeginFlush());
}

HRESULT CMpeg2DecOutputPin::DeliverEndFlush()
{
	CallQueue(EndFlush());
}

HRESULT CMpeg2DecOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CallQueue(NewSegment(tStart, tStop, dRate));
}


//
// CSubpicInputPin
//

#define PTS2RT(pts) (10000i64*pts/90)

CSubpicInputPin::CSubpicInputPin(CTransformFilter* pFilter, HRESULT* phr)
	: CMpeg2DecInputPin(pFilter, phr, L"SubPicture")
	, m_spon(TRUE)
	, m_fsppal(false)
{
	m_sppal[0].Y = 0x00;
	m_sppal[0].U = m_sppal[0].V = 0x80;
	m_sppal[1].Y = 0xe0;
	m_sppal[1].U = m_sppal[1].V = 0x80;
	m_sppal[2].Y = 0x80;
	m_sppal[2].U = m_sppal[2].V = 0x80;
	m_sppal[3].Y = 0x20;
	m_sppal[3].U = m_sppal[3].V = 0x80;
}

HRESULT CSubpicInputPin::CheckMediaType(const CMediaType* mtIn)
{
	return (mtIn->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK
			|| mtIn->majortype == MEDIATYPE_MPEG2_PACK
			|| mtIn->majortype == MEDIATYPE_MPEG2_PES
			|| mtIn->majortype == MEDIATYPE_Video)
		   && (mtIn->subtype == MEDIASUBTYPE_DVD_SUBPICTURE
			   || mtIn->subtype == MEDIASUBTYPE_CVD_SUBPICTURE
			   || mtIn->subtype == MEDIASUBTYPE_SVCD_SUBPICTURE)
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CSubpicInputPin::SetMediaType(const CMediaType* mtIn)
{
	return CBasePin::SetMediaType(mtIn);
}

bool CSubpicInputPin::HasAnythingToRender(REFERENCE_TIME rt)
{
	if(!IsConnected()) {
		return false;
	}

	CAutoLock cAutoLock(&m_csReceive);

	POSITION pos = m_sps.GetHeadPosition();
	while(pos) {
		spu* sp = m_sps.GetNext(pos);
		if(sp->m_rtStart <= rt && rt < sp->m_rtStop && (/*sp->m_psphli ||*/ sp->m_fForced || m_spon)) {
			return true;
		}
	}

	return false;
}

void CSubpicInputPin::RenderSubpics(REFERENCE_TIME rt, BYTE** yuv, int w, int h)
{
	CAutoLock cAutoLock(&m_csReceive);

	POSITION pos;

	// remove no longer needed things first
	pos = m_sps.GetHeadPosition();
	while(pos) {
		POSITION cur = pos;
		spu* sp = m_sps.GetNext(pos);
		if(sp->m_rtStop <= rt) {
			m_sps.RemoveAt(cur);
		}
	}

	pos = m_sps.GetHeadPosition();
	while(pos) {
		spu* sp = m_sps.GetNext(pos);
		if(sp->m_rtStart <= rt && rt < sp->m_rtStop
				&& (m_spon || sp->m_fForced && ((static_cast<CMpeg2DecFilter*>(m_pFilter))->IsForcedSubtitlesEnabled() || sp->m_psphli))) {
			sp->Render(rt, yuv, w, h, m_sppal, m_fsppal);
		}
	}
}

HRESULT CSubpicInputPin::Transform(IMediaSample* pSample)
{
	HRESULT hr;

	AM_MEDIA_TYPE* pmt;
	if(SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt) {
		CMediaType mt(*pmt);
		SetMediaType(&mt);
		DeleteMediaType(pmt);
	}

	BYTE* pDataIn = NULL;
	if(FAILED(hr = pSample->GetPointer(&pDataIn))) {
		return hr;
	}

	long len = pSample->GetActualDataLength();

	StripPacket(pDataIn, len);

	if(len <= 0) {
		return S_FALSE;
	}

	if(m_mt.subtype == MEDIASUBTYPE_SVCD_SUBPICTURE) {
		pDataIn += 4;
		len -= 4;
	}

	if(len <= 0) {
		return S_FALSE;
	}

	CAutoLock cAutoLock(&m_csReceive);

	REFERENCE_TIME rtStart = 0, rtStop = 0;
	hr = pSample->GetTime(&rtStart, &rtStop);

	bool fRefresh = false;

	if(FAILED(hr)) {
		if(!m_sps.IsEmpty()) {
			spu* sp = m_sps.GetTail();
			sp->SetCount(sp->GetCount() + len);
			memcpy(sp->GetData() + sp->GetCount() - len, pDataIn, len);
		}
	} else {
		POSITION pos = m_sps.GetTailPosition();
		while(pos) {
			spu* sp = m_sps.GetPrev(pos);
			if(sp->m_rtStop == _I64_MAX) {
				sp->m_rtStop = rtStart;
				break;
			}
		}

		CAutoPtr<spu> p;

		if(m_mt.subtype == MEDIASUBTYPE_DVD_SUBPICTURE) {
			p.Attach(DNew dvdspu());
		} else if(m_mt.subtype == MEDIASUBTYPE_CVD_SUBPICTURE) {
			p.Attach(DNew cvdspu());
		} else if(m_mt.subtype == MEDIASUBTYPE_SVCD_SUBPICTURE) {
			p.Attach(DNew svcdspu());
		} else {
			return E_FAIL;
		}

		p->m_rtStart = rtStart;
		p->m_rtStop = _I64_MAX;

		p->SetCount(len);
		memcpy(p->GetData(), pDataIn, len);

		if(m_sphli && p->m_rtStart == PTS2RT(m_sphli->StartPTM)) {
			p->m_psphli = m_sphli;
			fRefresh = true;
		}

		m_sps.AddTail(p);
	}

	if(!m_sps.IsEmpty()) {
		m_sps.GetTail()->Parse();
	}

	if(fRefresh) {
		//		((CMpeg2DecFilter*)m_pFilter)->Deliver(true);
	}

	return S_FALSE;
}

STDMETHODIMP CSubpicInputPin::EndFlush()
{
	CAutoLock cAutoLock(&m_csReceive);
	m_sps.RemoveAll();
	return S_OK;
}

// IKsPropertySet

STDMETHODIMP CSubpicInputPin::Set(REFGUID PropSet, ULONG Id, LPVOID pInstanceData, ULONG InstanceLength, LPVOID pPropertyData, ULONG DataLength)
{
	if(PropSet != AM_KSPROPSETID_DvdSubPic) {
		return __super::Set(PropSet, Id, pInstanceData, InstanceLength, pPropertyData, DataLength);
	}

	bool fRefresh = false;

	switch(Id) {
		case AM_PROPERTY_DVDSUBPIC_PALETTE: {
			CAutoLock cAutoLock(&m_csReceive);

			AM_PROPERTY_SPPAL* pSPPAL = (AM_PROPERTY_SPPAL*)pPropertyData;
			memcpy(m_sppal, pSPPAL->sppal, sizeof(AM_PROPERTY_SPPAL));
			m_fsppal = true;

			DbgLog((LOG_TRACE, 0, _T("new palette")));
		}
		break;
		case AM_PROPERTY_DVDSUBPIC_HLI: {
			CAutoLock cAutoLock(&m_csReceive);

			AM_PROPERTY_SPHLI* pSPHLI = (AM_PROPERTY_SPHLI*)pPropertyData;

			m_sphli.Free();

			if(pSPHLI->HLISS) {
				POSITION pos = m_sps.GetHeadPosition();
				while(pos) {
					spu* sp = m_sps.GetNext(pos);
					if(sp->m_rtStart <= PTS2RT(pSPHLI->StartPTM) && PTS2RT(pSPHLI->StartPTM) < sp->m_rtStop) {
						fRefresh = true;
						sp->m_psphli.Free();
						sp->m_psphli.Attach(DNew AM_PROPERTY_SPHLI);
						memcpy((AM_PROPERTY_SPHLI*)sp->m_psphli, pSPHLI, sizeof(AM_PROPERTY_SPHLI));
					}
				}

				if(!fRefresh) { // save it for later, a subpic might be late for this hli
					m_sphli.Attach(DNew AM_PROPERTY_SPHLI);
					memcpy((AM_PROPERTY_SPHLI*)m_sphli, pSPHLI, sizeof(AM_PROPERTY_SPHLI));
				}
			} else {
				POSITION pos = m_sps.GetHeadPosition();
				while(pos) {
					spu* sp = m_sps.GetNext(pos);
					fRefresh |= !!sp->m_psphli;
					sp->m_psphli.Free();
				}
			}

			if(pSPHLI->HLISS)
				DbgLog((LOG_TRACE, 0, _T("hli: %I64d - %I64d, (%d,%d) - (%d,%d)"),
						PTS2RT(pSPHLI->StartPTM)/10000, PTS2RT(pSPHLI->EndPTM)/10000,
						pSPHLI->StartX, pSPHLI->StartY, pSPHLI->StopX, pSPHLI->StopY));
		}
		break;
		case AM_PROPERTY_DVDSUBPIC_COMPOSIT_ON: {
			CAutoLock cAutoLock(&m_csReceive);

			AM_PROPERTY_COMPOSIT_ON* pCompositOn = (AM_PROPERTY_COMPOSIT_ON*)pPropertyData;
			m_spon = *pCompositOn;
		}
		break;
		default:
			return E_PROP_ID_UNSUPPORTED;
	}

	if(fRefresh) {
		(static_cast<CMpeg2DecFilter*>(m_pFilter))->Deliver(true);
	}

	return S_OK;
}

STDMETHODIMP CSubpicInputPin::QuerySupported(REFGUID PropSet, ULONG Id, ULONG* pTypeSupport)
{
	if(PropSet != AM_KSPROPSETID_DvdSubPic) {
		return __super::QuerySupported(PropSet, Id, pTypeSupport);
	}

	switch(Id) {
		case AM_PROPERTY_DVDSUBPIC_PALETTE:
			*pTypeSupport = KSPROPERTY_SUPPORT_SET;
			break;
		case AM_PROPERTY_DVDSUBPIC_HLI:
			*pTypeSupport = KSPROPERTY_SUPPORT_SET;
			break;
		case AM_PROPERTY_DVDSUBPIC_COMPOSIT_ON:
			*pTypeSupport = KSPROPERTY_SUPPORT_SET;
			break;
		default:
			return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

// CSubpicInputPin::spu

static __inline BYTE GetNibble(BYTE* p, DWORD* offset, int& nField, int& fAligned)
{
	BYTE ret = (p[offset[nField]] >> (fAligned << 2)) & 0x0f;
	offset[nField] += 1-fAligned;
	fAligned = !fAligned;
	return ret;
}

static __inline BYTE GetHalfNibble(BYTE* p, DWORD* offset, int& nField, int& n)
{
	BYTE ret = (p[offset[nField]] >> (n << 1)) & 0x03;
	if(!n) {
		offset[nField]++;
	}
	n = (n-1+4)&3;
	return ret;
}

static __inline void DrawPixel(BYTE** yuv, CPoint pt, int pitch, AM_DVD_YUV& c)
{
	if(c.Reserved == 0) {
		return;
	}

	BYTE* p = &yuv[0][pt.y*pitch + pt.x];
	//	*p = (*p*(15-contrast) + sppal[color].Y*contrast)>>4;
	*p -= (*p - c.Y) * c.Reserved >> 4;

	if(pt.y&1) {
		return;    // since U/V is half res there is no need to overwrite the same line again
	}

	pt.x = (pt.x + 1) / 2;
	pt.y = (pt.y /*+ 1*/) / 2; // only paint the upper field always, don't round it
	pitch /= 2;

	// U/V is exchanged? wierd but looks true when comparing the outputted colors from other decoders

	p = &yuv[1][pt.y*pitch + pt.x];
	//	*p = (BYTE)(((((int)*p-0x80)*(15-contrast) + ((int)sppal[color].V-0x80)*contrast) >> 4) + 0x80);
	*p -= (*p - c.V) * c.Reserved >> 4;

	p = &yuv[2][pt.y*pitch + pt.x];
	//	*p = (BYTE)(((((int)*p-0x80)*(15-contrast) + ((int)sppal[color].U-0x80)*contrast) >> 4) + 0x80);
	*p -= (*p - c.U) * c.Reserved >> 4;

	// Neighter of the blending formulas are accurate (">>4" should be "/15").
	// Even though the second one is a bit worse, since we are scaling the difference only,
	// the error is still not noticable.
}

static __inline void DrawPixels(BYTE** yuv, int pitch, CPoint pt, int len, AM_DVD_YUV& c, CRect& rc)
{
	if(pt.y < rc.top || pt.y >= rc.bottom) {
		return;
	}
	if(pt.x < rc.left) {
		len -= rc.left - pt.x;
		pt.x = rc.left;
	}
	if(pt.x + len > rc.right) {
		len = rc.right - pt.x;
	}
	if(len <= 0 || pt.x >= rc.right) {
		return;
	}

	if(c.Reserved == 0) {
		if(rc.IsRectEmpty()) {
			return;
		}

		if(pt.y < rc.top || pt.y >= rc.bottom
				|| pt.x+len < rc.left || pt.x >= rc.right) {
			return;
		}
	}

	while(len-- > 0) {
		DrawPixel(yuv, pt, pitch, c);
		pt.x++;
	}
}

// CSubpicInputPin::dvdspu

bool CSubpicInputPin::dvdspu::Parse()
{
	m_offsets.RemoveAll();

	BYTE* p = GetData();

	WORD packetsize = (p[0]<<8)|p[1];
	WORD datasize = (p[2]<<8)|p[3];

	if(packetsize > GetCount() || datasize > packetsize) {
		return false;
	}

	int i, next = datasize;

#define GetWORD (p[i]<<8)|p[i+1]; i += 2

	do {
		i = next;

		int pts = GetWORD;
		next = GetWORD;

		if(next > packetsize || next < datasize) {
			return false;
		}

		REFERENCE_TIME rt = m_rtStart + 1024*PTS2RT(pts);

		for(bool fBreak = false; !fBreak; ) {
			int len = 0;

			switch(p[i]) {
				case 0x00:
					len = 0;
					break;
				case 0x01:
					len = 0;
					break;
				case 0x02:
					len = 0;
					break;
				case 0x03:
					len = 2;
					break;
				case 0x04:
					len = 2;
					break;
				case 0x05:
					len = 6;
					break;
				case 0x06:
					len = 4;
					break;
				case 0x07:
					len = 0;
					break; // TODO
				default:
					len = 0;
					break;
			}

			if(i+len >= packetsize) {
				TRACE(_T("Warning: Wrong subpicture parameter block ending\n"));
				break;
			}

			switch(p[i++]) {
				case 0x00: // forced start displaying
					m_fForced = true;
					break;
				case 0x01: // normal start displaying
					m_fForced = false;
					break;
				case 0x02: // stop displaying
					m_rtStop = rt;
					break;
				case 0x03:
					m_sphli.ColCon.emph2col = p[i]>>4;
					m_sphli.ColCon.emph1col = p[i]&0xf;
					m_sphli.ColCon.patcol = p[i+1]>>4;
					m_sphli.ColCon.backcol = p[i+1]&0xf;
					i += 2;
					break;
				case 0x04:
					m_sphli.ColCon.emph2con = p[i]>>4;
					m_sphli.ColCon.emph1con = p[i]&0xf;
					m_sphli.ColCon.patcon = p[i+1]>>4;
					m_sphli.ColCon.backcon = p[i+1]&0xf;
					i += 2;
					break;
				case 0x05:
					m_sphli.StartX = (p[i]<<4) + (p[i+1]>>4);
					m_sphli.StopX = ((p[i+1]&0x0f)<<8) + p[i+2]+1;
					m_sphli.StartY = (p[i+3]<<4) + (p[i+4]>>4);
					m_sphli.StopY = ((p[i+4]&0x0f)<<8) + p[i+5]+1;
					i += 6;
					break;
				case 0x06:
					m_offset[0] = GetWORD;
					m_offset[1] = GetWORD;
					break;
				case 0x07:
					// TODO
					break;
				case 0xff: // end of ctrlblk
					fBreak = true;
					continue;
				default: // skip this ctrlblk
					fBreak = true;
					break;
			}
		}

		offset_t o = {rt, m_sphli};
		m_offsets.AddTail(o); // is it always going to be sorted?
	} while(i <= next && i < packetsize);

	return true;
}

void CSubpicInputPin::dvdspu::Render(REFERENCE_TIME rt, BYTE** yuv, int w, int h, AM_DVD_YUV* sppal, bool fsppal)
{
	BYTE* p = GetData();
	DWORD offset[2] = {m_offset[0], m_offset[1]};

	AM_PROPERTY_SPHLI sphli = m_sphli;
	CPoint pt(sphli.StartX, sphli.StartY);
	CRect rc(pt, CPoint(sphli.StopX, sphli.StopY));

	CRect rcclip(0, 0, w, h);
	rcclip &= rc;

	if(m_psphli) {
		rcclip &= CRect(m_psphli->StartX, m_psphli->StartY, m_psphli->StopX, m_psphli->StopY);
		sphli = *m_psphli;
	} else if(m_offsets.GetCount() > 1) {
		POSITION pos = m_offsets.GetTailPosition();
		while(pos) {
			const offset_t& o = m_offsets.GetPrev(pos);
			if(rt >= o.rt) {
				sphli = o.sphli;
				break;
			}
		}
	}

	AM_DVD_YUV pal[4];
	pal[0] = sppal[fsppal ? sphli.ColCon.backcol : 0];
	pal[0].Reserved = sphli.ColCon.backcon;
	pal[1] = sppal[fsppal ? sphli.ColCon.patcol : 1];
	pal[1].Reserved = sphli.ColCon.patcon;
	pal[2] = sppal[fsppal ? sphli.ColCon.emph1col : 2];
	pal[2].Reserved = sphli.ColCon.emph1con;
	pal[3] = sppal[fsppal ? sphli.ColCon.emph2col : 3];
	pal[3].Reserved = sphli.ColCon.emph2con;

	int nField = 0;
	int fAligned = 1;

	DWORD end[2] = {offset[1], (p[2]<<8)|p[3]};

	while((nField == 0 && offset[0] < end[0]) || (nField == 1 && offset[1] < end[1])) {
		DWORD code;

		if((code = GetNibble(p, offset, nField, fAligned)) >= 0x4
				|| (code = (code << 4) | GetNibble(p, offset, nField, fAligned)) >= 0x10
				|| (code = (code << 4) | GetNibble(p, offset, nField, fAligned)) >= 0x40
				|| (code = (code << 4) | GetNibble(p, offset, nField, fAligned)) >= 0x100) {
			DrawPixels(yuv, w, pt, code >> 2, pal[code&3], rcclip);
			if((pt.x += code >> 2) < rc.right) {
				continue;
			}
		}

		DrawPixels(yuv, w, pt, rc.right - pt.x, pal[code&3], rcclip);

		if(!fAligned) {
			GetNibble(p, offset, nField, fAligned);    // align to byte
		}

		pt.x = rc.left;
		pt.y++;
		nField = 1 - nField;
	}
}

// CSubpicInputPin::cvdspu

bool CSubpicInputPin::cvdspu::Parse()
{
	BYTE* p = GetData();

	WORD packetsize = (p[0]<<8)|p[1];
	WORD datasize = (p[2]<<8)|p[3];

	if(packetsize > GetCount() || datasize > packetsize) {
		return false;
	}

	p = GetData() + datasize;

	for(int i = datasize, j = packetsize-4; i <= j; i+=4, p+=4) {
		switch(p[0]) {
			case 0x0c:
				break;
			case 0x04:
				m_rtStop = m_rtStart + 10000i64*((p[1]<<16)|(p[2]<<8)|p[3])/90;
				break;
			case 0x17:
				m_sphli.StartX = ((p[1]&0x0f)<<6) + (p[2]>>2);
				m_sphli.StartY = ((p[2]&0x03)<<8) + p[3];
				break;
			case 0x1f:
				m_sphli.StopX = ((p[1]&0x0f)<<6) + (p[2]>>2);
				m_sphli.StopY = ((p[2]&0x03)<<8) + p[3];
				break;
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				m_sppal[0][p[0]-0x24].Y = p[1];
				m_sppal[0][p[0]-0x24].U = p[2];
				m_sppal[0][p[0]-0x24].V = p[3];
				break;
			case 0x2c:
			case 0x2d:
			case 0x2e:
			case 0x2f:
				m_sppal[1][p[0]-0x2c].Y = p[1];
				m_sppal[1][p[0]-0x2c].U = p[2];
				m_sppal[1][p[0]-0x2c].V = p[3];
				break;
			case 0x37:
				m_sppal[0][3].Reserved = p[2]>>4;
				m_sppal[0][2].Reserved = p[2]&0xf;
				m_sppal[0][1].Reserved = p[3]>>4;
				m_sppal[0][0].Reserved = p[3]&0xf;
				break;
			case 0x3f:
				m_sppal[1][3].Reserved = p[2]>>4;
				m_sppal[1][2].Reserved = p[2]&0xf;
				m_sppal[1][1].Reserved = p[3]>>4;
				m_sppal[1][0].Reserved = p[3]&0xf;
				break;
			case 0x47:
				m_offset[0] = (p[2]<<8)|p[3];
				break;
			case 0x4f:
				m_offset[1] = (p[2]<<8)|p[3];
				break;
			default:
				break;
		}
	}

	return true;
}

void CSubpicInputPin::cvdspu::Render(REFERENCE_TIME rt, BYTE** yuv, int w, int h, AM_DVD_YUV* sppal, bool fsppal)
{
	BYTE* p = GetData();
	DWORD offset[2] = {m_offset[0], m_offset[1]};

	CRect rcclip(0, 0, w, h);

	/* FIXME: startx/y looks to be wrong in the sample I tested
	CPoint pt(m_sphli.StartX, m_sphli.StartY);
	CRect rc(pt, CPoint(m_sphli.StopX, m_sphli.StopY));
	*/

	CSize size(m_sphli.StopX - m_sphli.StartX, m_sphli.StopY - m_sphli.StartY);
	CPoint pt((rcclip.Width() - size.cx) / 2, (rcclip.Height()*3 - size.cy*1) / 4);
	CRect rc(pt, size);

	int nField = 0;
	int fAligned = 1;

	DWORD end[2] = {offset[1], (p[2]<<8)|p[3]};

	while((nField == 0 && offset[0] < end[0]) || (nField == 1 && offset[1] < end[1])) {
		BYTE code;

		if((code = GetNibble(p, offset, nField, fAligned)) >= 0x4) {
			DrawPixels(yuv, w, pt, code >> 2, m_sppal[0][code&3], rcclip);
			pt.x += code >> 2;
			continue;
		}

		code = GetNibble(p, offset, nField, fAligned);
		DrawPixels(yuv, w, pt, rc.right - pt.x, m_sppal[0][code&3], rcclip);

		if(!fAligned) {
			GetNibble(p, offset, nField, fAligned);    // align to byte
		}

		pt.x = rc.left;
		pt.y++;
		nField = 1 - nField;
	}
}

// CSubpicInputPin::svcdspu

bool CSubpicInputPin::svcdspu::Parse()
{
	BYTE* p = GetData();
	BYTE* p0 = p;

	if(GetCount() < 2) {
		return false;
	}

	WORD packetsize = (p[0]<<8)|p[1];
	p += 2;

	if(packetsize > GetCount()) {
		return false;
	}

	bool duration = !!(*p++&0x04);

	*p++; // unknown

	if(duration) {
		m_rtStop = m_rtStart + 10000i64*((p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3])/90;
		p += 4;
	}

	m_sphli.StartX = m_sphli.StopX = (p[0]<<8)|p[1];
	p += 2;
	m_sphli.StartY = m_sphli.StopY = (p[0]<<8)|p[1];
	p += 2;
	m_sphli.StopX += (p[0]<<8)|p[1];
	p += 2;
	m_sphli.StopY += (p[0]<<8)|p[1];
	p += 2;

	for(int i = 0; i < 4; i++) {
		m_sppal[i].Y = *p++;
		m_sppal[i].U = *p++;
		m_sppal[i].V = *p++;
		m_sppal[i].Reserved = *p++ >> 4;
	}

	if(*p++&0xc0) {
		p += 4;    // duration of the shift operation should be here, but it is untested
	}

	m_offset[1] = (p[0]<<8)|p[1];
	p += 2;

	m_offset[0] = p - p0;
	m_offset[1] += m_offset[0];

	return true;
}

void CSubpicInputPin::svcdspu::Render(REFERENCE_TIME rt, BYTE** yuv, int w, int h, AM_DVD_YUV* sppal, bool fsppal)
{
	BYTE* p = GetData();
	DWORD offset[2] = {m_offset[0], m_offset[1]};

	CRect rcclip(0, 0, w, h);

	/* FIXME: startx/y looks to be wrong in the sample I tested (yes, this one too!)
	CPoint pt(m_sphli.StartX, m_sphli.StartY);
	CRect rc(pt, CPoint(m_sphli.StopX, m_sphli.StopY));
	*/

	CSize size(m_sphli.StopX - m_sphli.StartX, m_sphli.StopY - m_sphli.StartY);
	CPoint pt((rcclip.Width() - size.cx) / 2, (rcclip.Height()*3 - size.cy*1) / 4);
	CRect rc(pt, size);

	int nField = 0;
	int n = 3;

	DWORD end[2] = {offset[1], (p[2]<<8)|p[3]};

	while((nField == 0 && offset[0] < end[0]) || (nField == 1 && offset[1] < end[1])) {
		BYTE code = GetHalfNibble(p, offset, nField, n);
		BYTE repeat = 1 + (code == 0 ? GetHalfNibble(p, offset, nField, n) : 0);

		DrawPixels(yuv, w, pt, repeat, m_sppal[code&3], rcclip);
		if((pt.x += repeat) < rc.right) {
			continue;
		}

		while(n != 3) {
			GetHalfNibble(p, offset, nField, n);    // align to byte
		}

		pt.x = rc.left;
		pt.y++;
		nField = 1 - nField;
	}
}

//
// CClosedCaptionOutputPin
//

CClosedCaptionOutputPin::CClosedCaptionOutputPin(CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
	: CBaseOutputPin(NAME("CClosedCaptionOutputPin"), pFilter, pLock, phr, L"~CC")
{
}

HRESULT CClosedCaptionOutputPin::CheckMediaType(const CMediaType* mtOut)
{
	return mtOut->majortype == MEDIATYPE_AUXLine21Data && mtOut->subtype == MEDIASUBTYPE_Line21_GOPPacket
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CClosedCaptionOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
	if(iPosition < 0) {
		return E_INVALIDARG;
	}
	if(iPosition > 0) {
		return VFW_S_NO_MORE_ITEMS;
	}

	pmt->InitMediaType();
	pmt->majortype = MEDIATYPE_AUXLine21Data;
	pmt->subtype = MEDIASUBTYPE_Line21_GOPPacket;
	pmt->formattype = FORMAT_None;

	return S_OK;
}

HRESULT CClosedCaptionOutputPin::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	pProperties->cBuffers = 1;
	pProperties->cbBuffer = 2048;
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
	if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
		return hr;
	}

	return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		   ? E_FAIL
		   : NOERROR;
}

HRESULT CClosedCaptionOutputPin::Deliver(const void* ptr, int len)
{
	HRESULT hr = S_FALSE;

	if(len > 4 && ptr && *(DWORD*)ptr == 0xf8014343 && IsConnected()) {
		CComPtr<IMediaSample> pSample;
		GetDeliveryBuffer(&pSample, NULL, NULL, 0);
		BYTE* pData = NULL;
		pSample->GetPointer(&pData);
		*(DWORD*)pData = 0xb2010000;
		memcpy(pData + 4, ptr, len);
		pSample->SetActualDataLength(len + 4);
		hr = __super::Deliver(pSample);
	}

	return hr;
}
