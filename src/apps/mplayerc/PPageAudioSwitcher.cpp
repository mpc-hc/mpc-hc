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
#include <math.h>
#include "mplayerc.h"
#include "PPageAudioSwitcher.h"


// CPPageAudioSwitcher dialog

IMPLEMENT_DYNAMIC(CPPageAudioSwitcher, CPPageBase)
CPPageAudioSwitcher::CPPageAudioSwitcher(IFilterGraph* pFG)
	: CPPageBase(CPPageAudioSwitcher::IDD, CPPageAudioSwitcher::IDD)
	, m_fAudioNormalize(FALSE)
	, m_fAudioNormalizeRecover(FALSE)
	, m_fDownSampleTo441(FALSE)
	, m_fCustomChannelMapping(FALSE)
	, m_nChannels(0)
	, m_fEnableAudioSwitcher(FALSE)
	, m_dwChannelMask(0)
	, m_tAudioTimeShift(0)
	, m_fAudioTimeShift(FALSE)
	, m_AudioBoost(0)
{
	m_pASF = FindFilter(__uuidof(CAudioSwitcherFilter), pFG);
}

CPPageAudioSwitcher::~CPPageAudioSwitcher()
{
}

void CPPageAudioSwitcher::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK5, m_fAudioNormalize);
	DDX_Check(pDX, IDC_CHECK6, m_fAudioNormalizeRecover);	
	DDX_Slider(pDX, IDC_SLIDER1, m_AudioBoost);
	DDX_Control(pDX, IDC_SLIDER1, m_AudioBoostCtrl);
	DDX_Check(pDX, IDC_CHECK3, m_fDownSampleTo441);
	DDX_Check(pDX, IDC_CHECK1, m_fCustomChannelMapping);
	DDX_Control(pDX, IDC_EDIT1, m_nChannelsCtrl);
	DDX_Text(pDX, IDC_EDIT1, m_nChannels);
	DDX_Control(pDX, IDC_SPIN1, m_nChannelsSpinCtrl);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Check(pDX, IDC_CHECK2, m_fEnableAudioSwitcher);
	DDX_Control(pDX, IDC_CHECK3, m_fDownSampleTo441Ctrl);
	DDX_Control(pDX, IDC_CHECK1, m_fCustomChannelMappingCtrl);
	DDX_Control(pDX, IDC_EDIT2, m_tAudioTimeShiftCtrl);
	DDX_Control(pDX, IDC_SPIN2, m_tAudioTimeShiftSpin);
	DDX_Text(pDX, IDC_EDIT2, m_tAudioTimeShift);
	DDX_Check(pDX, IDC_CHECK4, m_fAudioTimeShift);
	DDX_Control(pDX, IDC_CHECK4, m_fAudioTimeShiftCtrl);
}

BEGIN_MESSAGE_MAP(CPPageAudioSwitcher, CPPageBase)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickList1)
	ON_WM_DRAWITEM()
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	ON_UPDATE_COMMAND_UI(IDC_SLIDER1, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK5, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK6, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK3, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK4, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_EDIT2, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_SPIN2, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_CHECK1, OnUpdateAudioSwitcher)
	ON_UPDATE_COMMAND_UI(IDC_EDIT1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_SPIN1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_LIST1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_STATIC1, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_STATIC2, OnUpdateChannelMapping)
	ON_UPDATE_COMMAND_UI(IDC_STATIC3, OnUpdateChannelMapping)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CPPageAudioSwitcher message handlers

