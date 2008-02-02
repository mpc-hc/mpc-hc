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
#include "MPCVideoDecSettingsWnd.h"
#include "..\..\..\dsutil\dsutil.h"


// ==>>> Resource identifier from "resource.h" present in mplayerc project!
#define ResStr(id) CString(MAKEINTRESOURCE(id))
#define IDS_VDF_FFSETTINGS				33167
#define IDS_VDF_THREADNUMBER			33168
#define IDS_VDF_SKIPDEBLOCK				33169
#define IDS_VDF_DBLK_NONE				33170
#define IDS_VDF_DBLK_DEFAULT			33171
#define IDS_VDF_DBLK_NONREF				33172
#define IDS_VDF_DBLK_BIDIR				33173
#define IDS_VDF_DBLK_NONKFRM			33174
#define IDS_VDF_DBLK_ALL				33175
#define IDS_VDF_ERROR_RESILIENCE		33176
#define IDS_VDF_ERR_CAREFUL				33177
#define IDS_VDF_ERR_COMPLIANT			33178
#define IDS_VDF_ERR_AGGRESSIVE			33179
#define IDS_VDF_ERR_VERYAGGRESSIVE		33180
#define IDS_VDF_IDCT_ALGO				33181
#define IDS_VDF_IDCT_AUTO				33182
#define IDS_VDF_IDCT_AUTO				33182
#define IDS_VDF_IDCT_LIBMPG2			33183
#define IDS_VDF_IDCT_SIMPLE_MMX			33184
#define IDS_VDF_IDCT_SKAL				33185
#define IDS_VDF_IDCT_SIMPLE				33186
#define IDS_VDF_DXVA_SETTING			33187
#define IDS_VDF_DXVA_ENABLE				33188
#define IDS_VDF_DXVA_MODE				33189

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

	m_grpFFMpeg.Create (ResStr (IDS_VDF_FFSETTINGS), WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10,  nPosY, 330, nPosY+150), this, IDC_STATIC);

	// Decoding frame number
	nPosY += VERTICAL_SPACING;
	m_txtThreadNumber.Create (ResStr (IDS_VDF_THREADNUMBER), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbThreadNumber.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 260, nPosY+8), this, IDC_PP_THREAD_NUMBER);
	m_cbThreadNumber.AddString (_T("1"));
	m_cbThreadNumber.AddString (_T("2"));
	m_cbThreadNumber.AddString (_T("3"));
	m_cbThreadNumber.AddString (_T("4"));
	m_cbThreadNumber.AddString (_T("5"));
	m_cbThreadNumber.AddString (_T("6"));

	// H264 deblocking mode
	nPosY += VERTICAL_SPACING;
	m_txtDiscardMode.Create (ResStr (IDS_VDF_SKIPDEBLOCK), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbDiscardMode.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 315, nPosY+8), this, IDC_PP_DISCARD_MODE);
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_NONE));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_DEFAULT));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_NONREF));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_BIDIR));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_NONKFRM));
	m_cbDiscardMode.AddString (ResStr (IDS_VDF_DBLK_ALL));
	
	// Error resilience
	nPosY += VERTICAL_SPACING;
	m_txtErrorResilience.Create (ResStr (IDS_VDF_ERROR_RESILIENCE), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbErrorResilience.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 315, nPosY+8), this, IDC_PP_DISCARD_MODE);
	m_cbErrorResilience.AddString (ResStr (IDS_VDF_ERR_CAREFUL));
	m_cbErrorResilience.AddString (ResStr (IDS_VDF_ERR_COMPLIANT));
	m_cbErrorResilience.AddString (ResStr (IDS_VDF_ERR_AGGRESSIVE));
	m_cbErrorResilience.AddString (ResStr (IDS_VDF_ERR_VERYAGGRESSIVE));

	// IDCT Algo
	nPosY += VERTICAL_SPACING;
	m_txtIDCTAlgo.Create (ResStr (IDS_VDF_IDCT_ALGO), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_cbIDCTAlgo.Create  (WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL, CRect (200,  nPosY-4, 315, nPosY+8), this, IDC_PP_DISCARD_MODE);
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_AUTO));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_LIBMPG2));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_SIMPLE));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_SKAL));
	m_cbIDCTAlgo.AddString (ResStr (IDS_VDF_IDCT_SIMPLE));


	nPosY = 170;
	m_grpDXVA.Create   (ResStr (IDS_VDF_DXVA_SETTING),   WS_VISIBLE|WS_CHILD | BS_GROUPBOX, CRect (10, nPosY, 330, nPosY+110), this, IDC_STATIC);
	nPosY += VERTICAL_SPACING;

	// Enable DXVA
	m_chkEnableDXVA.Create (ResStr (IDS_VDF_DXVA_ENABLE), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_PP_ENABLE_DXVA);

	// DXVA mode
	nPosY += VERTICAL_SPACING;
//	m_txtDXVAMode.Create (ResStr (IDS_VDF_DXVA_MODE), WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX|WS_DISABLED, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_PP_ENABLE_DEBLOCKING);
	m_txtDXVAMode.Create (ResStr (IDS_VDF_DXVA_MODE), WS_VISIBLE|WS_CHILD, CRect (LEFT_SPACING,  nPosY, 190, nPosY+15), this, IDC_STATIC);
	m_edtDXVAMode.Create (WS_CHILD|WS_VISIBLE|WS_DISABLED|WS_BORDER, CRect (140,  nPosY, 315, nPosY+20), this, 0);
	m_edtDXVAMode.SetWindowText (GetDXVAMode (m_pMDF->GetDXVADecoderGuid()));


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

		m_pMDF->Apply();
	}

	return true;
}



BEGIN_MESSAGE_MAP(CMPCVideoDecSettingsWnd, CInternalPropertyPageWnd)
END_MESSAGE_MAP()
