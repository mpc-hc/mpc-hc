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
#include "PPageInternalFilters.h"
#include "ComPropertySheet.h"
#include "../../filters/filters.h"
#include "internal_filter_config.h"


static struct filter_t {
	LPCTSTR label;
	int type;
	int flag;
	UINT nHintID;
	CUnknown* (WINAPI * CreateInstance)(LPUNKNOWN lpunk, HRESULT* phr);
}
s_filters[] = {
#if INTERNAL_SOURCEFILTER_AVI
	{_T("AVI"), SOURCE_FILTER, SRC_AVI, IDS_SRC_AVI, NULL},
#endif
#if INTERNAL_SOURCEFILTER_CDDA
	{_T("CDDA (Audio CD)"), SOURCE_FILTER, SRC_CDDA, IDS_SRC_CDDA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_CDXA
	{_T("CDXA (VCD/SVCD/XCD)"), SOURCE_FILTER, SRC_CDXA, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_DSM
	{_T("DirectShow Media"), SOURCE_FILTER, SRC_DSM, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_DTSAC3
	{_T("DTS/AC3"), SOURCE_FILTER, SRC_DTSAC3, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_VTS
	{_T("DVD Video Title Set"), SOURCE_FILTER, SRC_VTS, IDS_SRC_VTS, NULL},
#endif
#if INTERNAL_SOURCEFILTER_DVSOURCE
	{_T("DVD2AVI Project File"), SOURCE_FILTER, SRC_D2V, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_FLIC
	{_T("FLI/FLC"), SOURCE_FILTER, SRC_FLIC, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_FLAC
	{_T("Flac"), SOURCE_FILTER, SRC_FLAC, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_FLV
	{_T("FLV"), SOURCE_FILTER, SRC_FLV, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MATROSKA
	{_T("Matroska"), SOURCE_FILTER, SRC_MATROSKA, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MP4
	{_T("MP4/MOV"), SOURCE_FILTER, SRC_MP4, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MPEGAUDIO
	{_T("MPEG Audio"), SOURCE_FILTER, SRC_MPA, IDS_SRC_MPA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MPEG
	{_T("MPEG PS/TS/PVA"), SOURCE_FILTER, SRC_MPEG, 0, CreateInstance<CMpegSplitterFilter>},
#endif
#if INTERNAL_SOURCEFILTER_OGG
	{_T("Ogg"), SOURCE_FILTER, SRC_OGG, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_REALMEDIA
	{_T("RealMedia"), SOURCE_FILTER, SRC_REALMEDIA, IDS_SRC_REALMEDIA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_SHOUTCAST
	{_T("SHOUTcast"), SOURCE_FILTER, SRC_SHOUTCAST, 0, NULL},
#endif

#if INTERNAL_DECODER_AAC
	{_T("AAC"), DECODER, TRA_AAC, IDS_TRA_AAC, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_AC3
	{_T("AC3"), DECODER, TRA_AC3, IDS_TRA_AC3, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_DTS
	{_T("DTS"), DECODER, TRA_DTS, IDS_TRA_DTS, CreateInstance<CMpaDecFilter>},
	{_T("LPCM"), DECODER, TRA_LPCM, IDS_TRA_LPCM, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_MPEGAUDIO
	{_T("MPEG Audio"), DECODER, TRA_MPA, IDS_TRA_MPA, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_VORBIS
	{_T("Vorbis"), DECODER, TRA_VORBIS, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_FLAC
	{_T("FLAC"), DECODER, TRA_FLAC, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_NELLYMOSER
	{_T("Nellymoser"), DECODER, TRA_NELLY, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_AMR
	{_T("AMR"), DECODER, TRA_AMR, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_REALAUDIO
	{_T("RealAudio"), DECODER, TRA_RA, IDS_TRA_RA, NULL},
#endif
#if INTERNAL_DECODER_PS2AUDIO
	{_T("PS2 Audio (PCM/ADPCM)"), DECODER, TRA_PS2AUD, IDS_TRA_PS2AUD, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_PCM
	{_T("QT PCM"), DECODER, TRA_PCM, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif

#if INTERNAL_DECODER_MPEG1
	{_T("MPEG-1 Video"), DECODER, TRA_MPEG1, IDS_TRA_MPEG1, CreateInstance<CMpeg2DecFilter>},
#endif
#if INTERNAL_DECODER_MPEG2
	{_T("MPEG-2 Video"), DECODER, TRA_MPEG2, IDS_TRA_MPEG2, CreateInstance<CMpeg2DecFilter>},
#endif
#if INTERNAL_DECODER_MPEG2_DXVA
	{_T("MPEG-2 Video (DXVA)"), DXVA_DECODER, TRA_DXVA_MPEG2, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_REALVIDEO
	{_T("RealVideo"), DECODER, TRA_RV, IDS_TRA_RV, NULL},
#endif

#if INTERNAL_DECODER_H264_DXVA
	{_T("H264/AVC (DXVA)"), DXVA_DECODER, TRA_DXVA_H264, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_H264
	{_T("H264/AVC (FFmpeg)"), FFMPEG_DECODER, FFM_H264, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VC1_DXVA
	{_T("VC1 (DXVA)"), DXVA_DECODER, TRA_DXVA_VC1, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VC1
	{_T("VC1 (FFmpeg)"), FFMPEG_DECODER, FFM_VC1, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_XVID
	{_T("Xvid/MPEG-4"), FFMPEG_DECODER, FFM_XVID, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_DIVX
	{_T("DivX"), FFMPEG_DECODER, FFM_DIVX, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_MSMPEG4
	{_T("MS MPEG-4"), FFMPEG_DECODER, FFM_MSMPEG4, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_FLV
	{_T("FLV1/4"), FFMPEG_DECODER, FFM_FLV4, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VP6
	{_T("VP5/6"), FFMPEG_DECODER, FFM_VP62, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VP8
	{_T("VP8"), FFMPEG_DECODER, FFM_VP8, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_WMV
	{_T("WMV1/2/3"), FFMPEG_DECODER, FFM_WMV, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_SVQ
	{_T("SVQ1/3"), FFMPEG_DECODER, FFM_SVQ3, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_H263
	{_T("H263"), FFMPEG_DECODER, FFM_H263, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_AMVV
	{_T("AMV video"), FFMPEG_DECODER, FFM_AMVV, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_THEORA
	{_T("Theora"), FFMPEG_DECODER, FFM_THEORA, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
	{NULL, 0, 0, 0, NULL}
};

IMPLEMENT_DYNAMIC(CPPageInternalFiltersListBox, CCheckListBox)
CPPageInternalFiltersListBox::CPPageInternalFiltersListBox(int n)
	: CCheckListBox()
	, m_n(n)
{
}

void CPPageInternalFiltersListBox::PreSubclassWindow()
{
	__super::PreSubclassWindow();
	EnableToolTips(TRUE);
}

INT_PTR CPPageInternalFiltersListBox::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	BOOL b = FALSE;
	int row = ItemFromPoint(point, b);
	if (row < 0) {
		return -1;
	}

	CRect r;
	GetItemRect(row, r);
	pTI->rect = r;
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT)row;
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	pTI->uFlags |= TTF_ALWAYSTIP;

	return pTI->uId;
}

BEGIN_MESSAGE_MAP(CPPageInternalFiltersListBox, CCheckListBox)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CPPageInternalFiltersListBox::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

	filter_t* f = (filter_t*)GetItemDataPtr(pNMHDR->idFrom);
	if (f->nHintID == 0) {
		return FALSE;
	}

	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT)1000);

	static CString m_strTipText;

	m_strTipText = ResStr(f->nHintID);

	pTTT->lpszText = m_strTipText.GetBuffer();

	*pResult = 0;

	return TRUE;    // message was handled
}

void CPPageInternalFiltersListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	CFont* pOldFont = NULL;

	if ((lpDrawItemStruct->itemData != 0) && ((filter_t*)lpDrawItemStruct->itemData)->CreateInstance) {
		if (!(HFONT)m_bold) {
			CFont* pFont = pDC->GetCurrentFont();

			LOGFONT lf;
			pFont->GetLogFont(&lf);
			lf.lfWeight = FW_BOLD;

			m_bold.CreateFontIndirect(&lf);
		}

		if ((HFONT)m_bold) {
			pOldFont = pDC->SelectObject(&m_bold);
		}
	}

	__super::DrawItem(lpDrawItemStruct);

	if (pOldFont) {
		pDC->SelectObject(pOldFont);
	}
}

void CPPageInternalFiltersListBox::OnRButtonDown(UINT nFlags, CPoint point)
{
	CCheckListBox::OnRButtonDown(nFlags, point);

	CMenu m;
	m.CreatePopupMenu();

	enum {
		ENABLEALL=1,
		DISABLEALL,
		ENABLEFFDSHOW,
		DISABLEFFDSHOW,
		ENABLEDXVA,
		DISABLEDXVA,
	};

	m.AppendMenu(MF_STRING|MF_ENABLED, ENABLEALL, ResStr(IDS_ENABLE_ALL_FILTERS));
	m.AppendMenu(MF_STRING|MF_ENABLED, DISABLEALL, ResStr(IDS_DISABLE_ALL_FILTERS));
	if (m_n == 1) {
		m.AppendMenu(MF_SEPARATOR);
		m.AppendMenu(MF_STRING|MF_ENABLED, ENABLEFFDSHOW, ResStr(IDS_ENABLE_FFMPEG_FILTERS));
		m.AppendMenu(MF_STRING|MF_ENABLED, DISABLEFFDSHOW, ResStr(IDS_DISABLE_FFMPEG_FILTERS));
		m.AppendMenu(MF_SEPARATOR);
		m.AppendMenu(MF_STRING|MF_ENABLED, ENABLEDXVA, ResStr(IDS_ENABLE_DXVA_FILTERS));
		m.AppendMenu(MF_STRING|MF_ENABLED, DISABLEDXVA, ResStr(IDS_DISABLE_DXVA_FILTERS));
	}

	CPoint p = point;
	::MapWindowPoints(m_hWnd, HWND_DESKTOP, &p, 1);

	UINT id = m.TrackPopupMenu(TPM_LEFTBUTTON|TPM_RETURNCMD, p.x, p.y, this);

	if (id == 0) {
		return;
	}

	int Index = 0;
	for (int i = 0; i < countof(s_filters); i++) {
		switch (s_filters[i].type) {
			case 0: // source filter
				if (m_n == 1) {
					continue;
				}
				break;
			case 1: // decoder
			case 2: // dxva decoder
			case 3: // ffmpeg decoder
				if (m_n == 0) {
					continue;
				}
				break;
			default:
				continue;
		}

		switch (id) {
			case ENABLEALL:
				SetCheck(Index, TRUE);
				break;
			case DISABLEALL:
				SetCheck(Index, FALSE);
				break;
			case ENABLEFFDSHOW:
				if (s_filters[i].type == 3) {
					SetCheck(Index, TRUE);
				}
				break;
			case DISABLEFFDSHOW:
				if (s_filters[i].type == 3) {
					SetCheck(Index, FALSE);
				}
				break;
			case ENABLEDXVA:
				if (s_filters[i].type == 2) {
					SetCheck(Index, TRUE);
				}
				break;
			case DISABLEDXVA:
				if (s_filters[i].type == 2) {
					SetCheck(Index, FALSE);
				}
				break;
		}
		Index++;
	}

	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CLBN_CHKCHANGE), (LPARAM)m_hWnd);
}

// CPPageInternalFilters dialog

IMPLEMENT_DYNAMIC(CPPageInternalFilters, CPPageBase)
CPPageInternalFilters::CPPageInternalFilters()
	: CPPageBase(CPPageInternalFilters::IDD, CPPageInternalFilters::IDD)
	, m_listSrc(0)
	, m_listTra(1)
{
}

CPPageInternalFilters::~CPPageInternalFilters()
{
}


void CPPageInternalFilters::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listSrc);
	DDX_Control(pDX, IDC_LIST2, m_listTra);
}

BEGIN_MESSAGE_MAP(CPPageInternalFilters, CPPageBase)
	ON_LBN_DBLCLK(IDC_LIST1, OnLbnDblclkList1)
	ON_LBN_DBLCLK(IDC_LIST2, OnLbnDblclkList2)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelChange)
	ON_LBN_SELCHANGE(IDC_LIST2, OnSelChange)
	ON_CLBN_CHKCHANGE(IDC_LIST1, OnCheckBoxChange)
	ON_CLBN_CHKCHANGE(IDC_LIST2, OnCheckBoxChange)
END_MESSAGE_MAP()

// CPPageInternalFilters message handlers

BOOL CPPageInternalFilters::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	for (int i = 0; i < countof(s_filters)-1; i++) {
		CCheckListBox* l;
		bool checked;

		switch (s_filters[i].type) {
			case SOURCE_FILTER: // source filter
				l = &m_listSrc;
				checked = s.SrcFilters[s_filters[i].flag];
				break;
			case DECODER: // decoder
				l = &m_listTra;
				checked = s.TraFilters[s_filters[i].flag];
				break;
			case DXVA_DECODER: // dxva decoder
				l = &m_listTra;
				checked = s.DXVAFilters[s_filters[i].flag];
				break;
			case FFMPEG_DECODER: // ffmpeg decoder
				l = &m_listTra;
				checked = s.FFmpegFilters[s_filters[i].flag];
				break;
			default:
				l = NULL;
				checked = false;
		}

		if (l) {
			int Index = l->AddString(s_filters[i].label);
			l->SetCheck(Index, checked);
			l->SetItemDataPtr(Index, &s_filters[i]);
		}
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageInternalFilters::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	CPPageInternalFiltersListBox* list = &m_listSrc;
	for (int l=0; l<2; l++) {
		for (int i = 0; i < list->GetCount(); i++) {
			filter_t* f = (filter_t*) list->GetItemDataPtr(i);

			switch (f->type) {
				case SOURCE_FILTER:
					s.SrcFilters[f->flag] = list->GetCheck(i);
					break;
				case DECODER:
					s.TraFilters[f->flag] = list->GetCheck(i);
					break;
				case DXVA_DECODER:
					s.DXVAFilters[f->flag] = list->GetCheck(i);
					break;
				case FFMPEG_DECODER:
					s.FFmpegFilters[f->flag] = list->GetCheck(i);
					break;
			}
		}
		list = &m_listTra;
	}

	return __super::OnApply();
}

void CPPageInternalFilters::ShowPPage(CPPageInternalFiltersListBox& l)
{
	int i = l.GetCurSel();
	if (i < 0) {
		return;
	}

	filter_t* f = (filter_t*)l.GetItemDataPtr(i);
	if (!f || !f->CreateInstance) {
		return;
	}

	HRESULT hr;
	CUnknown* pObj = f->CreateInstance(NULL, &hr);
	if (!pObj) {
		return;
	}

	CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pObj;

	if (SUCCEEDED(hr)) {
		if (CComQIPtr<ISpecifyPropertyPages> pSPP = pUnk) {
			CComPropertySheet ps(ResStr(IDS_PROPSHEET_PROPERTIES), this);
			ps.AddPages(pSPP);
			ps.DoModal();
		}
	}
}

void CPPageInternalFilters::OnLbnDblclkList1()
{
	ShowPPage(m_listSrc);
}

void CPPageInternalFilters::OnLbnDblclkList2()
{
	ShowPPage(m_listTra);
}

void CPPageInternalFilters::OnSelChange()
{
	// We only catch the message so that the page is not marked as modified.
}

void CPPageInternalFilters::OnCheckBoxChange()
{
	SetModified();
}