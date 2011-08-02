/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
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
#include "resource.h"
#include "DirectVobSubFilter.h"
#include "../../../DSUtil/DSUtil.h"

// hWnd == INVALID_HANDLE_VALUE - get name, hWnd != INVALID_HANDLE_VALUE - show ppage
static TCHAR* CallPPage(IFilterGraph* pGraph, int idx, HWND hWnd);

static HHOOK g_hHook = (HHOOK)INVALID_HANDLE_VALUE;

static UINT WM_DVSPREVSUB = RegisterWindowMessage(TEXT("WM_DVSPREVSUB"));
static UINT WM_DVSNEXTSUB = RegisterWindowMessage(TEXT("WM_DVSNEXTSUB"));
static UINT WM_DVSHIDESUB = RegisterWindowMessage(TEXT("WM_DVSHIDESUB"));
static UINT WM_DVSSHOWSUB = RegisterWindowMessage(TEXT("WM_DVSSHOWSUB"));
static UINT WM_DVSSHOWHIDESUB = RegisterWindowMessage(TEXT("WM_DVSSHOWHIDESUB"));
static UINT s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
static UINT WM_NOTIFYICON = RegisterWindowMessage(TEXT("MYWM_NOTIFYICON"));

LRESULT CALLBACK HookProc(UINT code, WPARAM wParam, LPARAM lParam)
{
	MSG* msg = (MSG*)lParam;

	if(msg->message == WM_KEYDOWN) {
		switch(msg->wParam) {
			case VK_F13:
				PostMessage(HWND_BROADCAST, WM_DVSPREVSUB, 0, 0);
				break;
			case VK_F14:
				PostMessage(HWND_BROADCAST, WM_DVSNEXTSUB, 0, 0);
				break;
			case VK_F15:
				PostMessage(HWND_BROADCAST, WM_DVSHIDESUB, 0, 0);
				break;
			case VK_F16:
				PostMessage(HWND_BROADCAST, WM_DVSSHOWSUB, 0, 0);
				break;
			case VK_F17:
				PostMessage(HWND_BROADCAST, WM_DVSSHOWHIDESUB, 0, 0);
				break;
			default:
				break;
		}
	}

	// Always call next hook in chain
	return CallNextHookEx(g_hHook, code,  wParam, lParam);
}

class CSystrayWindow : public CWnd
{
	SystrayIconData* m_tbid;

	void StepSub(int dir) {
		int iSelected, nLangs;
		if(FAILED(m_tbid->dvs->get_LanguageCount(&nLangs))) {
			return;
		}
		if(FAILED(m_tbid->dvs->get_SelectedLanguage(&iSelected))) {
			return;
		}
		if(nLangs > 0) {
			m_tbid->dvs->put_SelectedLanguage((iSelected+dir+nLangs)%nLangs);
		}
	}

	void ShowSub(bool fShow) {
		m_tbid->dvs->put_HideSubtitles(!fShow);
	}

	void ToggleSub() {
		bool fShow;
		if(FAILED(m_tbid->dvs->get_HideSubtitles(&fShow))) {
			return;
		}
		m_tbid->dvs->put_HideSubtitles(!fShow);
	}

public:
	CSystrayWindow(SystrayIconData* tbid) : m_tbid(tbid) {}

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnDVSPrevSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSNextSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSHideSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSShowSub(WPARAM, LPARAM);
	afx_msg LRESULT OnDVSShowHideSub(WPARAM, LPARAM);
	afx_msg LRESULT OnTaskBarRestart(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyIcon(WPARAM, LPARAM);
};

BEGIN_MESSAGE_MAP(CSystrayWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(WM_DVSPREVSUB, OnDVSPrevSub)
	ON_REGISTERED_MESSAGE(WM_DVSNEXTSUB, OnDVSNextSub)
	ON_REGISTERED_MESSAGE(WM_DVSHIDESUB, OnDVSHideSub)
	ON_REGISTERED_MESSAGE(WM_DVSSHOWSUB, OnDVSShowSub)
	ON_REGISTERED_MESSAGE(WM_DVSSHOWHIDESUB, OnDVSShowHideSub)
	ON_REGISTERED_MESSAGE(s_uTaskbarRestart, OnTaskBarRestart)
	ON_REGISTERED_MESSAGE(WM_NOTIFYICON, OnNotifyIcon)
END_MESSAGE_MAP()

int CSystrayWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	if(g_hHook == INVALID_HANDLE_VALUE) {
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		//		g_hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)HookProc, AfxGetInstanceHandle(), 0);
	}

	SetTimer(1, 5000, NULL);

	PostMessage(s_uTaskbarRestart);

	return 0;
}

void CSystrayWindow::OnClose()
{
	DestroyWindow();
}

void CSystrayWindow::OnDestroy()
{
	NOTIFYICONDATA tnid;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = m_hWnd;
	tnid.uID = IDI_ICON1;
	Shell_NotifyIcon(NIM_DELETE, &tnid);

	if(g_hHook != INVALID_HANDLE_VALUE) {
		UnhookWindowsHookEx(g_hHook);
		g_hHook = (HHOOK)INVALID_HANDLE_VALUE;
	}

	PostQuitMessage(0);
}

void CSystrayWindow::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1) {
		UINT fScreenSaver = 0;
		if(SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, (PVOID)&fScreenSaver, 0)) {
			SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, 0, SPIF_SENDWININICHANGE); // this might not be needed at all...
			SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, fScreenSaver, 0, SPIF_SENDWININICHANGE);
		}
	}

	CWnd::OnTimer(nIDEvent);
}

