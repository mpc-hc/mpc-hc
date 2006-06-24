/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
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
#include "PinInfoWnd.h"
#include "..\dsutil\dsutil.h"

//
// CPinInfoWnd
//

CPinInfoWnd::CPinInfoWnd()
{
}

bool CPinInfoWnd::OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
	ASSERT(!m_pBF);

	m_pBF.Release();

	POSITION pos = pUnks.GetHeadPosition();
	while(pos && !(m_pBF = pUnks.GetNext(pos)));
	
	if(!m_pBF) return false;

	return true;
}

void CPinInfoWnd::OnDisconnect()
{
	m_pBF.Release();
}

bool CPinInfoWnd::OnActivate()
{
	DWORD dwStyle = WS_VISIBLE|WS_CHILD|WS_TABSTOP;

	CPoint p(10, 10);

	m_pin_static.Create(_T("Pin:"), dwStyle, CRect(p + CPoint(0, 3), CSize(30, m_fontheight)), this);
	m_pin_combo.Create(dwStyle|CBS_DROPDOWNLIST, CRect(p + CPoint(30, 0), CSize(450, 200)), this, IDC_PP_COMBO1);
	BeginEnumPins(m_pBF, pEP, pPin)
	{
		CPinInfo pi;
		if(FAILED(pPin->QueryPinInfo(&pi))) continue;
		CString str = CString(pi.achName);
		CString dir = _T("[?] ");
		if(pi.dir == PINDIR_INPUT) dir = _T("[IN] ");
		else if(pi.dir == PINDIR_OUTPUT) dir = _T("[OUT] ");
		m_pin_combo.SetItemDataPtr(m_pin_combo.AddString(dir + str), pPin);
	}
	EndEnumPins
	m_pin_combo.SetCurSel(0);

	p.y += m_fontheight + 20;

	m_info_edit.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), dwStyle|WS_BORDER|WS_VSCROLL|WS_HSCROLL|ES_MULTILINE|ES_AUTOHSCROLL|ES_READONLY, CRect(p, CSize(480, m_fontheight*20)), this, IDC_PP_EDIT1);
	m_info_edit.SetLimitText(60000);

	OnCbnSelchangeCombo1();

	for(CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow())
		pWnd->SetFont(&m_font, FALSE);

	m_info_edit.SetFont(&m_monospacefont);

	return true;
}

void CPinInfoWnd::OnDeactivate()
{
}

bool CPinInfoWnd::OnApply()
{
	OnDeactivate();

	if(m_pBF)
	{
	}

	return true;
}

BEGIN_MESSAGE_MAP(CPinInfoWnd, CInternalPropertyPageWnd)
	ON_CBN_SELCHANGE(IDC_PP_COMBO1, OnCbnSelchangeCombo1)
END_MESSAGE_MAP()

void CPinInfoWnd::AddLine(CString str)
{
	str.Replace(_T("\n"), _T("\r\n"));
	int len = m_info_edit.GetWindowTextLength();
	m_info_edit.SetSel(len, len, TRUE);
	m_info_edit.ReplaceSel(str);
}

void CPinInfoWnd::OnCbnSelchangeCombo1()
{
	m_info_edit.SetWindowText(_T(""));

	int i = m_pin_combo.GetCurSel();
	if(i < 0) return;

	CComPtr<IPin> pPin = (IPin*)m_pin_combo.GetItemDataPtr(i);
	if(!pPin) return;

	CString str;

	CMediaTypeEx cmt;

	CComPtr<IPin> pPinTo;
	if(SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo)
	{
		str.Format(_T("- Connected to:\n\nCLSID: %s\nFilter: %s\nPin: %s\n\n"), 
			CString(CStringFromGUID(GetCLSID(pPinTo))),
			CString(GetFilterName(GetFilterFromPin(pPinTo))), 
			CString(GetPinName(pPinTo)));

		AddLine(str);

		AddLine(_T("- Connection media type:\n\n"));

		if(SUCCEEDED(pPin->ConnectionMediaType(&cmt)))
		{
			CAtlList<CString> sl;
			cmt.Dump(sl);
			POSITION pos = sl.GetHeadPosition();
			while(pos) AddLine(sl.GetNext(pos) + '\n');
		}
	}
	else
	{
		str = _T("- Not connected\n\n");
	}

	int iMT = 0;

	BeginEnumMediaTypes(pPin, pEMT, pmt)
	{
		CMediaTypeEx mt(*pmt);

		str.Format(_T("- Enumerated media type %d:\n\n"), iMT++);
		AddLine(str);

		if(cmt.majortype != GUID_NULL && mt == cmt)
		{
			AddLine(_T("Set as the current media type\n\n"));
		}
		else
		{
			CAtlList<CString> sl;
			mt.Dump(sl);
			POSITION pos = sl.GetHeadPosition();
			while(pos) AddLine(sl.GetNext(pos) + '\n');
		}
	}
	EndEnumMediaTypes(pmt)

	m_info_edit.SetSel(0, 0);
}