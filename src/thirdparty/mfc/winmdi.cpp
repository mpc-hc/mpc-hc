// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
//#include "afxmdichildwndex.h"
//#include "afxmdiframewndex.h"


/////////////////////////////////////////////////////////////////////////////
// CMDIFrameWnd

BEGIN_MESSAGE_MAP(CMDIFrameWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CMDIFrameWnd)
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, CMDIFrameWnd::OnIdleUpdateCmdUI)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ARRANGE, &CMDIFrameWnd::OnUpdateMDIWindowCmd)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_CASCADE, &CMDIFrameWnd::OnUpdateMDIWindowCmd)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_HORZ, &CMDIFrameWnd::OnUpdateMDIWindowCmd)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_VERT, &CMDIFrameWnd::OnUpdateMDIWindowCmd)
	ON_WM_SIZE()
	ON_COMMAND_EX(ID_WINDOW_ARRANGE, &CMDIFrameWnd::OnMDIWindowCmd)
	ON_COMMAND_EX(ID_WINDOW_CASCADE, &CMDIFrameWnd::OnMDIWindowCmd)
	ON_COMMAND_EX(ID_WINDOW_TILE_HORZ, &CMDIFrameWnd::OnMDIWindowCmd)
	ON_COMMAND_EX(ID_WINDOW_TILE_VERT, &CMDIFrameWnd::OnMDIWindowCmd)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_NEW, &CMDIFrameWnd::OnUpdateMDIWindowCmd)
	ON_COMMAND(ID_WINDOW_NEW, &CMDIFrameWnd::OnWindowNew)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_COMMANDHELP, &CMDIFrameWnd::OnCommandHelp)
	ON_WM_MENUCHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMDIFrameWnd::CMDIFrameWnd()
{
	m_hWndMDIClient = NULL;
}

BOOL CMDIFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// send to MDI child first - will be re-sent through OnCmdMsg later
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild != NULL && AfxCallWndProc(pActiveChild,
	  pActiveChild->m_hWnd, WM_COMMAND, wParam, lParam) != 0)
		return TRUE; // handled by child

	if (CFrameWnd::OnCommand(wParam, lParam))
		return TRUE; // handled through normal mechanism (MDI child or frame)

	HWND hWndCtrl = (HWND)lParam;

	ASSERT(AFX_IDM_FIRST_MDICHILD == 0xFF00);
	if (hWndCtrl == NULL && (LOWORD(wParam) & 0xf000) == 0xf000)
	{
		// menu or accelerator within range of MDI children
		// default frame proc will handle it
		DefWindowProc(WM_COMMAND, wParam, lParam);
		return TRUE;
	}

	return FALSE;   // not handled
}

BOOL CMDIFrameWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	// pump through active child FIRST
	if (pActiveChild != NULL)
	{
		CPushRoutingFrame push(this);
		if (pActiveChild->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	// then pump through normal frame
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

LRESULT CMDIFrameWnd::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 0 && IsTracking())
		lParam = HID_BASE_COMMAND+m_nIDTracking;

	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild != NULL && AfxCallWndProc(pActiveChild,
	  pActiveChild->m_hWnd, WM_COMMANDHELP, wParam, lParam) != 0)
	{
		// handled by child
		return TRUE;
	}

	if (CFrameWnd::OnCommandHelp(wParam, lParam))
	{
		// handled by our base
		return TRUE;
	}

	if (lParam != 0)
	{
		CWinApp* pApp = AfxGetApp();
		if (pApp != NULL)
		{
			AfxGetApp()->WinHelpInternal(lParam);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CMDIFrameWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext*)
{
	CMenu* pMenu = NULL;
	if (m_hMenuDefault == NULL)
	{
		// default implementation for MFC V1 backward compatibility
		pMenu = GetMenu();
		ASSERT(pMenu != NULL);
		// This is attempting to guess which sub-menu is the Window menu.
		// The Windows user interface guidelines say that the right-most
		// menu on the menu bar should be Help and Window should be one
		// to the left of that.
		int iMenu = pMenu->GetMenuItemCount() - 2;

		// If this assertion fails, your menu bar does not follow the guidelines
		// so you will have to override this function and call CreateClient
		// appropriately or use the MFC V2 MDI functionality.
		ASSERT(iMenu >= 0);
		pMenu = pMenu->GetSubMenu(iMenu);
		ASSERT(pMenu != NULL);
	}

	return CreateClient(lpcs, pMenu);
}

BOOL CMDIFrameWnd::CreateClient(LPCREATESTRUCT lpCreateStruct,
	CMenu* pWindowMenu)
{
	ASSERT(m_hWnd != NULL);
	ASSERT(m_hWndMDIClient == NULL);
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_BORDER |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		MDIS_ALLCHILDSTYLES;    // allow children to be created invisible
	DWORD dwExStyle = 0;
	// will be inset by the frame

	// special styles for 3d effect on Win4
	dwStyle &= ~WS_BORDER;
	dwExStyle = WS_EX_CLIENTEDGE;

	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = pWindowMenu->GetSafeHmenu();
		// set hWindowMenu for MFC V1 backward compatibility
		// for MFC V2, window menu will be set in OnMDIActivate
	ccs.idFirstChild = AFX_IDM_FIRST_MDICHILD;

	if (lpCreateStruct->style & (WS_HSCROLL|WS_VSCROLL))
	{
		// parent MDIFrame's scroll styles move to the MDICLIENT
		dwStyle |= (lpCreateStruct->style & (WS_HSCROLL|WS_VSCROLL));

		// fast way to turn off the scrollbar bits (without a resize)
		ModifyStyle(WS_HSCROLL|WS_VSCROLL, 0, SWP_NOREDRAW|SWP_FRAMECHANGED);
	}

	// Create MDICLIENT control with special IDC
	if ((m_hWndMDIClient = ::AfxCtxCreateWindowEx(dwExStyle, _T("mdiclient"), NULL,
		dwStyle, 0, 0, 0, 0, m_hWnd, (HMENU)AFX_IDW_PANE_FIRST,
		AfxGetInstanceHandle(), (LPVOID)&ccs)) == NULL)
	{
		TRACE(traceAppMsg, 0, _T("Warning: CMDIFrameWnd::OnCreateClient: failed to create MDICLIENT.")
			_T(" GetLastError returns 0x%8.8X\n"), ::GetLastError());
		return FALSE;
	}
	// Move it to the top of z-order
	::BringWindowToTop(m_hWndMDIClient);

	return TRUE;
}

LRESULT CMDIFrameWnd::DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return ::DefFrameProc(m_hWnd, m_hWndMDIClient, nMsg, wParam, lParam);
}