LRESULT CSystrayWindow::OnDVSPrevSub(WPARAM, LPARAM)
{
	StepSub(-1);
	return 0;
}

LRESULT CSystrayWindow::OnDVSNextSub(WPARAM, LPARAM)
{
	StepSub(+1);
	return 0;
}

LRESULT CSystrayWindow::OnDVSHideSub(WPARAM, LPARAM)
{
	ShowSub(false);
	return 0;
}

LRESULT CSystrayWindow::OnDVSShowSub(WPARAM, LPARAM)
{
	ShowSub(true);
	return 0;
}

LRESULT CSystrayWindow::OnDVSShowHideSub(WPARAM, LPARAM)
{
	ToggleSub();
	return 0;
}

LRESULT CSystrayWindow::OnTaskBarRestart(WPARAM, LPARAM)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(m_tbid->fShowIcon) {
		NOTIFYICONDATA tnid;
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = m_hWnd;
		tnid.uID = IDI_ICON1;
		tnid.hIcon = (HICON)LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON1));
		//		tnid.hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_LOADTRANSPARENT);
		tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		tnid.uCallbackMessage = WM_NOTIFYICON;
		lstrcpyn(tnid.szTip, TEXT("DirectVobSub"), sizeof(tnid.szTip));

		BOOL res = Shell_NotifyIcon(NIM_ADD, &tnid);

		if(tnid.hIcon) {
			DestroyIcon(tnid.hIcon);
		}

		return res?0:-1;
	}

	return 0;
}

