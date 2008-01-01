/* 
 * $Id: MPCVideoDecSettingsWnd.cpp 249 2007-09-26 11:07:22Z casimir666 $
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
#include "MPCVideoDecSettingsWnd.h"
#include "..\..\..\dsutil\dsutil.h"


// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))

//

#define LEFT_SPACING					25
#define VERTICAL_SPACING				25

//
// CMPCVideoDecSettingsWnd
//

int		g_AVDiscard[] =
{
    -16, ///< AVDISCARD_NONE    discard nothing
      0, ///< AVDISCARD_DEFAULT discard useless packets like 0 size packets in avi
      8, ///< AVDISCARD_NONREF  discard all non reference
     16, ///< AVDISCARD_BIDIR   discard all bidirectional frames
     32, ///< AVDISCARD_NONKEY  discard all frames except keyframes
     48, ///< AVDISCARD_ALL     discard all
};

int FindDiscardIndex(int nValue)
{
	for (int i=0; i<countof (g_AVDiscard); i++)
		if (g_AVDiscard[i] == nValue) return i;
	return 1;
}


CMPCVideoDecSettingsWnd::CMPCVideoDecSettingsWnd()
{
}

bool CMPCVideoDecSettingsWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pMDF);

	m_pMDF.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while(pos && !(m_pMDF = pUnks.GetNext(pos)));
	
	if(!m_pMDF) return false;

	return true;
}

void CMPCVideoDecSettingsWnd::OnDisconnect()
{
	m_pMDF.Release();
}

bool CMPCVideoDecSettingsWnd::OnActivate()
{
	DWORD	dwStyle = WS_VISIBLE|WS_CHILD|WS_BORDER;
	int		nPosY	= 10;

	m_grpFFMpeg.Create (_T("FFMpeg settings"), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10,  nPosY, 330, nPosY+150), this, IDC_STATIC);

	// Decoding frame number
	nPosY += VERTICAL_SPACING;
	m_txtThreadNumber.Create (_T("Decoding thread number"), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbThreadNumber.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 260, nPosY+8), this, IDC_PP_THREAD_NUMBER);
	m_cbThreadNumber.AddString (_T("1"));
	m_cbThreadNumber.AddString (_T("2"));
	m_cbThreadNumber.AddString (_T("3"));
	m_cbThreadNumber.AddString (_T("4"));
	m_cbThreadNumber.AddString (_T("5"));
	m_cbThreadNumber.AddString (_T("6"));

	// H264 deblocking mode
	nPosY += VERTICAL_SPACING;
	m_txtDiscardMode.Create (_T("H264 skip deblocking mode"), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbDiscardMode.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 315, nPosY+8), this, IDC_PP_DISCARD_MODE);
	m_cbDiscardMode.AddString (_T("None"));
	m_cbDiscardMode.AddString (_T("Default"));
	m_cbDiscardMode.AddString (_T("Non Reference"));
	m_cbDiscardMode.AddString (_T("Bidirectional"));
	m_cbDiscardMode.AddString (_T("Non keyframes"));
	m_cbDiscardMode.AddString (_T("All frames"));
	
	// Error resilience
	nPosY += VERTICAL_SPACING;
	m_txtErrorResilience.Create (_T("Error resilience"), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbErrorResilience.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 315, nPosY+8), this, IDC_PP_DISCARD_MODE);
	m_cbErrorResilience.AddString (_T("Careful"));
	m_cbErrorResilience.AddString (_T("Compliant"));
	m_cbErrorResilience.AddString (_T("Aggressive"));
	m_cbErrorResilience.AddString (_T("Very aggressive"));

	// IDCT Algo
	nPosY += VERTICAL_SPACING;
	m_txtIDCTAlgo.Create (_T("IDCT Algorithm"), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbIDCTAlgo.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 315, nPosY+8), this, IDC_PP_DISCARD_MODE);
	m_cbIDCTAlgo.AddString (_T("Auto"));
	m_cbIDCTAlgo.AddString (_T("Lib Mpeg2 MMX"));
	m_cbIDCTAlgo.AddString (_T("Simple MMX"));
	m_cbIDCTAlgo.AddString (_T("SKAL"));
	m_cbIDCTAlgo.AddString (_T("Simple"));


	nPosY = 170;
	m_grpDXVA.Create   (_T("DXVA settings"),   WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10, nPosY, 330, nPosY+110), this, IDC_STATIC);
	nPosY += VERTICAL_SPACING;

	// Enable DXVA
	m_chkEnableDXVA.Create (_T("Enable DXVA"), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_PP_ENABLE_DXVA);

	// Enable deblocking
	nPosY += VERTICAL_SPACING;
	m_chkEnableDeblocking.Create (_T("Enable deblocking"), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|WS_DISABLED, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_PP_ENABLE_DEBLOCKING);


	for(CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow())
		pWnd->SetFont(&m_font, FALSE);

	if (m_pMDF)
	{
		m_cbThreadNumber.SetCurSel		(m_pMDF->GetThreadNumber() - 1);
		m_chkEnableDXVA.SetCheck		(m_pMDF->GetEnableDXVA());
		m_cbDiscardMode.SetCurSel		(FindDiscardIndex (m_pMDF->GetDiscardMode()));
		m_cbErrorResilience.SetCurSel	(m_pMDF->GetErrorResilience()-1);
		m_cbIDCTAlgo.SetCurSel			(m_pMDF->GetIDCTAlgo());
	}

	return true;
}

void CMPCVideoDecSettingsWnd::OnDeactivate()
{
}

bool CMPCVideoDecSettingsWnd::OnApply()
{
	OnDeactivate();

	if(m_pMDF)
	{
		m_pMDF->SetThreadNumber		(m_cbThreadNumber.GetCurSel() + 1);
		m_pMDF->SetEnableDXVA		(!!m_chkEnableDXVA.GetCheck());
		m_pMDF->SetDiscardMode		(g_AVDiscard[m_cbDiscardMode.GetCurSel()]);
		m_pMDF->SetErrorResilience  (m_cbErrorResilience.GetCurSel()+1);
		m_pMDF->SetIDCTAlgo			(m_cbIDCTAlgo.GetCurSel());
	}

	return true;
}



BEGIN_MESSAGE_MAP(CMPCVideoDecSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