BOOL CMDIFrameWnd::PreTranslateMessage(MSG* pMsg)
{
	// check for special cancel modes for ComboBoxes
	if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_NCLBUTTONDOWN)
		AfxCancelModes(pMsg->hwnd);    // filter clicks

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to consume message
	if (m_pNotifyHook != NULL && m_pNotifyHook->OnPreTranslateMessage(pMsg))
		return TRUE;
#endif

	CMDIChildWnd* pActiveChild = MDIGetActive();

	// current active child gets first crack at it
	if (pActiveChild != NULL && pActiveChild->PreTranslateMessage(pMsg))
		return TRUE;

	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// translate accelerators for frame and any children
		if (m_hAccelTable != NULL &&
			::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg))
		{
			return TRUE;
		}

		// special processing for MDI accelerators last
		// and only if it is not in SDI mode (print preview)
		if (GetActiveView() == NULL)
		{
			if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
			{
				// the MDICLIENT window may translate it
				if (::TranslateMDISysAccel(m_hWndMDIClient, pMsg))
					return TRUE;
			}
		}
	}

	return FALSE;
}

void CMDIFrameWnd::DelayUpdateFrameMenu(HMENU hMenuAlt)
{
	OnUpdateFrameMenu(hMenuAlt);

	m_nIdleFlags |= idleMenu;
}

void CMDIFrameWnd::OnIdleUpdateCmdUI()
{
	if (m_nIdleFlags & idleMenu)
	{
		DrawMenuBar();
		m_nIdleFlags &= ~idleMenu;
	}
	CFrameWnd::OnIdleUpdateCmdUI();
}

CFrameWnd* CMDIFrameWnd::GetActiveFrame()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild == NULL)
		return this;
	return pActiveChild;
}

BOOL CMDIFrameWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (cs.lpszClass == NULL)
	{
		VERIFY(AfxDeferRegisterClass(AFX_WNDMDIFRAME_REG));
		cs.lpszClass = _afxWndMDIFrame;
	}
	return TRUE;
}

BOOL CMDIFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
	CWnd* pParentWnd, CCreateContext* pContext)
{
	if (!CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle,
	  pParentWnd, pContext))
		return FALSE;

	// save menu to use when no active MDI child window is present
	ASSERT(m_hWnd != NULL);
	m_hMenuDefault = ::GetMenu(m_hWnd);
	return TRUE;
}

void CMDIFrameWnd::OnDestroy()
{
	CFrameWnd::OnDestroy();     // exit and misc cleanup

	// owned menu stored in shared slot for MDIFRAME
	if (m_hMenuDefault != NULL && ::GetMenu(m_hWnd) != m_hMenuDefault)
	{
		// must go through MDI client to get rid of MDI menu additions
		::SendMessage(m_hWndMDIClient, WM_MDISETMENU,
			(WPARAM)m_hMenuDefault, NULL);
		ASSERT(::GetMenu(m_hWnd) == m_hMenuDefault);
	}
}

void CMDIFrameWnd::OnSize(UINT nType, int, int)
{
	// do not call default - it will reposition the MDICLIENT
	if (nType != SIZE_MINIMIZED)
		RecalcLayout();
}