LRESULT CSystrayWindow::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	if((UINT)wParam != IDI_ICON1) {
		return -1;
	}

	HWND hWnd = m_hWnd;

	switch((UINT)lParam) {
		case WM_LBUTTONDBLCLK: {
			// IMPORTANT: we must not hold the graph at the same time as showing the property page
			// or else when closing the app the graph doesn't get released and dvobsub's JoinFilterGraph
			// is never called to close us down.

			CComPtr<IBaseFilter> pBF2;

			BeginEnumFilters(m_tbid->graph, pEF, pBF) {
				if(!CComQIPtr<IDirectVobSub>(pBF)) {
					continue;
				}

				if(CComQIPtr<IVideoWindow> pVW = m_tbid->graph) {
					HWND hwnd;
					if(SUCCEEDED(pVW->get_Owner((OAHWND*)&hwnd))
							|| SUCCEEDED(pVW->get_MessageDrain((OAHWND*)&hwnd))) {
						hWnd = hwnd;
					}
				}

				pBF2 = pBF;

				break;
			}
			EndEnumFilters

			if(pBF2) {
				ShowPPage(pBF2, hWnd);
			}
		}
		break;

		case WM_RBUTTONDOWN: {
			POINT p;
			GetCursorPos(&p);

			CInterfaceArray<IAMStreamSelect> pStreams;
			CStringArray names;

			BeginEnumFilters(m_tbid->graph, pEF, pBF) {
				CString name = GetFilterName(pBF);
				if(name.IsEmpty()) {
					continue;
				}

				if(CComQIPtr<IAMStreamSelect> pSS = pBF) {
					pStreams.Add(pSS);
					names.Add(name);
				}
			}
			EndEnumFilters

			CMenu popup;
			popup.CreatePopupMenu();

			for(ptrdiff_t j = 0; j < pStreams.GetCount(); j++) {
				bool fMMSwitcher = !names[j].Compare(_T("Morgan Stream Switcher"));

				DWORD cStreams = 0;
				pStreams[j]->Count(&cStreams);

				DWORD flags, group, prevgroup = (DWORD)-1;

				for(UINT i = 0; i < cStreams; i++) {
					WCHAR* pName = NULL;

					if(S_OK == pStreams[j]->Info(i, 0, &flags, 0, &group, &pName, 0, 0)) {
						if(prevgroup != group && i > 1) {
							if(fMMSwitcher) {
								cStreams = i;
								break;
							}
							popup.AppendMenu(MF_SEPARATOR);
						}
						prevgroup = group;

						if(pName) {
							popup.AppendMenu(MF_ENABLED|MF_STRING|(flags?MF_CHECKED:MF_UNCHECKED), (1<<15)|(j<<8)|(i), CString(pName));
							CoTaskMemFree(pName);
						}
					}
				}

				if(cStreams > 0) {
					popup.AppendMenu(MF_SEPARATOR);
				}
			}

			int i = 0;

			TCHAR* str;
			str = CallPPage(m_tbid->graph, i, (HWND)INVALID_HANDLE_VALUE);
			while(str) {
				if(_tcsncmp(str, _T("DivX MPEG"), 9) || m_tbid->fRunOnce) { // divx3's ppage will crash if the graph hasn't been run at least once yet
					popup.AppendMenu(MF_ENABLED|MF_STRING|MF_UNCHECKED, (1<<14)|(i), str);
				}

				delete [] str;

				i++;
				str = CallPPage(m_tbid->graph, i, (HWND)INVALID_HANDLE_VALUE);
			}

			SetForegroundWindow();
			UINT id = popup.TrackPopupMenu(TPM_LEFTBUTTON|TPM_RETURNCMD, p.x, p.y, CWnd::FromHandle(hWnd), 0);
			PostMessage(WM_NULL);

			if(id & (1<<15)) {
				pStreams[(id>>8)&0x3f]->Enable(id&0xff, AMSTREAMSELECTENABLE_ENABLE);
			} else if(id & (1<<14)) {
				if(CComQIPtr<IVideoWindow> pVW = m_tbid->graph) {
					HWND hwnd;
					if(SUCCEEDED(pVW->get_Owner((OAHWND*)&hwnd))
							|| SUCCEEDED(pVW->get_MessageDrain((OAHWND*)&hwnd))) {
						hWnd = hwnd;
					}
				}

				CallPPage(m_tbid->graph, id&0xff, hWnd);
			}
		}
		break;

		default:
			break;
	}

	return 0;
}

//

DWORD CALLBACK SystrayThreadProc(void* pParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CSystrayWindow wnd((SystrayIconData*)pParam);
	if(!wnd.CreateEx(0, AfxRegisterWndClass(0), _T("DVSWND"), WS_OVERLAPPED, CRect(0, 0, 0, 0), NULL, 0, NULL)) {
		return (DWORD)-1;
	}

	((SystrayIconData*)pParam)->hSystrayWnd = wnd.m_hWnd;

	MSG msg;
	while(GetMessage(&msg, NULL/*wnd.m_hWnd*/, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

// TODO: replace this function

// hWnd == INVALID_HANDLE_VALUE - get name, hWnd != INVALID_HANDLE_VALUE - show ppage
static TCHAR* CallPPage(IFilterGraph* pGraph, int idx, HWND hWnd)
{
	int i = 0;
	bool fFound = false;

	WCHAR* wstr = NULL;
	CComPtr<IBaseFilter> pFilter;
	CAUUID caGUID;
	caGUID.pElems = NULL;

	BeginEnumFilters(pGraph, pEF, pBF) {
		CComQIPtr<ISpecifyPropertyPages> pSPS = pBF;
		if(!pSPS) {
			continue;
		}

		if(i == idx) {
			pFilter = pBF;
			pSPS->GetPages(&caGUID);
			wstr = _wcsdup(CStringW(GetFilterName(pBF))); // double char-wchar conversion happens in the non-unicode build, but anyway... :)
			break;
		}

		i++;
	}
	EndEnumFilters

	TCHAR* ret = NULL;

	if(pFilter) {
		if(hWnd != INVALID_HANDLE_VALUE) {
			ShowPPage(pFilter, hWnd);
		} else {
			ret = new TCHAR[wcslen(wstr)+1];
			if(ret) {
				_tcscpy(ret, CString(wstr));
			}
		}
	}

	if(caGUID.pElems) {
		CoTaskMemFree(caGUID.pElems);
	}
	if(wstr) {
		free(wstr);
	}

	return(ret);
}