BOOL CPPageAudioSwitcher::OnInitDialog()
{
	__super::OnInitDialog();

	AppSettings& s = AfxGetAppSettings();

	m_fEnableAudioSwitcher = s.fEnableAudioSwitcher;
	m_fAudioNormalize = s.fAudioNormalize;
	m_fAudioNormalizeRecover = s.fAudioNormalizeRecover;	
	m_AudioBoost = (int)(50.0f*log10(s.AudioBoost));
	m_AudioBoostCtrl.SetRange(0, 100);
	m_fDownSampleTo441 = s.fDownSampleTo441;
	m_fAudioTimeShift = s.fAudioTimeShift;
	m_tAudioTimeShift = s.tAudioTimeShift;
	m_tAudioTimeShiftSpin.SetRange32(-1000*60*60*24, 1000*60*60*24);
	m_fCustomChannelMapping = s.fCustomChannelMapping;
	memcpy(m_pSpeakerToChannelMap, s.pSpeakerToChannelMap, sizeof(s.pSpeakerToChannelMap));

	if(m_pASF)
		m_pASF->GetInputSpeakerConfig(&m_dwChannelMask);

	m_nChannels = s.fnChannels;
	m_nChannelsSpinCtrl.SetRange(1, 18);

	if(m_pASF)
		m_nChannels = m_pASF->GetNumberOfInputChannels();		

	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, 100);
	m_list.InsertItem(0, _T(""));
	m_list.InsertItem(1, ResStr(IDS_FRONT_LEFT));
	m_list.InsertItem(2, ResStr(IDS_FRONT_RIGHT));
	m_list.InsertItem(3, ResStr(IDS_FRONT_CENTER));
	m_list.InsertItem(4, ResStr(IDS_LOW_FREQUENCY));
	m_list.InsertItem(5, ResStr(IDS_BACK_LEFT));
	m_list.InsertItem(6, ResStr(IDS_BACK_RIGHT));
	m_list.InsertItem(7, ResStr(IDS_FRONT_LEFT_OF_CENTER));
	m_list.InsertItem(8, ResStr(IDS_FRONT_RIGHT_OF_CENTER));
	m_list.InsertItem(9, ResStr(IDS_BACK_CENTER));
	m_list.InsertItem(10, ResStr(IDS_SIDE_LEFT));
	m_list.InsertItem(11, ResStr(IDS_SIDE_RIGHT));
	m_list.InsertItem(12, ResStr(IDS_TOP_CENTER));
	m_list.InsertItem(13, ResStr(IDS_TOP_FRONT_LEFT));
	m_list.InsertItem(14, ResStr(IDS_TOP_FRONT_CENTER));
	m_list.InsertItem(15, ResStr(IDS_TOP_FRONT_RIGHT));
	m_list.InsertItem(16, ResStr(IDS_TOP_BACK_LEFT));
	m_list.InsertItem(17, ResStr(IDS_TOP_BACK_CENTER));
	m_list.InsertItem(18, ResStr(IDS_TOP_BACK_RIGHT));
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);

	for(int i = 1; i <= 18; i++)
	{
		m_list.InsertColumn(i, _T(""), LVCFMT_CENTER, 16);
		CString n;
		n.Format(_T("%d"), i);
		m_list.SetItemText(0, i, n);
//		m_list.SetColumnWidth(i, LVSCW_AUTOSIZE);
//		m_list.SetColumnWidth(i, m_list.GetColumnWidth(i)*8/10);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageAudioSwitcher::OnApply()
{
	UpdateData();

	AppSettings& s = AfxGetAppSettings();

	s.fEnableAudioSwitcher = !!m_fEnableAudioSwitcher;
	s.fAudioNormalize = !!m_fAudioNormalize;
	s.fAudioNormalizeRecover = !!m_fAudioNormalizeRecover;
	s.AudioBoost = (float)pow(10.0, (double)m_AudioBoost/50);
	s.fDownSampleTo441 = !!m_fDownSampleTo441;
	s.fAudioTimeShift = !!m_fAudioTimeShift;
	s.tAudioTimeShift = m_tAudioTimeShift;
	s.fCustomChannelMapping = !!m_fCustomChannelMapping;
	memcpy(s.pSpeakerToChannelMap, m_pSpeakerToChannelMap, sizeof(m_pSpeakerToChannelMap));

	if(m_pASF)
	{
		m_pASF->SetSpeakerConfig(s.fCustomChannelMapping, s.pSpeakerToChannelMap);
		m_pASF->EnableDownSamplingTo441(s.fDownSampleTo441);
		m_pASF->SetAudioTimeShift(s.fAudioTimeShift ? 10000i64*s.tAudioTimeShift : 0);
		m_pASF->SetNormalizeBoost(s.fAudioNormalize, s.fAudioNormalizeRecover, s.AudioBoost);
	}

	s.fnChannels = m_nChannels;

	return __super::OnApply();
}

void CPPageAudioSwitcher::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

	if(lpnmlv->iItem > 0 && lpnmlv->iSubItem > 0 && lpnmlv->iSubItem <= m_nChannels)
	{
		UpdateData();
		m_pSpeakerToChannelMap[m_nChannels-1][lpnmlv->iItem-1] ^= 1<<(lpnmlv->iSubItem-1);
		m_list.RedrawItems(lpnmlv->iItem, lpnmlv->iItem);
		SetModified();

		if(GetKeyState(VK_SHIFT) & 0x8000)
		{
			OnApply();
		}
	}

	*pResult = 0;
}

void CPPageAudioSwitcher::OnEnChangeEdit1()
{
	if(IsWindow(m_list.m_hWnd))
	{
		UpdateData();
		m_list.Invalidate();
		SetModified();
	}
}