LRESULT CMDIFrameWnd::OnMenuChar(UINT nChar, UINT, CMenu*)
{
	// do not call Default() for Alt+(-) when in print preview mode
	if (m_lpfnCloseProc != NULL && nChar == (UINT)'-')
		return 0;
	else
		return Default();
}

CMDIChildWnd* CMDIFrameWnd::MDIGetActive(BOOL* pbMaximized) const
{
	// check first for MDI client window not created
	if (m_hWndMDIClient == NULL)
	{
		if (pbMaximized != NULL)
			*pbMaximized = FALSE;
		return NULL;
	}

	// MDI client has been created, get active MDI child
	HWND hWnd = (HWND)::SendMessage(m_hWndMDIClient, WM_MDIGETACTIVE, 0,
		(LPARAM)pbMaximized);
	CMDIChildWnd* pWnd = (CMDIChildWnd*)CWnd::FromHandlePermanent(hWnd);
	ASSERT(pWnd == NULL || pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)));

	// check for special pseudo-inactive state
	if (pWnd != NULL && pWnd->m_bPseudoInactive &&
		(pWnd->GetStyle() & WS_VISIBLE) == 0)
	{
		// Window is hidden, active, but m_bPseudoInactive -- return NULL
		pWnd = NULL;
		// Ignore maximized flag if pseudo-inactive and maximized
		if (pbMaximized != NULL)
			*pbMaximized = FALSE;
	}
	return pWnd;
}


CMDIChildWnd* CMDIFrameWnd::CreateNewChild(CRuntimeClass* pClass,
		UINT nResources, HMENU hMenu /* = NULL */, HACCEL hAccel /* = NULL */)
{
	ASSERT(pClass != NULL);
	CMDIChildWnd* pFrame = (CMDIChildWnd*) pClass->CreateObject();
	ASSERT_KINDOF(CMDIChildWnd, pFrame);

	// load the frame
	CCreateContext context;
	context.m_pCurrentFrame = this;

	pFrame->SetHandles(hMenu, hAccel);
	if (!pFrame->LoadFrame(nResources,
			WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, &context))
	{
		TRACE(traceAppMsg, 0, "Couldn't load frame window.\n");
		return NULL;
	}

	CString strFullString, strTitle;
	if (strFullString.LoadString(nResources))
		AfxExtractSubString(strTitle, strFullString, CDocTemplate::docName);

	// redraw the frame and parent
	pFrame->SetTitle(strTitle);
	pFrame->InitialUpdateFrame(NULL, TRUE);

	return pFrame;
}

/////////////////////////////////////////////////////////////////////////////
// CMDIFrameWnd Diagnostics

#ifdef _DEBUG
void CMDIFrameWnd::AssertValid() const
{
	CFrameWnd::AssertValid();
	ASSERT(m_hWndMDIClient == NULL || ::IsWindow(m_hWndMDIClient));
	ASSERT(m_hMenuDefault == NULL || ::IsMenu(m_hMenuDefault));
}

void CMDIFrameWnd::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);

	dc << "m_hWndMDIClient = " << (void*)m_hWndMDIClient;
	dc << "\nm_hMenuDefault = " << (void*)m_hMenuDefault;

	dc << "\n";
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWnd

BEGIN_MESSAGE_MAP(CMDIChildWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CMDIChildWnd)
	ON_WM_MOUSEACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_MDIACTIVATE()
	ON_WM_SIZE()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_NCCREATE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, &CMDIChildWnd::OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, &CMDIChildWnd::OnToolTipText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMDIChildWnd::CMDIChildWnd()
{
	m_hMenuShared = NULL;
	m_bPseudoInactive = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWnd special processing

LRESULT CMDIChildWnd::DefWindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return ::DefMDIChildProc(m_hWnd, nMsg, wParam, lParam);
}

BOOL CMDIChildWnd::DestroyWindow()
{
	if (m_hWnd == NULL)
		return FALSE;

	// avoid changing the caption during the destroy message(s)
	CMDIFrameWnd* pFrameWnd = GetMDIFrame();
	HWND hWndFrame = pFrameWnd->m_hWnd;
	ASSERT(::IsWindow(hWndFrame));
	DWORD dwStyle = SetWindowLong(hWndFrame, GWL_STYLE,
		GetWindowLong(hWndFrame, GWL_STYLE) & ~FWS_ADDTOTITLE);

	MDIDestroy();

	if (::IsWindow(hWndFrame))
	{
		ASSERT(hWndFrame == pFrameWnd->m_hWnd);
		SetWindowLong(hWndFrame, GWL_STYLE, dwStyle);
		pFrameWnd->OnUpdateFrameTitle(TRUE);
	}

	return TRUE;
}

