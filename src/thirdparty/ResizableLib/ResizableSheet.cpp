// ResizableSheet.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////
//
// This file is part of ResizableLib
// https://github.com/ppescher/resizablelib
//
// Copyright (C) 2000-2015 by Paolo Messina
// mailto:ppescher@hotmail.com
//
// The contents of this file are subject to the Artistic License 2.0
// http://opensource.org/licenses/Artistic-2.0
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResizableSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizableSheet

IMPLEMENT_DYNAMIC(CResizableSheet, CPropertySheet)

inline void CResizableSheet::PrivateConstruct()
{
	m_bEnableSaveRestore = FALSE;
	m_bSavePage = FALSE;
	m_dwGripTempState = 1;
	m_bLayoutDone = FALSE;
	m_bRectOnly = FALSE;
	m_nCallbackID = 0;
}

inline BOOL CResizableSheet::IsWizard() const
{
	return (m_psh.dwFlags & PSH_WIZARD);
}

CResizableSheet::CResizableSheet()
{
	PrivateConstruct();
}

CResizableSheet::CResizableSheet(UINT nIDCaption, CWnd *pParentWnd, UINT iSelectPage)
	 : CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	PrivateConstruct();
}

CResizableSheet::CResizableSheet(LPCTSTR pszCaption, CWnd *pParentWnd, UINT iSelectPage)
	 : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	PrivateConstruct();
}

CResizableSheet::~CResizableSheet()
{
}

BEGIN_MESSAGE_MAP(CResizableSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CResizableSheet)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_NCCREATE()
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT_EX(PSN_SETACTIVE, OnPageChanging)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizableSheet message handlers

BOOL CResizableSheet::OnNcCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (!CPropertySheet::OnNcCreate(lpCreateStruct))
		return FALSE;

	// child dialogs don't want resizable border or size grip,
	// nor they can handle the min/max size constraints
	BOOL bChild = lpCreateStruct->style & WS_CHILD;

	// create and init the size-grip
	if (!CreateSizeGrip(!bChild))
		return FALSE;

	MakeResizable(lpCreateStruct);
	
	return TRUE;
}

BOOL CResizableSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	// initialize layout
	PresetLayout();
	m_bLayoutDone = TRUE;

	return bResult;
}

void CResizableSheet::OnDestroy() 
{
	if (m_bEnableSaveRestore)
	{
		SaveWindowRect(m_sSection, m_bRectOnly);
		if (m_bSavePage)
			SavePage(m_sSection);
	}

	// reset instance data
	RemoveAllAnchors();
	ResetAllRects();
	PrivateConstruct();

	CPropertySheet::OnDestroy();
}

// maps an index to a button ID and vice-versa
static UINT _propButtons[] =
{
	IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP,
	ID_WIZBACK, ID_WIZNEXT, ID_WIZFINISH
};
const int _propButtonsCount = sizeof(_propButtons)/sizeof(UINT);

// horizontal line in wizard mode
#define ID_WIZLINE	ID_WIZFINISH+1

void CResizableSheet::PresetLayout()
{
	// set the initial size as the min track size
	CRect rc;
	GetWindowRect(&rc);
	SetMinTrackSize(rc.Size());

	// use *total* parent size to have correct margins
	CRect rectPage, rectSheet;
	GetTotalClientRect(&rectSheet);

	// get page area
	if (IsWizard())
	{
		HWND hPage = PropSheet_GetCurrentPageHwnd(m_hWnd);
		::GetWindowRect(hPage, &rectPage);
	}
	else
	{
		GetTabControl()->GetWindowRect(&rectPage);
	}
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rectPage, 2);

	// calculate margins
	CRect rect;
	int cxDiff = rectSheet.right - rectPage.right;
	int cyDiff = 0;

	// try all possible buttons
	for (int i = 0; i < _propButtonsCount; i++)
	{
		CWnd* pWnd = GetDlgItem(_propButtons[i]);
		if (NULL != pWnd)
		{
			// move buttons if necessary
			if (GetStyle() & WS_CHILD)
			{
				pWnd->GetWindowRect(&rect);
				::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);

				cyDiff = rectSheet.bottom - rect.bottom;
				rect.OffsetRect(cxDiff, cyDiff);

				pWnd->MoveWindow(&rect);
			}
			// add buttons to the layout manager
			AddAnchor(_propButtons[i], BOTTOM_RIGHT);
		}
	}

	// setup pages area
	if (IsWizard())
	{
		// move line and pages if necessary
		if (GetStyle() & WS_CHILD)
		{
			GetDlgItem(ID_WIZLINE)->GetWindowRect(&rect);
			::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);

			rect.OffsetRect(0, cyDiff);
			rect.InflateRect(cxDiff, 0);

			GetDlgItem(ID_WIZLINE)->MoveWindow(&rect);

			rectPage.bottom += cyDiff;
			rectPage.left = 0;
			rectPage.top = 0;
			rectPage.right = rectSheet.right;
		}

		AddAnchor(ID_WIZLINE, BOTTOM_LEFT, BOTTOM_RIGHT);

		// hide tab control
		GetTabControl()->ShowWindow(SW_HIDE);

		// pre-calculate margins
		m_sizePageTL = rectPage.TopLeft() - rectSheet.TopLeft();
		m_sizePageBR = rectPage.BottomRight() - rectSheet.BottomRight();
	}
	else
	{
		// grow tab to the available sheet space
		if (cyDiff > 0)
			rectSheet.bottom = rectPage.bottom + cyDiff;
		
		if (GetStyle() & WS_CHILD)
			GetTabControl()->MoveWindow(&rectSheet);

		AddAnchor(AFX_IDC_TAB_CONTROL, TOP_LEFT, BOTTOM_RIGHT);
	}

	// add a callback for active page (which can change at run-time)
	m_nCallbackID = AddAnchorCallback();

	// prevent flickering
	GetTabControl()->ModifyStyle(0, WS_CLIPSIBLINGS);
}

