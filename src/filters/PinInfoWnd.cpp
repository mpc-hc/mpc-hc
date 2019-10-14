/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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
#include "PinInfoWnd.h"
#include "../DSUtil/DSUtil.h"
#include "../mpc-hc/DpiHelper.h"
#include "../DSUtil/MediaTypeEx.h"

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
    while (pos && !(m_pBF = pUnks.GetNext(pos))) {
        ;
    }

    if (!m_pBF) {
        return false;
    }

    return true;
}

void CPinInfoWnd::OnDisconnect()
{
    m_pBF.Release();
}

static WNDPROC OldControlProc;
static LRESULT CALLBACK ControlProc(HWND control, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_KEYDOWN) {
        if (LOWORD(wParam) == VK_ESCAPE) {
            return 0;    // just ignore ESCAPE in edit control
        }
        if ((LOWORD(wParam) == 'A' || LOWORD(wParam) == 'a')
                && (GetKeyState(VK_CONTROL) < 0)) {
            CEdit* pEdit = (CEdit*)CWnd::FromHandle(control);
            pEdit->SetSel(0, pEdit->GetWindowTextLength(), TRUE);
            return 0;
        }
    }

    return CallWindowProc(OldControlProc, control, message, wParam, lParam); // call edit control's own windowproc
}

bool CPinInfoWnd::OnActivate()
{
    m_dpi.Override(GetSafeHwnd());

    DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
    CPoint p(m_dpi.ScaleX(10), m_dpi.ScaleY(10));

    m_pin_static.Create(_T("Pin:"), dwStyle,
                        CRect(p + CPoint(m_dpi.ScaleX(0), m_dpi.ScaleY(3)), CSize(m_dpi.ScaleX(30), m_fontheight)),
                        this);
    m_pin_combo.Create(dwStyle | CBS_DROPDOWNLIST,
                       CRect(p + CPoint(m_dpi.ScaleX(30), m_dpi.ScaleY(0)), CSize(m_dpi.ScaleX(520), m_dpi.ScaleY(200))),
                       this, IDC_PP_COMBO1);
    BeginEnumPins(m_pBF, pEP, pPin) {
        CPinInfo pi;
        if (FAILED(pPin->QueryPinInfo(&pi))) {
            continue;
        }
        CString str(pi.achName);
        if (!str.Find(_T("Apple"))) {
            str.Delete(0, 1);
        }
        CString dir = _T("[?] ");
        if (pi.dir == PINDIR_INPUT) {
            dir = _T("[IN] ");
        } else if (pi.dir == PINDIR_OUTPUT) {
            dir = _T("[OUT] ");
        }
        m_pin_combo.SetItemDataPtr(m_pin_combo.AddString(dir + str), pPin);
    }
    EndEnumPins;
    m_pin_combo.SetCurSel(0);

    p.y += m_fontheight + m_dpi.ScaleY(20);

    m_info_edit.CreateEx(WS_EX_CLIENTEDGE,
                         _T("EDIT"),
                         _T(""),
                         dwStyle |
                        /* WS_BORDER | */ // equivalent to WS_EX_CLIENTEDGE 
                         WS_VSCROLL |
                         WS_HSCROLL |
                         ES_MULTILINE |
                         ES_READONLY,
                         CRect(p, CSize(m_dpi.ScaleX(550), m_fontheight * 30)),
                         this,
                         IDC_PP_EDIT1);
    m_info_edit.SetLimitText(60000);

    OnSelectedPinChange();

    for (CWnd* pWnd = GetWindow(GW_CHILD); pWnd; pWnd = pWnd->GetNextWindow()) {
        pWnd->SetFont(&m_font, FALSE);
    }

    m_info_edit.SetFixedWidthFont(m_monospacefont);

    // subclass the edit control
    OldControlProc = (WNDPROC)SetWindowLongPtr(m_info_edit.m_hWnd, GWLP_WNDPROC, (LONG_PTR)ControlProc);

    return true;
}

void CPinInfoWnd::OnDeactivate()
{
}

bool CPinInfoWnd::OnApply()
{
    OnDeactivate();

    if (m_pBF) {
    }

    return true;
}

BOOL CPinInfoWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    //we don't need this anymore, as we bypass the CInternalPropertyPageWnd which is what sets it dirty
    //SetDirty(false); 
    //we call CWnd implementation because CInternalPropertyPageWnd will set it right back to dirty on a scroll/command message
    return CWnd::OnWndMsg(message, wParam, lParam, pResult);
}