BOOL CMDIChildWnd::PreTranslateMessage(MSG* pMsg)
{
	// check for special cancel modes for combo boxes
	if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_NCLBUTTONDOWN)
		AfxCancelModes(pMsg->hwnd);    // filter clicks

	// allow tooltip messages to be filtered
	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

	// we can't call 'CFrameWnd::PreTranslate' since it will translate
	//  accelerators in the context of the MDI Child - but since MDI Child
	//  windows don't have menus this doesn't work properly.  MDI Child
	//  accelerators must be translated in context of their MDI Frame.

	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// use document specific accelerator table over m_hAccelTable
		HACCEL hAccel = GetDefaultAccelerator();
		return hAccel != NULL &&
		   ::TranslateAccelerator(GetMDIFrame()->m_hWnd, hAccel, pMsg);
	}
	return FALSE;
}

BOOL CMDIChildWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	ASSERT(cs.style & WS_CHILD);
		// MFC V2 requires that MDI Children are created with proper styles,
		//  usually: WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW.
		// See Technical note TN019 for more details on MFC V1->V2 migration.

	return CFrameWnd::PreCreateWindow(cs);
}

BOOL CMDIChildWnd::Create(LPCTSTR lpszClassName,
	LPCTSTR lpszWindowName, DWORD dwStyle,
	const RECT& rect, CMDIFrameWnd* pParentWnd,
	CCreateContext* pContext)
{
	if (pParentWnd == NULL)
	{
		CWinThread *pThread = AfxGetThread();
		ENSURE_VALID(pThread);
		CWnd* pMainWnd = pThread->m_pMainWnd;
		ENSURE_VALID(pMainWnd);
		ASSERT_KINDOF(CMDIFrameWnd, pMainWnd);
		pParentWnd = (CMDIFrameWnd*)pMainWnd;
	}
	ASSERT(::IsWindow(pParentWnd->m_hWndMDIClient));

	// insure correct window positioning
	pParentWnd->RecalcLayout();

	// first copy into a CREATESTRUCT for PreCreate
	CREATESTRUCT cs;
	cs.dwExStyle = 0L;
	cs.lpszClass = lpszClassName;
	cs.lpszName = lpszWindowName;
	cs.style = dwStyle;
	cs.x = rect.left;
	cs.y = rect.top;
	cs.cx = rect.right - rect.left;
	cs.cy = rect.bottom - rect.top;
	cs.hwndParent = pParentWnd->m_hWnd;
	cs.hMenu = NULL;
	cs.hInstance = AfxGetInstanceHandle();
	cs.lpCreateParams = (LPVOID)pContext;

	if (!PreCreateWindow(cs))
	{
		PostNcDestroy();
		return FALSE;
	}
	// extended style must be zero for MDI Children (except under Win4)
	ASSERT(cs.hwndParent == pParentWnd->m_hWnd);    // must not change

	// now copy into a MDICREATESTRUCT for real create
	MDICREATESTRUCT mcs;
	mcs.szClass = cs.lpszClass;
	mcs.szTitle = cs.lpszName;
	mcs.hOwner = cs.hInstance;
	mcs.x = cs.x;
	mcs.y = cs.y;
	mcs.cx = cs.cx;
	mcs.cy = cs.cy;
	mcs.style = cs.style & ~(WS_MAXIMIZE | WS_VISIBLE);
	mcs.lParam = (LPARAM)cs.lpCreateParams;

	// create the window through the MDICLIENT window
	AfxHookWindowCreate(this);
	HWND hWnd = (HWND)::SendMessage(pParentWnd->m_hWndMDIClient,
		WM_MDICREATE, 0, (LPARAM)&mcs);
	if (!AfxUnhookWindowCreate())
		PostNcDestroy();        // cleanup if MDICREATE fails too soon

	if (hWnd == NULL)
		return FALSE;

	// special handling of visibility (always created invisible)
	if (cs.style & WS_VISIBLE)
	{
		// place the window on top in z-order before showing it
		::BringWindowToTop(hWnd);

		// show it as specified
		if (cs.style & WS_MINIMIZE)
			ShowWindow(SW_SHOWMINIMIZED);
		else if (cs.style & WS_MAXIMIZE)
			ShowWindow(SW_SHOWMAXIMIZED);
		else
			ShowWindow(SW_SHOWNORMAL);

		// make sure it is active (visibility == activation)
		pParentWnd->MDIActivate(this);

		// refresh MDI Window menu
		::SendMessage(pParentWnd->m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
	}

	ASSERT(hWnd == m_hWnd);
	return TRUE;
}

BOOL CMDIChildWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
		CWnd* pParentWnd, CCreateContext* pContext)
{
	// only do this once
	ASSERT_VALID_IDR(nIDResource);
	ASSERT(m_nIDHelp == 0 || m_nIDHelp == nIDResource);

	m_nIDHelp = nIDResource;    // ID for help context (+HID_BASE_RESOURCE)

	// parent must be MDI Frame (or NULL for default)
	ASSERT(pParentWnd == NULL || pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)));
	// will be a child of MDIClient
	ASSERT(!(dwDefaultStyle & WS_POPUP));
	dwDefaultStyle |= WS_CHILD;

	// if available - get MDI child menus from doc template
	CMultiDocTemplate* pTemplate;
	if (pContext != NULL &&
		(pTemplate = (CMultiDocTemplate*)pContext->m_pNewDocTemplate) != NULL)
	{
		ASSERT_KINDOF(CMultiDocTemplate, pTemplate);
		// get shared menu from doc template
		m_hMenuShared = pTemplate->m_hMenuShared;
		m_hAccelTable = pTemplate->m_hAccelTable;
	}
	else
	{
		TRACE(traceAppMsg, 0, "Warning: no shared menu/acceltable for MDI Child window.\n");
			// if this happens, programmer must load these manually
	}

	CString strFullString, strTitle;
	if (strFullString.LoadString(nIDResource))
		AfxExtractSubString(strTitle, strFullString, 0);    // first sub-string

	ASSERT(m_hWnd == NULL);
	if (!Create(GetIconWndClass(dwDefaultStyle, nIDResource),
	  strTitle, dwDefaultStyle, rectDefault,
	  (CMDIFrameWnd*)pParentWnd, pContext))
	{
		return FALSE;   // will self destruct on failure normally
	}

	// it worked !
	return TRUE;
}

