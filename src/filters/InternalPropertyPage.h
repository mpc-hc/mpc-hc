/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2017 see Authors.txt
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

#pragma once

#include <atlcoll.h>

#define IPP_FONTSIZE 13
#define IPP_SCALE(size) ((size) * m_fontheight / IPP_FONTSIZE)

interface __declspec(uuid("03481710-D73E-4674-839F-03EDE2D60ED8"))
    ISpecifyPropertyPages2 :
    public ISpecifyPropertyPages
{
    STDMETHOD(CreatePage)(const GUID& guid, IPropertyPage** ppPage) = 0;
};

class CInternalPropertyPageWnd : public CWnd
{
    bool m_fDirty;
    CComPtr<IPropertyPageSite> m_pPageSite;

protected:
    CFont m_font, m_monospacefont;
    int m_fontheight;

    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

public:
    CInternalPropertyPageWnd();

    void SetDirty(bool fDirty = true) {
        m_fDirty = fDirty;
        if (m_pPageSite) {
            if (fDirty) {
                m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
            } else {
                m_pPageSite->OnStatusChange(PROPPAGESTATUS_CLEAN);
            }
        }
    }
    bool GetDirty() { return m_fDirty; }

    virtual BOOL Create(IPropertyPageSite* pPageSite, LPCRECT pRect, CWnd* pParentWnd);

    virtual bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks) { return true; }
    virtual void OnDisconnect() {}
    virtual bool OnActivate() { return true; }
    virtual void OnDeactivate() {}
    virtual bool OnApply() { return true; }

    DECLARE_MESSAGE_MAP()
};

class CInternalPropertyPage
    : public CUnknown
    , public IPropertyPage
    , public CCritSec
{
    CComPtr<IPropertyPageSite> m_pPageSite;
    CInterfaceList<IUnknown, &IID_IUnknown> m_pUnks;
    CInternalPropertyPageWnd* m_pWnd;

protected:
    virtual CInternalPropertyPageWnd* GetWindow() = 0;
    virtual LPCTSTR GetWindowTitle() = 0;
    virtual CSize GetWindowSize() = 0;

public:
    CInternalPropertyPage(LPUNKNOWN lpunk, HRESULT* phr);
    virtual ~CInternalPropertyPage();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IPropertyPage

    STDMETHODIMP SetPageSite(IPropertyPageSite* pPageSite);
    STDMETHODIMP Activate(HWND hwndParent, LPCRECT pRect, BOOL fModal);
    STDMETHODIMP Deactivate();
    STDMETHODIMP GetPageInfo(PROPPAGEINFO* pPageInfo);
    STDMETHODIMP SetObjects(ULONG cObjects, LPUNKNOWN* ppUnk);
    STDMETHODIMP Show(UINT nCmdShow);
    STDMETHODIMP Move(LPCRECT prect);
    STDMETHODIMP IsPageDirty();
    STDMETHODIMP Apply();
    STDMETHODIMP Help(LPCWSTR lpszHelpDir);
    STDMETHODIMP TranslateAccelerator(LPMSG lpMsg);
};

template<class WndClass>
class CInternalPropertyPageTempl : public CInternalPropertyPage
{
    virtual CInternalPropertyPageWnd* GetWindow() { return DEBUG_NEW WndClass(); }
    virtual LPCTSTR GetWindowTitle() { return WndClass::GetWindowTitle(); }
    virtual CSize GetWindowSize() { return WndClass::GetWindowSize(); }

public:
    CInternalPropertyPageTempl(LPUNKNOWN lpunk, HRESULT* phr)
        : CInternalPropertyPage(lpunk, phr) {
    }
};