#include <math.h>

void CPPageAudioSwitcher::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if(nIDCtl != IDC_LIST1) return;

//	if(lpDrawItemStruct->itemID == 0)
//		UpdateData();

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	pDC->SetBkMode(TRANSPARENT);

	CPen p(PS_INSIDEFRAME, 1, 0xe0e0e0);
	CPen* old = pDC->SelectObject(&p);

	pDC->MoveTo(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.bottom-1);
	pDC->LineTo(lpDrawItemStruct->rcItem.right, lpDrawItemStruct->rcItem.bottom-1);

	CHeaderCtrl* pHeader = m_list.GetHeaderCtrl();
	int nColumnCount = pHeader->GetItemCount();

	for(int i = 0; i < nColumnCount; i++)
	{
		CRect r, rb;
		m_list.GetSubItemRect(lpDrawItemStruct->itemID, i, LVIR_BOUNDS, rb);
		m_list.GetSubItemRect(lpDrawItemStruct->itemID, i, LVIR_LABEL, r);

		pDC->MoveTo(r.right-1, r.top);
		pDC->LineTo(r.right-1, r.bottom-1);

		CSize s = pDC->GetTextExtent(m_list.GetItemText(lpDrawItemStruct->itemID, i));

		if(i == 0)
		{
			r.left = rb.left;

			if(lpDrawItemStruct->itemID == 0)
			{
				pDC->MoveTo(0, 0);
				pDC->LineTo(r.right, r.bottom-1);
			}
			else
			{
				pDC->SetTextColor(m_list.IsWindowEnabled() ? 0 : 0xb0b0b0);
				pDC->TextOut(r.left+1, (r.top+r.bottom-s.cy)/2, m_list.GetItemText(lpDrawItemStruct->itemID, i));
			}
		}
		else
		{
			pDC->SetTextColor(i > m_nChannels ? 0xe0e0e0 : (!m_list.IsWindowEnabled() ? 0xb0b0b0 : 0));

			if(lpDrawItemStruct->itemID == 0)
			{
				pDC->TextOut((r.left+r.right-s.cx)/2, (r.top+r.bottom-s.cy)/2, m_list.GetItemText(lpDrawItemStruct->itemID, i));
			}
			else
			{
				if(m_dwChannelMask & (1<<(lpDrawItemStruct->itemID-1)))
				{
					int nBitsSet = 0;

					for(int j = 1; j <= (1<<(lpDrawItemStruct->itemID-1)); j <<= 1)
					{
						if(m_dwChannelMask & j)
							nBitsSet++;
					}

					if(nBitsSet == i)
					{
						COLORREF tmp = pDC->GetTextColor();

						pDC->SetTextColor(0xe0e0e0);
						CFont f;
						f.CreatePointFont(MulDiv(100, 96, pDC->GetDeviceCaps(LOGPIXELSX)), _T("Marlett"));
						CFont* old = pDC->SelectObject(&f);
						s = pDC->GetTextExtent(_T("g"));
						pDC->TextOut((r.left+r.right-s.cx)/2, (r.top+r.bottom-s.cy)/2, _T("g"));

						pDC->SetTextColor(tmp);
					}
				}

				if(m_pSpeakerToChannelMap[m_nChannels-1][lpDrawItemStruct->itemID-1] & (1<<(i-1)))
				{
					CFont f;
					f.CreatePointFont(MulDiv(100, 96, pDC->GetDeviceCaps(LOGPIXELSX)), _T("Marlett"));
					CFont* old = pDC->SelectObject(&f);
					s = pDC->GetTextExtent(_T("a"));
					pDC->TextOut((r.left+r.right-s.cx)/2, (r.top+r.bottom-s.cy)/2, _T("a"));
					pDC->SelectObject(old);
				}
			}
		}
	}

	pDC->SelectObject(old);
}

void CPPageAudioSwitcher::OnUpdateAudioSwitcher(CCmdUI* pCmdUI)
{
//	UpdateData();
	pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK2)/*m_fEnableAudioSwitcher*/);
}

void CPPageAudioSwitcher::OnUpdateChannelMapping(CCmdUI* pCmdUI)
{
//	UpdateData();
	pCmdUI->Enable(IsDlgButtonChecked(IDC_CHECK2)/*m_fEnableAudioSwitcher*/ 
		&& IsDlgButtonChecked(IDC_CHECK1)/*m_fCustomChannelMapping*/);
}

void CPPageAudioSwitcher::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SetModified();

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}