void CMDIChildWnd::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	// update our parent frame - in case we are now maximized or not
	GetMDIFrame()->OnUpdateFrameTitle(TRUE);
}

BOOL CMDIChildWnd::UpdateClientEdge(LPRECT lpRect)
{
	// only adjust for active MDI child window
	CMDIFrameWnd* pFrameWnd = GetMDIFrame();
	CMDIChildWnd* pChild = pFrameWnd->MDIGetActive();

	// Only adjust for regular MDI child windows, not tabbed windows.  Attempting to set WS_EX_CLIENTEDGE on the tabbed
	// MDI client area window is subverted by CMDIClientAreaWnd::OnStyleChanging, so we always try to reset the style and
	// always repaint, none of which is necessary since the tabbed MDI children never change from maximized to restored.
#if 0
	CMDIChildWndEx* pChildEx = (pChild == NULL) ? NULL : DYNAMIC_DOWNCAST(CMDIChildWndEx, pChild);
	BOOL bIsTabbedMDIChild = (pChildEx == NULL) ? FALSE : pChildEx->GetMDIFrameWndEx() != NULL && pChildEx->GetMDIFrameWndEx()->AreMDITabs();
	if ((pChild == NULL || pChild == this) && !bIsTabbedMDIChild)
	{
		// need to adjust the client edge style as max/restore happens
		DWORD dwStyle = ::GetWindowLong(pFrameWnd->m_hWndMDIClient, GWL_EXSTYLE);
		DWORD dwNewStyle = dwStyle;
		if (pChild != NULL && !(GetExStyle() & WS_EX_CLIENTEDGE) && (GetStyle() & WS_MAXIMIZE))
		{
			dwNewStyle &= ~(WS_EX_CLIENTEDGE);
		}
		else
		{
			dwNewStyle |= WS_EX_CLIENTEDGE;
		}

		if (dwStyle != dwNewStyle)
		{
			// SetWindowPos will not move invalid bits
			::RedrawWindow(pFrameWnd->m_hWndMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

			// remove/add WS_EX_CLIENTEDGE to MDI client area
			::SetWindowLong(pFrameWnd->m_hWndMDIClient, GWL_EXSTYLE, dwNewStyle);
			::SetWindowPos(pFrameWnd->m_hWndMDIClient, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOCOPYBITS);

			// return new client area
			if (lpRect != NULL)
			{
				::GetClientRect(pFrameWnd->m_hWndMDIClient, lpRect);
			}

			return TRUE;
		}
	}
#endif
	return FALSE;
}

void CMDIChildWnd::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
	if (!(lpWndPos->flags & SWP_NOSIZE))
	{
		CRect rectClient;
		if (UpdateClientEdge(rectClient) && (GetStyle() & WS_MAXIMIZE))
		{
			// adjust maximized window size and position based on new
			//  size/position of the MDI client area.
			::AdjustWindowRectEx(rectClient, GetStyle(), FALSE, GetExStyle());
			lpWndPos->x = rectClient.left;
			lpWndPos->y = rectClient.top;
			lpWndPos->cx = rectClient.Width();
			lpWndPos->cy = rectClient.Height();
		}
	}

	CFrameWnd::OnWindowPosChanging(lpWndPos);
}

void CMDIChildWnd::OnDestroy()
{
	UpdateClientEdge();

	CFrameWnd::OnDestroy();
}

BOOL CMDIChildWnd::OnNcActivate(BOOL bActive)
{
	// bypass CFrameWnd::OnNcActivate()
	return CWnd::OnNcActivate(bActive);
}

int CMDIChildWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	int nResult = CFrameWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
	if (nResult == MA_NOACTIVATE || nResult == MA_NOACTIVATEANDEAT)
		return nResult;   // frame does not want to activate

	// activate this window if necessary
	CMDIFrameWnd* pFrameWnd = GetMDIFrame();
	ENSURE_VALID(pFrameWnd);
	CMDIChildWnd* pActive = pFrameWnd->MDIGetActive();
	if (pActive != this)
		MDIActivate();

	return nResult;
}

