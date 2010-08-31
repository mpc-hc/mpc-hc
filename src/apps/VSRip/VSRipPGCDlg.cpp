/*
 * $Id$
 *
 * (C) 2003-2005 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of vsrip.
 *
 * Vsrip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vsrip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <atlcoll.h>
#include "VSRip.h"
#include "VSRipPGCDlg.h"
#include "../../Subtitles/VobSubFile.h"


// CVSRipPGCDlg dialog

IMPLEMENT_DYNAMIC(CVSRipPGCDlg, CVSRipPage)
CVSRipPGCDlg::CVSRipPGCDlg(IVSFRipper* pVSFRipper, CWnd* pParent /*=NULL*/)
	: CVSRipPage(pVSFRipper, CVSRipPGCDlg::IDD, pParent)
	, m_bResetTime(TRUE)
	, m_bClosedCaption(FALSE)
	, m_bForcedOnly(FALSE)
{
	m_rd.Reset();
}

CVSRipPGCDlg::~CVSRipPGCDlg()
{
}

void CVSRipPGCDlg::DoDataExchange(CDataExchange* pDX)
{
	CVSRipPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_pgclist);
	DDX_Control(pDX, IDC_LIST2, m_anglelist);
	DDX_Control(pDX, IDC_LIST3, m_vclist);
	DDX_Control(pDX, IDC_LIST4, m_langlist);
	DDX_Check(pDX, IDC_CHECK1, m_bResetTime);
	DDX_Check(pDX, IDC_CHECK2, m_bClosedCaption);
	DDX_Check(pDX, IDC_CHECK3, m_bForcedOnly);
}

void CVSRipPGCDlg::OnPrev()
{
	OnNext();
}

void CVSRipPGCDlg::OnNext()
{
	CAutoVectorPtr<int> items;

	m_rd.iSelPGC = m_pgclist.GetCurSel();

	m_rd.selids.RemoveAll();
	if(items.Allocate(m_langlist.GetSelCount()))
	{
		int j = m_langlist.GetSelItems(m_langlist.GetSelCount(), items);
		for(int i = 0; i < j; i++)
			m_rd.selids[(BYTE)m_langlist.GetItemData(items[i])] = true;
		items.Free();
	}

	m_rd.pgcs[m_rd.iSelPGC].iSelAngle = m_anglelist.GetCurSel();

	m_rd.selvcs.RemoveAll();
	if(items.Allocate(m_vclist.GetSelCount()))
	{
		int j = m_vclist.GetSelItems(m_vclist.GetSelCount(), items);
		for(int i = 0; i < j; i++)
			m_rd.selvcs.Add((DWORD)m_vclist.GetItemData(items[i]));
		items.Free();
	}

	m_rd.fClosedCaption = !!m_bClosedCaption;
	m_rd.fResetTime = !!m_bResetTime;
	m_rd.fForcedOnly = !!m_bForcedOnly;

	m_pVSFRipper->UpdateRipperData(m_rd);
}

bool CVSRipPGCDlg::CanGoNext()
{
	UpdateData();

	return(m_pgclist.GetCurSel() >= 0
		   && m_anglelist.GetCurSel() >= 0
		   && m_vclist.GetSelCount() > 0
		   && (m_langlist.GetSelCount() > 0 || m_bClosedCaption));
}

void CVSRipPGCDlg::SetupPGCList()
{
	ASSERT(m_rd.iSelPGC >= 0);

	m_pgclist.ResetContent();

	for(int i = 0; i < (int)m_rd.pgcs.GetCount(); i++)
	{
		CString str;
		str.Format(_T("PGC %d"), i+1);
		m_pgclist.AddString(str);
	}

	m_pgclist.SetCurSel(m_rd.iSelPGC);

	SetupLangList();
	SetupAngleList();
}

void CVSRipPGCDlg::SetupLangList()
{
	m_langlist.ResetContent();

	for(BYTE i = 0; i < 32; i++)
	{
		WORD id = m_rd.pgcs[m_rd.iSelPGC].ids[i];

		CString str;

		if(id == 0)
		{
			str.Format(_T("%02d (empty)"), (int)i);
		}
		else if(!isalpha(id>>8) || !isalpha(id&0xff))
		{
			str.Format(_T("%02d (unknown)"), (int)i);
		}
		else
		{
			str.Format(_T("%02d %s (%c%c)"), (int)i, FindLangFromId(id), TCHAR(id>>8), TCHAR(id&0xff));
		}

		int j = m_langlist.AddString(str);
		m_langlist.SetSel(j, !!id);
		m_langlist.SetItemData(j, (DWORD_PTR)i);
	}

	m_langlist.SetTopIndex(0);
}

void CVSRipPGCDlg::SetupAngleList()
{
	m_anglelist.ResetContent();

	m_rd.pgcs[m_rd.iSelPGC].iSelAngle = m_rd.pgcs[m_rd.iSelPGC].nAngles > 0 ? 1 : 0;

	for(int i = 0; i < 10; i++)
	{
		CString str;

		if(i == 0)
		{
			str = _T("Everything");
		}
		else
		{
			str.Format(_T("Angle %d"), i);
			if(i > m_rd.pgcs[m_rd.iSelPGC].nAngles)
				str += _T(" (empty)");
		}

		m_anglelist.AddString(str);
	}

	m_anglelist.SetCurSel(m_rd.pgcs[m_rd.iSelPGC].iSelAngle);

	SetupVCList();
}

void CVSRipPGCDlg::SetupVCList()
{
	m_vclist.ResetContent();

	CAtlArray<vc_t>& vca =( m_rd.pgcs[m_rd.iSelPGC].angles[m_rd.pgcs[m_rd.iSelPGC].iSelAngle]);

	for(int i = 0; i < (int)vca.GetCount(); i++)
	{
		CString str;
		str.Format(_T("V%02d C%02d"), vca[i].vob, vca[i].cell);

		DWORD vc = (vca[i].vob<<16)|vca[i].cell;

		int j = m_vclist.AddString(str);
		m_vclist.SetSel(j, TRUE);
		m_vclist.SetItemData(j, (DWORD_PTR)vc);
	}

	m_vclist.SetTopIndex(0);
}

BEGIN_MESSAGE_MAP(CVSRipPGCDlg, CVSRipPage)
	ON_LBN_SELCHANGE(IDC_LIST1, OnLbnSelchangeList1)
	ON_LBN_SELCHANGE(IDC_LIST2, OnLbnSelchangeList2)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CVSRipPGCDlg message handlers

void CVSRipPGCDlg::OnLbnSelchangeList1()
{
	if(m_rd.iSelPGC == m_pgclist.GetCurSel()) return;
	m_rd.iSelPGC = m_pgclist.GetCurSel();
	SetupAngleList();
}

void CVSRipPGCDlg::OnLbnSelchangeList2()
{
	if(m_rd.pgcs[m_rd.iSelPGC].iSelAngle == m_anglelist.GetCurSel()) return;
	m_rd.pgcs[m_rd.iSelPGC].iSelAngle = m_anglelist.GetCurSel();
	SetupVCList();
}

void CVSRipPGCDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CVSRipPage::OnShowWindow(bShow, nStatus);

	if(!bShow) return;

	m_pVSFRipper->GetRipperData(m_rd);

	if(m_rd.iSelPGC == -1)
	{
		m_rd.iSelPGC = 0;
		SetupPGCList();

		m_bClosedCaption = m_rd.vidinfo.line21_1 || m_rd.vidinfo.line21_2;
		UpdateData(FALSE);
	}
}