BOOL CResizableSheet::ArrangeLayoutCallback(LAYOUTINFO &layout) const
{
	if (layout.nCallbackID != m_nCallbackID)	// we only added 1 callback
		return CResizableLayout::ArrangeLayoutCallback(layout);

	// set layout info for active page
	layout.hWnd = PropSheet_GetCurrentPageHwnd(m_hWnd);
	if (!::IsWindow(layout.hWnd))
		return FALSE;

	// set margins
	if (IsWizard())	// wizard mode
	{
		// use pre-calculated margins
		layout.marginTopLeft = m_sizePageTL;
		layout.marginBottomRight = m_sizePageBR;
	}
	else	// tab mode
	{
		CTabCtrl* pTab = GetTabControl();
		ASSERT(pTab != NULL);

		// get tab position after resizing and calc page rect
		CRect rectPage, rectSheet;
		GetTotalClientRect(&rectSheet);

		if (!GetAnchorPosition(pTab->m_hWnd, rectSheet, rectPage))
			return FALSE; // no page yet

		// temporarily resize the tab control to calc page size
		CRect rectSave;
		pTab->GetWindowRect(rectSave);
		::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rectSave, 2);
		pTab->SetRedraw(FALSE);
		pTab->MoveWindow(rectPage, FALSE);
		pTab->AdjustRect(FALSE, &rectPage);
		pTab->MoveWindow(rectSave, FALSE);
		pTab->SetRedraw(TRUE);

		// set margins
		layout.marginTopLeft = rectPage.TopLeft() - rectSheet.TopLeft();
		layout.marginBottomRight = rectPage.BottomRight() - rectSheet.BottomRight();
	}

	// set anchor types
	layout.anchorTopLeft = TOP_LEFT;
	layout.anchorBottomRight = BOTTOM_RIGHT;

	// use this layout info
	return TRUE;
}

void CResizableSheet::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (nType == SIZE_MAXHIDE || nType == SIZE_MAXSHOW)
		return;		// arrangement not needed

	if (nType == SIZE_MAXIMIZED)
		HideSizeGrip(&m_dwGripTempState);
	else
		ShowSizeGrip(&m_dwGripTempState);

	// update grip and layout
	UpdateSizeGrip();
	ArrangeLayout();
}

BOOL CResizableSheet::OnPageChanging(NMHDR* /*pNotifyStruct*/, LRESULT* /*pResult*/)
{
	// update new wizard page
	// active page changes after this notification
	PostMessage(WM_SIZE);

	return FALSE;	// continue routing
}

BOOL CResizableSheet::OnEraseBkgnd(CDC* pDC) 
{
	ClipChildren(pDC, FALSE);

	BOOL bRet = CPropertySheet::OnEraseBkgnd(pDC);

	ClipChildren(pDC, TRUE);

	return bRet;
}