BOOL CMDIChildWnd::OnToolTipText(UINT msg, NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);
	UNUSED(pNMHDR);

	// check to see if the message is going directly to this window or not
	const MSG* pMsg = GetCurrentMessage();
	if (pMsg->hwnd != m_hWnd)
	{
		// let top level frame handle this for us
		return FALSE;
	}

	// otherwise, handle it ourselves
	return CFrameWnd::OnToolTipText(msg, pNMHDR, pResult);
}

void CMDIChildWnd::ActivateFrame(int nCmdShow)
{
	BOOL bVisibleThen = (GetStyle() & WS_VISIBLE) != 0;
	CMDIFrameWnd* pFrameWnd = GetMDIFrame();
	ASSERT_VALID(pFrameWnd);

	// determine default show command
	if (nCmdShow == -1)
	{
		// get maximized state of frame window (previously active child)
		BOOL bMaximized;
		pFrameWnd->MDIGetActive(&bMaximized);

		// convert show command based on current style
		DWORD dwStyle = GetStyle();
		if (bMaximized || (dwStyle & WS_MAXIMIZE))
			nCmdShow = SW_SHOWMAXIMIZED;
		else if (dwStyle & WS_MINIMIZE)
			nCmdShow = SW_SHOWMINIMIZED;
	}

	// finally, show the window
	CFrameWnd::ActivateFrame(nCmdShow);

	// update the Window menu to reflect new child window
	CMDIFrameWnd* pFrame = GetMDIFrame();
	::SendMessage(pFrame->m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);

	// Note: Update the m_bPseudoInactive flag.  This is used to handle the
	//  last MDI child getting hidden.  Windows provides no way to deactivate
	//  an MDI child window.

	BOOL bVisibleNow = (GetStyle() & WS_VISIBLE) != 0;
	if (bVisibleNow == bVisibleThen)
		return;

	if (!bVisibleNow)
	{
		// get current active window according to Windows MDI
		HWND hWnd = (HWND)::SendMessage(pFrameWnd->m_hWndMDIClient,
			WM_MDIGETACTIVE, 0, 0);
		if (hWnd != m_hWnd)
		{
			// not active any more -- window must have been deactivated
			ASSERT(!m_bPseudoInactive);
			return;
		}

		// check next window
		ASSERT(hWnd != NULL);
		pFrameWnd->MDINext();

		// see if it has been deactivated now...
		hWnd = (HWND)::SendMessage(pFrameWnd->m_hWndMDIClient,
			WM_MDIGETACTIVE, 0, 0);
		if (hWnd == m_hWnd)
		{
			// still active -- fake deactivate it
			ASSERT(hWnd != NULL);
			::SendMessage(pFrameWnd->m_hWndMDIClient, WM_MDIACTIVATE, (WPARAM)m_hWnd, NULL);
			m_bPseudoInactive = TRUE;   // so MDIGetActive returns NULL
		}
	}
	else if (m_bPseudoInactive)
	{
		// if state transitioned from not visible to visible, but
		//  was pseudo deactivated -- send activate notify now
		::SendMessage(pFrameWnd->m_hWndMDIClient, WM_MDIACTIVATE, NULL, (LPARAM)m_hWnd);
		ASSERT(!m_bPseudoInactive); // should get set in OnMDIActivate!
	}
}

void CMDIChildWnd::SetHandles(HMENU hMenu, HACCEL hAccel)
{
	m_hMenuShared = hMenu;
	m_hAccelTable = hAccel;
}

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWnd Diagnostics

#ifdef _DEBUG
void CMDIChildWnd::AssertValid() const
{
	CFrameWnd::AssertValid();
	ASSERT(m_hMenuShared == NULL || ::IsMenu(m_hMenuShared));
}

void CMDIChildWnd::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);

	dc << "m_hMenuShared = " << (void*)m_hMenuShared;
	dc << "\n";
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Smarts for the "Window" menu

