/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
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

#pragma once

#include <atlcoll.h>

interface __declspec(uuid("03481710-D73E-4674-839F-03EDE2D60ED8"))
ISpecifyPropertyPages2 :
public ISpecifyPropertyPages {
	STDMETHOD (CreatePage) (const GUID& guid, IPropertyPage** ppPage) = 0;
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
		if(fDirty && m_pPageSite) {
			m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
		}
	}
	bool GetDirty() {
		return m_fDirty;
	}

	virtual BOOL Create(IPropertyPageSite* pPageSite, LPCRECT pRect, CWnd* pParentWnd);

	virtual bool OnConnect(const CInterfaceList<IUnknown, &IID_IUnknown>& pUnks) {
		return true;
	}
	virtual void OnDisconnect() {}
	virtual bool OnActivate() {
		return true;
	}
	virtual void OnDeactivate() {}
	virtual bool OnApply() {
		return true;
	}

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
	virtual CInternalPropertyPageWnd* GetWindow() {
		return DNew WndClass();
	}

	virtual LPCTSTR GetWindowTitle() {
		return WndClass::GetWindowTitle();
	}

	virtual CSize GetWindowSize() {
		return WndClass::GetWindowSize();
	}

public:
	CInternalPropertyPageTempl(LPUNKNOWN lpunk, HRESULT* phr)
		: CInternalPropertyPage(lpunk, phr) {
	}
};
