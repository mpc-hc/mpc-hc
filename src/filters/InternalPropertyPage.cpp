/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include "InternalPropertyPage.h"
#include "../DSUtil/DSUtil.h"

//
// CInternalPropertyPageWnd
//

CInternalPropertyPageWnd::CInternalPropertyPageWnd()
    : m_fDirty(false)
    , m_fontheight(IPP_FONTSIZE)
{
}

BOOL CInternalPropertyPageWnd::Create(IPropertyPageSite* pPageSite, LPCRECT pRect, CWnd* pParentWnd)
{
    if (!pPageSite || !pRect) {
        return FALSE;
    }

    m_pPageSite = pPageSite;

    if (!m_font.m_hObject) {
        CString face;
        WORD height;
        extern BOOL AFXAPI AfxGetPropSheetFont(CString & strFace, WORD & wSize, BOOL bWizard); // yay
        if (!AfxGetPropSheetFont(face, height, FALSE)) {
            return FALSE;
        }

        LOGFONT lf;
        ZeroMemory(&lf, sizeof(lf));
        _tcscpy_s(lf.lfFaceName, face);
        HDC hDC = ::GetDC(nullptr);
        lf.lfHeight = -MulDiv(height, GetDeviceCaps(hDC, LOGPIXELSY), 72);
        ::ReleaseDC(nullptr, hDC);
        lf.lfWeight = FW_NORMAL;
        lf.lfCharSet = DEFAULT_CHARSET;
        if (!m_font.CreateFontIndirect(&lf)) {
            return FALSE;
        }

        lf.lfHeight -= -1;
        _tcscpy_s(lf.lfFaceName, _T("Lucida Console"));
        if (!m_monospacefont.CreateFontIndirect(&lf)) {
            _tcscpy_s(lf.lfFaceName, _T("Courier New"));
            if (!m_monospacefont.CreateFontIndirect(&lf)) {
                return FALSE;
            }
        }

        hDC = ::GetDC(nullptr);
        HFONT hFontOld = (HFONT)::SelectObject(hDC, m_font.m_hObject);
        CSize size;
        ::GetTextExtentPoint32(hDC, _T("x"), 1, &size);
        ::SelectObject(hDC, hFontOld);
        ::ReleaseDC(nullptr, hDC);

        m_fontheight = size.cy;
    }

    LPCTSTR wc = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS, 0, (HBRUSH)(COLOR_BTNFACE + 1));
    if (!CreateEx(0, wc, _T("CInternalPropertyPageWnd"), WS_CHILDWINDOW, *pRect, pParentWnd, 0)) {
        return FALSE;
    }

    SetFont(&m_font);

    return TRUE;
}

BOOL CInternalPropertyPageWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    if (message == WM_COMMAND || message == WM_HSCROLL || message == WM_VSCROLL) {
        SetDirty(true);
    }

    return __super::OnWndMsg(message, wParam, lParam, pResult);
}

BEGIN_MESSAGE_MAP(CInternalPropertyPageWnd, CWnd)
END_MESSAGE_MAP()

//
// CInternalPropertyPage
//

CInternalPropertyPage::CInternalPropertyPage(LPUNKNOWN lpunk, HRESULT* phr)
    : CUnknown(_T("CInternalPropertyPage"), lpunk)
    , m_pWnd(nullptr)
{
    if (phr) {
        *phr = S_OK;
    }
}

CInternalPropertyPage::~CInternalPropertyPage()
{
    if (m_pWnd) {
        if (m_pWnd->m_hWnd) {
            ASSERT(0);
            m_pWnd->DestroyWindow();
        }
        delete m_pWnd;
        m_pWnd = nullptr;
    }
}

STDMETHODIMP CInternalPropertyPage::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI2(IPropertyPage)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IPropertyPage