HMENU CMDIFrameWnd::GetWindowMenuPopup(HMENU hMenuBar)
	// find which popup is the "Window" menu
{
	if (hMenuBar == NULL)
		return NULL;

	ASSERT(::IsMenu(hMenuBar));

	int iItem = ::GetMenuItemCount(hMenuBar);
	while (iItem--)
	{
		HMENU hMenuPop = ::GetSubMenu(hMenuBar, iItem);
		if (hMenuPop != NULL)
		{
			int iItemMax = ::GetMenuItemCount(hMenuPop);
			for (int iItemPop = 0; iItemPop < iItemMax; iItemPop++)
			{
				UINT nID = GetMenuItemID(hMenuPop, iItemPop);
				if (nID >= AFX_IDM_WINDOW_FIRST && nID <= AFX_IDM_WINDOW_LAST)
					return hMenuPop;
			}
		}
	}

	// no default menu found
	TRACE(traceAppMsg, 0, "Warning: GetWindowMenuPopup failed!\n");
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Smarts for updating the window menu based on the current child

void CMDIFrameWnd::OnUpdateFrameMenu(HMENU hMenuAlt)
{
	CMDIChildWnd* pActiveWnd = MDIGetActive();
	if (pActiveWnd != NULL)
	{
		// let child update the menu bar
		pActiveWnd->OnUpdateFrameMenu(TRUE, pActiveWnd, hMenuAlt);
	}
	else
	{
		// no child active, so have to update it ourselves
		//  (we can't send it to a child window, since pActiveWnd is NULL)
		if (hMenuAlt == NULL)
			hMenuAlt = m_hMenuDefault;  // use default
		::SendMessage(m_hWndMDIClient, WM_MDISETMENU, (WPARAM)hMenuAlt, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// MDI Child Extensions

// walk up two parents for MDIFrame that owns MDIChild (skip MDIClient)
CMDIFrameWnd* CMDIChildWnd::GetMDIFrame()
{
	ASSERT_KINDOF(CMDIChildWnd, this);
	ASSERT(m_hWnd != NULL);
	HWND hWndMDIClient = ::GetParent(m_hWnd);
	ASSERT(hWndMDIClient != NULL);

	CMDIFrameWnd* pMDIFrame;
	pMDIFrame = (CMDIFrameWnd*)CWnd::FromHandle(::GetParent(hWndMDIClient));
	ASSERT(pMDIFrame != NULL);
	ASSERT_KINDOF(CMDIFrameWnd, pMDIFrame);
	ASSERT(pMDIFrame->m_hWndMDIClient == hWndMDIClient);
	ASSERT_VALID(pMDIFrame);
	return pMDIFrame;
}

CWnd* CMDIChildWnd::GetMessageBar()
{
	// status bar/message bar owned by parent MDI frame
	return GetMDIFrame()->GetMessageBar();
}

void CMDIChildWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// update our parent window first
	GetMDIFrame()->OnUpdateFrameTitle(bAddToTitle);

	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;     // leave child window alone!

	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle)
	{
		TCHAR szText[256+_MAX_PATH];
		if (pDocument == NULL)
			Checked::tcsncpy_s(szText, _countof(szText), m_strTitle, _TRUNCATE);
		else
			Checked::tcsncpy_s(szText, _countof(szText), pDocument->GetTitle(), _TRUNCATE);
		if (m_nWindow > 0)
		{
			TCHAR szWinNumber[16+1];
			_stprintf_s(szWinNumber, _countof(szWinNumber), _T(":%d"), m_nWindow);
			
			if( lstrlen(szText) + lstrlen(szWinNumber) < _countof(szText) )
			{
				Checked::tcscat_s( szText, _countof(szText), szWinNumber ); 
			}
		}

		// set title if changed, but don't remove completely
		AfxSetWindowText(m_hWnd, szText);
	}
}

void CMDIChildWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd*)
{
	m_bPseudoInactive = FALSE;  // must be happening for real

	// make sure MDI client window has correct client edge
	UpdateClientEdge();

	// send deactivate notification to active view
	CView* pActiveView = GetActiveView();
	if (!bActivate && pActiveView != NULL)
		pActiveView->OnActivateView(FALSE, pActiveView, pActiveView);

	// allow hook to short circuit normal activation
	BOOL bHooked = FALSE;
#ifndef _AFX_NO_OLE_SUPPORT
	if (m_pNotifyHook != NULL && m_pNotifyHook->OnDocActivate(bActivate))
		bHooked = TRUE;
#endif

	// update titles (don't AddToTitle if deactivate last)
	if (!bHooked)
		OnUpdateFrameTitle(bActivate || (pActivateWnd != NULL));

	// re-activate the appropriate view
	if (bActivate)
	{
		if (pActiveView != NULL && GetMDIFrame() == GetActiveWindow())
			pActiveView->OnActivateView(TRUE, pActiveView, pActiveView);
	}

	// update menus
	if (!bHooked)
	{
		OnUpdateFrameMenu(bActivate, pActivateWnd, NULL);
		GetMDIFrame()->DrawMenuBar();
	}
}

void CMDIChildWnd::OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd,
	HMENU hMenuAlt)
{
	CMDIFrameWnd* pFrame = GetMDIFrame();

	if (hMenuAlt == NULL && bActivate)
	{
		// attempt to get default menu from document
		CDocument* pDoc = GetActiveDocument();
		if (pDoc != NULL)
			hMenuAlt = pDoc->GetDefaultMenu();
	}

	// use default menu stored in frame if none from document
	if (hMenuAlt == NULL)
		hMenuAlt = m_hMenuShared;

	if (hMenuAlt != NULL && bActivate)
	{
		ASSERT(pActivateWnd == this);

		// activating child, set parent menu
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDISETMENU,
			(WPARAM)hMenuAlt, (LPARAM)pFrame->GetWindowMenuPopup(hMenuAlt));
	}
	else if (hMenuAlt != NULL && !bActivate && pActivateWnd == NULL)
	{
		// destroying last child
		HMENU hMenuLast = NULL;
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDISETMENU,
			(WPARAM)pFrame->m_hMenuDefault, (LPARAM)hMenuLast);
	}
	else
	{
		// refresh MDI Window menu (even if non-shared menu)
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
	}
}