BEGIN_MESSAGE_MAP(CPinInfoWnd, CInternalPropertyPageWnd)
    ON_CBN_SELCHANGE(IDC_PP_COMBO1, OnSelectedPinChange)
    ON_WM_CTLCOLOR()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CPinInfoWnd::AddLine(CString str)
{
    str.Replace(_T("\n"), _T("\r\n"));
    int len = m_info_edit.GetWindowTextLength();
    m_info_edit.SetSel(len, len, TRUE);
    m_info_edit.ReplaceSel(str);
}

void CPinInfoWnd::OnSelectedPinChange()
{
    m_info_edit.SetWindowText(_T(""));

    int i = m_pin_combo.GetCurSel();
    if (i < 0) {
        return;
    }

    CComPtr<IPin> pPin = (IPin*)m_pin_combo.GetItemDataPtr(i);
    if (!pPin) {
        return;
    }

    CString str;
    CPinInfo pinInfo;

    if (SUCCEEDED(pPin->QueryPinInfo(&pinInfo))) {
        CString strName;
        CLSID   filterClsid;
        CFilterInfo filterInfo;

        if (SUCCEEDED(pinInfo.pFilter->QueryFilterInfo(&filterInfo))) {
            CRegKey key;
            pinInfo.pFilter->GetClassID(&filterClsid);
            TCHAR buff[128];
            ULONG len = _countof(buff);
            if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("CLSID\\{083863F1-70DE-11D0-BD40-00A0C911CE86}\\Instance\\") + CStringFromGUID(filterClsid), KEY_READ)
                    && ERROR_SUCCESS == key.QueryStringValue(_T("FriendlyName"), buff, &len)) {
                strName = CString(buff);
            } else {
                strName = filterInfo.achName;
            }
            str.Format(_T("Filter : %s - CLSID : %s\n\n"), strName.GetString(), CStringFromGUID(filterClsid).GetString());
            AddLine(str);
        }
    }

    CMediaTypeEx cmt;

    CComPtr<IPin> pPinTo;
    if (SUCCEEDED(pPin->ConnectedTo(&pPinTo)) && pPinTo) {
        str.Format(_T("- Connected to:\n\nCLSID: %s\nFilter: %s\nPin: %s\n\n"),
                   CStringFromGUID(GetCLSID(pPinTo)).GetString(),
                   static_cast<LPCTSTR>(GetFilterName(GetFilterFromPin(pPinTo))),
                   static_cast<LPCTSTR>(GetPinName(pPinTo)));

        AddLine(str);

        AddLine(_T("- Connection media type:\n\n"));

        if (SUCCEEDED(pPin->ConnectionMediaType(&cmt))) {
            CAtlList<CString> sl;
            cmt.Dump(sl);
            POSITION pos = sl.GetHeadPosition();
            while (pos) {
                AddLine(sl.GetNext(pos) + '\n');
            }
        }
    } else {
        str = _T("- Not connected\n\n");
    }

    int iMT = 0;

    BeginEnumMediaTypes(pPin, pEMT, pmt) {
        CMediaTypeEx mt(*pmt);

        str.Format(_T("- Enumerated media type %d:\n\n"), iMT++);
        AddLine(str);

        if (cmt.majortype != GUID_NULL && mt == cmt) {
            AddLine(_T("Set as the current media type\n\n"));
        } else {
            CAtlList<CString> sl;
            mt.Dump(sl);
            POSITION pos = sl.GetHeadPosition();
            while (pos) {
                AddLine(sl.GetNext(pos) + '\n');
            }
        }
    }
    EndEnumMediaTypes(pmt);

    m_info_edit.SetSel(0, 0);
}


HBRUSH CPinInfoWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH ret;
    ret = getCtlColor(pDC, pWnd, nCtlColor);
    if (nullptr != ret) {
        return ret;
    } else {
        return __super::OnCtlColor(pDC, pWnd, nCtlColor);
    }
}

BOOL CPinInfoWnd::OnEraseBkgnd(CDC* pDC) {
    bool ret = MPCThemeEraseBkgnd(pDC, this, CTLCOLOR_DLG);
    if (ret) {
        return ret;
    } else {
        return __super::OnEraseBkgnd(pDC);
    }
}