BOOL CResizableSheet::CalcSizeExtra(HWND /*hWndChild*/, const CSize& sizeChild, CSize& sizeExtra)
{
	CTabCtrl* pTab = GetTabControl();
	if (!pTab)
		return FALSE;

	// get margins of tabcontrol
	CRect rectMargins;
	if (!GetAnchorMargins(pTab->m_hWnd, sizeChild, rectMargins))
		return FALSE;

	// get margin caused by tabcontrol
	CRect rectTabMargins(0,0,0,0);

	// get tab position after resizing and calc page rect
	CRect rectPage, rectSheet;
	GetTotalClientRect(&rectSheet);

	if (!GetAnchorPosition(pTab->m_hWnd, rectSheet, rectPage))
		return FALSE; // no page yet

	// temporarily resize the tab control to calc page size
	CRect rectSave;
	pTab->GetWindowRect(rectSave);
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rectSave, 2);
	pTab->SetRedraw(FALSE);
	pTab->MoveWindow(rectPage, FALSE);
	pTab->AdjustRect(TRUE, &rectTabMargins);
	pTab->MoveWindow(rectSave, FALSE);
	pTab->SetRedraw(TRUE);

	// add non-client size
	::AdjustWindowRectEx(&rectTabMargins, GetStyle(), !(GetStyle() & WS_CHILD) &&
		::IsMenu(GetMenu()->GetSafeHmenu()), GetExStyle());

	// compute extra size
	sizeExtra = rectMargins.TopLeft() + rectMargins.BottomRight() +
		rectTabMargins.Size();
	return TRUE;
}

void CResizableSheet::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	MinMaxInfo(lpMMI);

	if (!GetTabControl())
		return;

	const int nCount = GetPageCount();
	for (int idx = 0; idx < nCount; ++idx)
	{
		if (IsWizard())	// wizard mode
		{
			// use pre-calculated margins
			CRect rectExtra(-CPoint(m_sizePageTL), -CPoint(m_sizePageBR));
			// add non-client size
			::AdjustWindowRectEx(&rectExtra, GetStyle(), !(GetStyle() & WS_CHILD) &&
				::IsMenu(GetMenu()->GetSafeHmenu()), GetExStyle());
			ChainMinMaxInfo(lpMMI, *GetPage(idx), rectExtra.Size());
		}
		else	// tab mode
		{
			ChainMinMaxInfoCB(lpMMI, *GetPage(idx));
		}
	}
}

// protected members

int CResizableSheet::GetMinWidth()
{
	CRect rectWnd, rectSheet;
	GetTotalClientRect(&rectSheet);

	int max = 0, min = rectSheet.Width();
	// search for leftmost and rightmost button margins
	for (int i = 0; i < 7; i++)
	{
		const CWnd* pWnd = GetDlgItem(_propButtons[i]);
		// exclude not present or hidden buttons
		if (pWnd == NULL || !(pWnd->GetStyle() & WS_VISIBLE))
			continue;

		// left position is relative to the right border
		// of the parent window (negative value)
		pWnd->GetWindowRect(&rectWnd);
		::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rectWnd, 2);
		const int left = rectSheet.right - rectWnd.left;
		const int right = rectSheet.right - rectWnd.right;

		if (left > max)
			max = left;
		if (right < min)
			min = right;
	}

	// sizing border width
	const int border = GetSystemMetrics(SM_CXSIZEFRAME);
	
	// compute total width
	return max + min + 2*border;
}


// NOTE: this must be called after all the other settings
//       to have the window and its controls displayed properly
void CResizableSheet::EnableSaveRestore(LPCTSTR pszSection, BOOL bRectOnly, BOOL bWithPage)
{
	m_sSection = pszSection;
	m_bSavePage = bWithPage;

	m_bEnableSaveRestore = TRUE;
	m_bRectOnly = bRectOnly;

	// restore immediately
	LoadWindowRect(pszSection, bRectOnly);
	{
		LoadPage(pszSection);
		ArrangeLayout();	// needs refresh
	}
}

void CResizableSheet::RefreshLayout()
{
	SendMessage(WM_SIZE);
}

LRESULT CResizableSheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message != WM_NCCALCSIZE || wParam == 0 || !m_bLayoutDone)
		return CPropertySheet::WindowProc(message, wParam, lParam);

	// specifying valid rects needs controls already anchored
	LRESULT lResult = 0;
	HandleNcCalcSize(FALSE, (LPNCCALCSIZE_PARAMS)lParam, lResult);
	lResult = CPropertySheet::WindowProc(message, wParam, lParam);
	HandleNcCalcSize(TRUE, (LPNCCALCSIZE_PARAMS)lParam, lResult);
	return lResult;
}