BOOL CMDIChildWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CFrameWnd::OnNcCreate(lpCreateStruct))
		return FALSE;

	// handle extended styles under Win4
	// call PreCreateWindow again just to get dwExStyle
	VERIFY(PreCreateWindow(*lpCreateStruct));
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lpCreateStruct->dwExStyle);

	return TRUE;
}

int CMDIChildWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// call base class with lParam context (not MDI one)
	MDICREATESTRUCT* lpmcs;
	lpmcs = (MDICREATESTRUCT*)lpCreateStruct->lpCreateParams;
	CCreateContext* pContext = (CCreateContext*)lpmcs->lParam;

	return OnCreateHelper(lpCreateStruct, pContext);
}

/////////////////////////////////////////////////////////////////////////////
// Special UI processing depending on current active child

void CMDIFrameWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;     // leave it alone!

#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to set the title (used for OLE support)
	if (m_pNotifyHook != NULL && m_pNotifyHook->OnUpdateFrameTitle())
		return;
#endif

	CMDIChildWnd* pActiveChild = NULL;
	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle &&
	  (pActiveChild = MDIGetActive()) != NULL &&
	  (pActiveChild->GetStyle() & WS_MAXIMIZE) == 0 &&
	  (pDocument != NULL ||
	   (pDocument = pActiveChild->GetActiveDocument()) != NULL))
		UpdateFrameTitleForDocument(pDocument->GetTitle());
	else
	{
		LPCTSTR lpstrTitle = NULL;
		CString strTitle;

		if (pActiveChild != NULL &&
			(pActiveChild->GetStyle() & WS_MAXIMIZE) == 0)
		{
			strTitle = pActiveChild->GetTitle();
			if (!strTitle.IsEmpty())
				lpstrTitle = strTitle;
		}
		UpdateFrameTitleForDocument(lpstrTitle);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Standard MDI Commands

// Two function for all standard MDI "Window" commands
void CMDIFrameWnd::OnUpdateMDIWindowCmd(CCmdUI* pCmdUI)
{
	ASSERT(m_hWndMDIClient != NULL);
	pCmdUI->Enable(MDIGetActive() != NULL);
}

BOOL CMDIFrameWnd::OnMDIWindowCmd(UINT nID)
{
	ASSERT(m_hWndMDIClient != NULL);

	UINT msg;
	UINT wParam = 0;
	switch (nID)
	{
	default:
		return FALSE;       // not for us
	case ID_WINDOW_ARRANGE:
		msg = WM_MDIICONARRANGE;
		break;
	case ID_WINDOW_CASCADE:
		msg = WM_MDICASCADE;
		break;
	case ID_WINDOW_TILE_HORZ:
		wParam = MDITILE_HORIZONTAL;
		// fall through
	case ID_WINDOW_TILE_VERT:
		ASSERT(MDITILE_VERTICAL == 0);
		msg = WM_MDITILE;
		break;
	}

	::SendMessage(m_hWndMDIClient, msg, wParam, 0);
	return TRUE;
}

void CMDIFrameWnd::OnWindowNew()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	CDocument* pDocument;
	if (pActiveChild == NULL ||
	  (pDocument = pActiveChild->GetActiveDocument()) == NULL)
	{
		TRACE(traceAppMsg, 0, "Warning: No active document for WindowNew command.\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		return;     // command failed
	}

	// otherwise we have a new frame !
	CDocTemplate* pTemplate = pDocument->GetDocTemplate();
	ASSERT_VALID(pTemplate);
	CFrameWnd* pFrame = pTemplate->CreateNewFrame(pDocument, pActiveChild);
	if (pFrame == NULL)
	{
		TRACE(traceAppMsg, 0, "Warning: failed to create new frame.\n");
		return;     // command failed
	}

	pTemplate->InitialUpdateFrame(pFrame, pDocument);
}

void CMDIFrameWnd::SetMenuBarVisibility(DWORD dwStyle)
{
	ENSURE_ARG(dwStyle == AFX_MBV_KEEPVISIBLE);
	ASSERT(m_dwMenuBarVisibility == AFX_MBV_KEEPVISIBLE);
}

BOOL CMDIFrameWnd::SetMenuBarState(DWORD dwState)
{
	return m_dwMenuBarState == AFX_MBS_HIDDEN ? FALSE : CFrameWnd::SetMenuBarState(dwState);
}

IMPLEMENT_DYNCREATE(CMDIFrameWnd, CFrameWnd)
IMPLEMENT_DYNCREATE(CMDIChildWnd, CFrameWnd)

////////////////////////////////////////////////////////////////////////////