STDMETHODIMP CInternalPropertyPage::SetPageSite(IPropertyPageSite* pPageSite)
{
    CAutoLock cAutoLock(this);

    if (pPageSite && m_pPageSite || !pPageSite && !m_pPageSite) {
        return E_UNEXPECTED;
    }

    m_pPageSite = pPageSite;

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::Activate(HWND hwndParent, LPCRECT pRect, BOOL fModal)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(this);

    CheckPointer(pRect, E_POINTER);

    if (!m_pWnd || m_pWnd->m_hWnd || m_pUnks.IsEmpty()) {
        return E_UNEXPECTED;
    }

    if (!m_pWnd->Create(m_pPageSite, pRect, CWnd::FromHandle(hwndParent))) {
        return E_OUTOFMEMORY;
    }

    if (!m_pWnd->OnActivate()) {
        m_pWnd->DestroyWindow();
        return E_FAIL;
    }

    m_pWnd->ModifyStyleEx(WS_EX_DLGMODALFRAME, WS_EX_CONTROLPARENT);
    m_pWnd->ShowWindow(SW_SHOWNORMAL);

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::Deactivate()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(this);

    if (!m_pWnd || !m_pWnd->m_hWnd) {
        return E_UNEXPECTED;
    }

    m_pWnd->OnDeactivate();

    m_pWnd->ModifyStyleEx(WS_EX_CONTROLPARENT, 0);
    m_pWnd->DestroyWindow();

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::GetPageInfo(PROPPAGEINFO* pPageInfo)
{
    CAutoLock cAutoLock(this);

    CheckPointer(pPageInfo, E_POINTER);

    LPOLESTR pszTitle;
    HRESULT hr = AMGetWideString(CStringW(GetWindowTitle()), &pszTitle);
    if (FAILED(hr)) {
        return hr;
    }

    pPageInfo->cb = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle = pszTitle;
    pPageInfo->pszDocString = nullptr;
    pPageInfo->pszHelpFile = nullptr;
    pPageInfo->dwHelpContext = 0;
    pPageInfo->size = GetWindowSize();

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::SetObjects(ULONG cObjects, LPUNKNOWN* ppUnk)
{
    CAutoLock cAutoLock(this);

    if (cObjects && m_pWnd || !cObjects && !m_pWnd) {
        return E_UNEXPECTED;
    }

    m_pUnks.RemoveAll();

    if (cObjects > 0) {
        CheckPointer(ppUnk, E_POINTER);

        for (ULONG i = 0; i < cObjects; i++) {
            m_pUnks.AddTail(ppUnk[i]);
        }

        m_pWnd = GetWindow();
        if (!m_pWnd) {
            return E_OUTOFMEMORY;
        }

        if (!m_pWnd->OnConnect(m_pUnks)) {
            delete m_pWnd;
            m_pWnd = nullptr;

            return E_FAIL;
        }
    } else {
        m_pWnd->OnDisconnect();

        m_pWnd->DestroyWindow();
        delete m_pWnd;
        m_pWnd = nullptr;
    }

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::Show(UINT nCmdShow)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(this);

    if (!m_pWnd) {
        return E_UNEXPECTED;
    }

    if ((nCmdShow != SW_SHOW) && (nCmdShow != SW_SHOWNORMAL) && (nCmdShow != SW_HIDE)) {
        return E_INVALIDARG;
    }

    m_pWnd->ShowWindow(nCmdShow);
    m_pWnd->Invalidate();

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::Move(LPCRECT pRect)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(this);

    CheckPointer(pRect, E_POINTER);

    if (!m_pWnd) {
        return E_UNEXPECTED;
    }

    m_pWnd->MoveWindow(pRect, TRUE);

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::IsPageDirty()
{
    CAutoLock cAutoLock(this);

    return m_pWnd && m_pWnd->GetDirty() ? S_OK : S_FALSE;
}

STDMETHODIMP CInternalPropertyPage::Apply()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(this);

    if (!m_pWnd || m_pUnks.IsEmpty() || !m_pPageSite) {
        return E_UNEXPECTED;
    }

    if (m_pWnd->GetDirty() && m_pWnd->OnApply()) {
        m_pWnd->SetDirty(false);
    }

    return S_OK;
}

STDMETHODIMP CInternalPropertyPage::Help(LPCWSTR lpszHelpDir)
{
    CAutoLock cAutoLock(this);

    return E_NOTIMPL;
}

STDMETHODIMP CInternalPropertyPage::TranslateAccelerator(LPMSG lpMsg)
{
    CAutoLock cAutoLock(this);

    return E_NOTIMPL;
}
