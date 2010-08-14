/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
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


static struct filter_t
{
	LPCTSTR label;
	int type;
	int flag;
	UINT nHintID;
	CUnknown* (WINAPI * CreateInstance)(LPUNKNOWN lpunk, HRESULT* phr);
}
s_filters[] =
{
#if INTERNAL_SOURCEFILTER_AVI
	{_T("AVI"), 0, SRC_AVI, IDS_SRC_AVI, NULL},
#endif
#if INTERNAL_SOURCEFILTER_CDDA
	{_T("CDDA (Audio CD)"), 0, SRC_CDDA, IDS_SRC_CDDA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_CDXA
	{_T("CDXA (VCD/SVCD/XCD)"), 0, SRC_CDXA, IDS_SRC_CDXA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_DIRAC
	{_T("Dirac"), 0, SRC_DIRAC, IDS_SRC_DIRAC, NULL},
#endif
#if INTERNAL_SOURCEFILTER_DSM
	{_T("DirectShow Media"), 0, SRC_DSM, IDS_SRC_DSM, NULL},
#endif
#if INTERNAL_SOURCEFILTER_DTSAC3
	{_T("DTS/AC3"), 0, SRC_DTSAC3, IDS_SRC_DTSAC3, NULL},
#endif
#if INTERNAL_SOURCEFILTER_VTS
	{_T("DVD Video Title Set"), 0, SRC_VTS, IDS_SRC_VTS, NULL},
#endif
#if INTERNAL_SOURCEFILTER_DVSOURCE
	{_T("DVD2AVI Project File"), 0, SRC_D2V, IDS_SRC_D2V, NULL},
#endif
#if INTERNAL_SOURCEFILTER_FLIC
	{_T("FLI/FLC"), 0, SRC_FLIC, IDS_SRC_FLIC, NULL},
#endif
#if INTERNAL_SOURCEFILTER_FLAC
	{_T("Flac"), 0, SRC_FLAC, IDS_SRC_FLAC, NULL},
#endif
#if INTERNAL_SOURCEFILTER_FLV
	{_T("FLV"), 0, SRC_FLV, IDS_SRC_FLV, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MATROSKA
	{_T("Matroska"), 0, SRC_MATROSKA, IDS_SRC_MATROSKA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MP4
	{_T("MP4/MOV"), 0, SRC_MP4, IDS_SRC_MP4, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MPEGAUDIO
	{_T("MPEG Audio"), 0, SRC_MPA, IDS_SRC_MPA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_MPEG
	{_T("MPEG PS/TS/PVA"), 0, SRC_MPEG, 0, NULL},
#endif
#if INTERNAL_SOURCEFILTER_NUT
	{_T("Nut"), 0, SRC_NUT, IDS_SRC_NUT, NULL},
#endif
#if INTERNAL_SOURCEFILTER_OGG
	{_T("Ogg"), 0, SRC_OGG, IDS_SRC_OGG, NULL},
#endif
#if INTERNAL_SOURCEFILTER_REALMEDIA
	{_T("RealMedia"), 0, SRC_REALMEDIA, IDS_SRC_REALMEDIA, NULL},
#endif
#if INTERNAL_SOURCEFILTER_ROQ
	{_T("RoQ"), 0, SRC_ROQ, IDS_SRC_ROQ, NULL},
#endif
#if INTERNAL_SOURCEFILTER_SHOUTCAST
	{_T("SHOUTcast"), 0, SRC_SHOUTCAST, IDS_SRC_SHOUTCAST, NULL},
#endif
#if INTERNAL_DECODER_AAC
	{_T("AAC"), 1, TRA_AAC, IDS_TRA_AAC, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_AC3
	{_T("AC3"), 1, TRA_AC3, IDS_TRA_AC3, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_DTS
	{_T("DTS"), 1, TRA_DTS, IDS_TRA_DTS, CreateInstance<CMpaDecFilter>},
	{_T("LPCM"), 1, TRA_LPCM, IDS_TRA_LPCM, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_MPEGAUDIO
	{_T("MPEG Audio"), 1, TRA_MPA, IDS_TRA_MPA, CreateInstance<CMpaDecFilter>},
#endif
#if INTERNAL_DECODER_VORBIS
	{_T("Vorbis"), 1, TRA_VORBIS, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_FLAC
	{_T("FLAC"), 1, TRA_FLAC, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_NELLYMOSER
	{_T("Nellymoser"), 1, TRA_NELLY, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_AMR
	{_T("AMR"), 1, TRA_AMR, 0, NULL /* TODO: CreateInstance<CMpaDecFilter>*/},
#endif
#if INTERNAL_DECODER_REALAUDIO
	{_T("RealAudio"), 1, TRA_RA, IDS_TRA_RA, NULL},
#endif
#if INTERNAL_DECODER_PS2AUDIO
	{_T("PS2 Audio (PCM/ADPCM)"), 1, TRA_PS2AUD, IDS_TRA_PS2AUD, CreateInstance<CMpaDecFilter>},
#endif

#if INTERNAL_DECODER_MPEG1
	{_T("MPEG-1 Video"), 1, TRA_MPEG1, IDS_TRA_MPEG1, CreateInstance<CMpeg2DecFilter>},
#endif
#if INTERNAL_DECODER_MPEG2
	{_T("MPEG-2 Video"), 1, TRA_MPEG2, IDS_TRA_MPEG2, CreateInstance<CMpeg2DecFilter>},
#endif
#if INTERNAL_DECODER_MPEG2_DXVA
	{_T("MPEG-2 Video (DXVA)"), 2, DXVA_MPEG2, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_DIRAC
	{_T("Dirac"), 1, TRA_DIRAC, IDS_TRA_DIRAC, NULL},
#endif
#if INTERNAL_DECODER_REALVIDEO
	{_T("RealVideo"), 1, TRA_RV, IDS_TRA_RV, NULL},
#endif

#if INTERNAL_DECODER_H264_DXVA
	{_T("H264/AVC (DXVA)"), 2, DXVA_H264, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_H264
	{_T("H264/AVC (FFmpeg)"), 3, FFM_H264, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VC1_DXVA
	{_T("VC1 (DXVA)"), 2, DXVA_VC1, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VC1
	{_T("VC1 (FFmpeg)"), 3, FFM_VC1, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_XVID
	{_T("Xvid/MPEG-4"), 3, FFM_XVID, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_DIVX
	{_T("DivX"), 3, FFM_DIVX, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_MSMPEG4
	{_T("MS MPEG-4"), 3, FFM_MSMPEG4, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_FLV
	{_T("FLV1/4"), 3, FFM_FLV4, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VP6
	{_T("VP5/6"), 3, FFM_VP62, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_VP8
	{_T("VP8"), 3, FFM_VP8, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_WMV
	{_T("WMV1/2/3"), 3, FFM_WMV, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_SVQ
	{_T("SVQ1/3"), 3, FFM_SVQ3, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_H263
	{_T("H263"), 3, FFM_H263, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_AMVV
	{_T("AMV video"), 3, FFM_AMVV, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
#endif
#if INTERNAL_DECODER_THEORA
	{_T("Theora"), 3, FFM_THEORA, IDS_TRA_FFMPEG, CreateInstance<CMPCVideoDecFilter>},
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
	if(row < 0) return -1;

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
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CPPageInternalFiltersListBox::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	filter_t* f = (filter_t*)GetItemDataPtr(pNMHDR->idFrom);
	if(f->nHintID == 0) return FALSE;

	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT)1000);

	static CStringA m_strTipTextA;
	static CStringW m_strTipTextW;

	m_strTipTextA = CString(MAKEINTRESOURCE(f->nHintID));
	m_strTipTextW = CString(MAKEINTRESOURCE(f->nHintID));

	if(pNMHDR->code == TTN_NEEDTEXTA) pTTTA->lpszText = (LPSTR)(LPCSTR)m_strTipTextA;
	else pTTTW->lpszText = (LPWSTR)(LPCWSTR)m_strTipTextW;

	*pResult = 0;

	return TRUE;    // message was handled
}

void CPPageInternalFiltersListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	CFont* pOldFont = NULL;

	if((lpDrawItemStruct->itemData != 0) && ((filter_t*)lpDrawItemStruct->itemData)->CreateInstance)
	{
		if(!(HFONT)m_bold)
		{
			CFont* pFont = pDC->GetCurrentFont();

			LOGFONT lf;
			pFont->GetLogFont(&lf);
			lf.lfWeight = FW_BOLD;

			m_bold.CreateFontIndirect(&lf);
		}

		if((HFONT)m_bold)
		{
			pOldFont = pDC->SelectObject(&m_bold);
		}
	}

	__super::DrawItem(lpDrawItemStruct);

	if(pOldFont)
	{
		pDC->SelectObject(pOldFont);
	}
}

void CPPageInternalFiltersListBox::OnRButtonDown(UINT nFlags, CPoint point)
{
	CCheckListBox::OnRButtonDown(nFlags, point);

	CMenu m;
	m.CreatePopupMenu();

	enum
	{
		ENABLEALL=1,
		DISABLEALL,
		ENABLEFFDSHOW,
		DISABLEFFDSHOW,
		ENABLEDXVA,
		DISABLEDXVA,
	};

	m.AppendMenu(MF_STRING|MF_ENABLED, ENABLEALL, ResStr(IDS_ENABLE_ALL_FILTERS));
	m.AppendMenu(MF_STRING|MF_ENABLED, DISABLEALL, ResStr(IDS_DISABLE_ALL_FILTERS));
	if (m_n == 1)
	{
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

	if (id == 0)
		return;

	int Index = 0;
	for(int i = 0; i < countof(s_filters); i++)
	{
		switch(s_filters[i].type)
		{
		case 0: // source filter
			if (m_n == 1)
				continue;
			break;
		case 1: // decoder
		case 2: // dxva decoder
		case 3: // ffmpeg decoder
			if (m_n == 0)
				continue;
			break;
		default:
			continue;
		}

		switch(id)
		{
		case ENABLEALL:
			SetCheck(Index, TRUE);
			break;
		case DISABLEALL:
			SetCheck(Index, FALSE);
			break;
		case ENABLEFFDSHOW:
			if(s_filters[i].type == 3)
				SetCheck(Index, TRUE);
			break;
		case DISABLEFFDSHOW:
			if(s_filters[i].type == 3)
				SetCheck(Index, FALSE);
			break;
		case ENABLEDXVA:
			if(s_filters[i].type == 2)
				SetCheck(Index, TRUE);
			break;
		case DISABLEDXVA:
			if(s_filters[i].type == 2)
				SetCheck(Index, FALSE);
			break;
		}
		Index++;
	}
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
	ON_LBN_DBLCLK(IDC_LIST1, &CPPageInternalFilters::OnLbnDblclkList1)
	ON_LBN_DBLCLK(IDC_LIST2, &CPPageInternalFilters::OnLbnDblclkList2)
END_MESSAGE_MAP()

// CPPageInternalFilters message handlers

BOOL CPPageInternalFilters::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	for(int i = 0; i < countof(s_filters)-1; i++)
	{
		CCheckListBox* l;
		UINT* pflags;

		switch(s_filters[i].type)
		{
		case 0: // source filter
			l = &m_listSrc;
			pflags = &s.SrcFilters;
			break;
		case 1: // decoder
			l = &m_listTra;
			pflags = &s.TraFilters;
			break;
		case 2: // dxva decoder
			l = &m_listTra;
			pflags = &s.DXVAFilters;
			break;
		case 3: // ffmpeg decoder
			l = &m_listTra;
			pflags = &s.FFmpegFilters;
			break;
		default:
			l = NULL;
			pflags = NULL;
		}

		if(l && pflags)
		{
			int Index = l->AddString(s_filters[i].label);
			l->SetCheck(Index, !!(*pflags & s_filters[i].flag));
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

	s.SrcFilters = s.TraFilters = s.DXVAFilters = s.FFmpegFilters = 0;

	CList<filter_t*> fl;
	for(int i = 0; i < m_listSrc.GetCount(); i++)
		if(m_listSrc.GetCheck(i))
			fl.AddTail((filter_t*)m_listSrc.GetItemDataPtr(i));
	for(int i = 0; i < m_listTra.GetCount(); i++)
		if(m_listTra.GetCheck(i))
			fl.AddTail((filter_t*)m_listTra.GetItemDataPtr(i));

	POSITION pos = fl.GetHeadPosition();
	while(pos)
	{
		filter_t* f = fl.GetNext(pos);

		switch(f->type)
		{
		case 0:
			s.SrcFilters |= f->flag;
			break;
		case 1:
			s.TraFilters |= f->flag;
			break;
		case 2:
			s.DXVAFilters |= f->flag;
			break;
		case 3:
			s.FFmpegFilters |= f->flag;
			break;
		}
	}

	return __super::OnApply();
}

void CPPageInternalFilters::ShowPPage(CPPageInternalFiltersListBox& l)
{
	int i = l.GetCurSel();
	if(i < 0) return;

	filter_t* f = (filter_t*)l.GetItemDataPtr(i);
	if(!f || !f->CreateInstance) return;

	HRESULT hr;
	CUnknown* pObj = f->CreateInstance(NULL, &hr);
	if(!pObj) return;

	CComPtr<IUnknown> pUnk = (IUnknown*)(INonDelegatingUnknown*)pObj;

	if(SUCCEEDED(hr))
	{
		if(CComQIPtr<ISpecifyPropertyPages> pSPP = pUnk)
		{
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
