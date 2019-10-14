/////////////////////////////////////////////////////////////////////////////////////////
// XeScrollBarBase.cpp  Version 1.1
//     This class is designed as a base class for another class that will 
//     do all painting. This class handles all business logic. No painting
//     is done in this class - except in debug mode.
//
// Author:  Snorri Kristjansson
//          snorrikris@gmail.com
//
// History
//     Version 1.1 - 2010 October 21
//     - Changed base class to CScrollBar (was CWnd).
//     - Fixed many issues to make this scroll bar behave (almost) the same as windows
//       scroll bar.
//
//     Version 1.0 - 2009
//     - Never released.
//
// Acknowledgements:
//     Thanks to Hans Dietrich for his CXScrollBar class,
//     which I used as the starting point for CXeScrollBarBase:
//         http://www.codeproject.com/KB/miscctrl/XScrollBar.aspx
//     (I don't think any of his code survived into this version - but thanks all the same).
//
// License:
//     This software is released into the public domain.  You are free to use
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this
//     software may cause.
//
/////////////////////////////////////////////////////////////////////////////////////////

/****************************************************************************************
TODO:
	H Mouse wheel support (L/R wheel push) - WM_MOUSEHWHEEL (new message Vista and later).

	Change code to pure WIN32 - no MFC - use as base base class for CXeScrollBarBase.
****************************************************************************************/

#include "stdafx.h"
#include "XeScrollBarBase.h"

/****************************************************************************************
Research resources:

Scroll bar MSDN WIN32 reference.
http://msdn.microsoft.com/en-us/library/bb787529(VS.85).aspx

Scroll Bar Controls in Win32
http://msdn.microsoft.com/en-us/library/ms997557.aspx

How/why to handle WM_CANCELMODE message.
http://support.microsoft.com/kb/74548/en-us

How/why to handle WM_GETDLGCODE message.
http://support.microsoft.com/kb/83302

Discussion about thumb size calculations and more (good one).
http://blogs.msdn.com/oldnewthing/archive/2009/09/21/9897553.aspx

Discussion about thumb size calculations.
http://social.msdn.microsoft.com/forums/en-US/wpf/thread/415eacf6-481e-4ebd-a6b0-2953e851183d/

From the November 2001 issue of MSDN Magazine. 
Understanding CControlView, Changing Scroll Bar Color in MFC Apps  
by Paul DiLascia 
http://msdn.microsoft.com/en-us/magazine/cc301457.aspx

Handling Keyboard and Mouse Application Buttons in WTL
By Michael Dunn
http://www.codeproject.com/KB/wtl/WTLAppButtons.aspx

XScrollBar - Scroll bar like Windows Media Player's
By Hans Dietrich
http://www.codeproject.com/KB/miscctrl/XScrollBar.aspx

Developing a Custom Windows Scrollbar in MFC
By Don Metzler, DDJ June 01, 2003
http://www.drdobbs.com/windows/184416659

SkinControls 1.1 - A journey in automating the skinning of Windows controls
by .dan.g.
http://www.codeproject.com/KB/library/SkinCtrl.aspx

Custom Scrollbar Library version 1.1
by James Brown
http://www.codeproject.com/KB/dialog/coolscroll.aspx

Replace a Window's Internal Scrollbar with a customdraw scrollbar Control
By flyhigh
http://www.codeproject.com/KB/dialog/skinscrollbar.aspx
****************************************************************************************/

double fabs_dbl( double dbl )
{
	if( dbl >= 0 )
		return dbl;
	return (-dbl);
}

// Round a double number up or down.
int round_ud_dbl( double dbl )
{
	BOOL bNeg = FALSE;
	if( dbl < 0 )
	{
		bNeg = TRUE;
		dbl = -dbl;
	}
	int n = (int)dbl;
	double fract = dbl - (double)n;
	if( fract > 0.5 )
		n++;
	if( bNeg )
		n = -n;
	return n;
}

///////////////////////////////////////////////////////////////////////////
// CXeScrollBarBase

#define XESCROLLBARWND_CLASSNAME    _T("XeScrollBarWndClass")  // Window class name

#define XSB_LBTN_DOWN_TIMERID	1001
#define XSB_LBTN_DOWN_TIME		200		// mS - time to first auto repeat.
#define XSB_LBTN_REPT_TIMERID	1002
#define XSB_LBTN_REPT_TIME		50		// mS - repeat interval.
#define XSB_FOCUS_TIMERID		1003
#define XSB_FOCUS_TIME			500		// mS - blink 'gripper' interval.

// Menu ID's - Note: same ID's as menu from user32.dll.
#define XSB_IDM_SCRHERE			4100	// Scroll Here		Scroll Here
#define XSB_IDM_SCR_TL			4102	// Top				Left Edge
#define XSB_IDM_SCR_BR			4103	// Bottom			Right Edge
#define XSB_IDM_SCR_PG_UL		4098	// Page Up			Page Left
#define XSB_IDM_SCR_PG_DR		4099	// Page Down		Page Right
#define XSB_IDM_SCR_UL			4096	// Scroll Up		Scroll Left
#define XSB_IDM_SCR_DR			4097	// Scroll Down		Scroll Right


IMPLEMENT_DYNAMIC(CXeScrollBarBase, CScrollBar)

BEGIN_MESSAGE_MAP(CXeScrollBarBase, CScrollBar)
	ON_MESSAGE(SBM_ENABLE_ARROWS, OnSbmEnableArrows)
	ON_MESSAGE(SBM_GETPOS, OnSbmGetPos)
	ON_MESSAGE(SBM_GETRANGE, OnSbmGetRange)
	ON_MESSAGE(SBM_GETSCROLLBARINFO, OnSbmGetScrollBarInfo)
	ON_MESSAGE(SBM_GETSCROLLINFO, OnSbmGetScrollInfo)
	ON_MESSAGE(SBM_SETPOS, OnSbmSetPos)
	ON_MESSAGE(SBM_SETRANGE, OnSbmSetRange)
	ON_MESSAGE(SBM_SETRANGEREDRAW, OnSbmSetRangeRedraw)
	ON_MESSAGE(SBM_SETSCROLLINFO, OnSbmSetScrollInfo)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_KEYDOWN, OnKeyDown)
	ON_MESSAGE(WM_KEYUP, OnKeyUp)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_GETDLGCODE, OnGetDlgCode)
	ON_WM_CANCELMODE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND_RANGE(XSB_IDM_SCR_UL,XSB_IDM_SCR_BR,OnMenuCommands)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_ENABLE()
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CXeScrollBarBase::CXeScrollBarBase()
{
	m_pParent = 0;
	m_bEnabled = m_bHorizontal = m_bDrawGripper = TRUE;
	m_bHasFocus = FALSE;
	m_bNoScroll = m_bDragging = m_bTrackMouseLeave = m_bNeedEndScroll = FALSE;
	m_nPos = m_nTrackPos = m_nMinPos = m_nMaxPos = m_nMaxReportedPos = 0;
	m_uPage = 1;
	m_rectClient.SetRectEmpty();
	m_rectThumb.SetRectEmpty();
	m_rectTLArrow.SetRectEmpty();
	m_rectBRArrow.SetRectEmpty();
	m_rectTLChannel.SetRectEmpty();
	m_rectBRChannel.SetRectEmpty();
	m_uArrowWH = 0;
	m_uThumbMinHW = 8;				// Minimum thumb width or height
	m_dblPx_SU = 0;
	m_ptMenu.SetPoint(0,0);
	m_xyThumbDragOffset = 0;
    if (!SystemParametersInfoA(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, 0)) { //added to support multiple rows per mouse notch
        scrollLines = 3;
    }
}

CXeScrollBarBase::~CXeScrollBarBase()
{
	// Note - base class dtor (CScrollBar) calls DestroyWindow().
}

BOOL CXeScrollBarBase::Create( DWORD dwStyle, const RECT& rect, CWnd *pParentWnd, UINT nID )
{
	if( !RegisterWindowClass() )
		return FALSE;

	ASSERT(pParentWnd);
	ASSERT(IsWindow(pParentWnd->m_hWnd));
	m_pParent = pParentWnd;

	m_bHorizontal = ( dwStyle & SBS_VERT ) ? FALSE : TRUE;
	// Note - SBS_HORZ is defined as 0 in winuser.h.

	m_bEnabled = ( dwStyle & WS_DISABLED ) ? FALSE : TRUE;

	BOOL bResult = CWnd::Create( XESCROLLBARWND_CLASSNAME, 0, dwStyle, rect, 
		pParentWnd, nID );

	if( bResult )
	{
		RecalcRects();
	}
	else
	{
		TRACE(_T("ERROR - failed to create %s\n"),XESCROLLBARWND_CLASSNAME);
		ASSERT(FALSE);
	}

	return bResult;

	// Note - We DO NOT call base class (CScrollBar) because CScrollBar creates 'normal'
	//        windows scrollbar.
}

BOOL CXeScrollBarBase::CreateFromExisting( CWnd *pParentWnd, UINT nID, 
										  BOOL bUseDefaultWH /*= TRUE*/ )
{
	ASSERT(pParentWnd);
	HWND hWndParent = pParentWnd->GetSafeHwnd();
	if( !::IsWindow( hWndParent ) )
	{
		ASSERT(FALSE);
		return FALSE;
	}
	HWND hWndExistingSB = ::GetDlgItem( hWndParent, nID );
	if( !hWndExistingSB )
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	DWORD dwStyle = ::GetWindowLong( hWndExistingSB, GWL_STYLE );

	RECT rect;
	::GetWindowRect( hWndExistingSB, &rect );
	::ScreenToClient( hWndParent, (LPPOINT)&rect );
	::ScreenToClient( hWndParent, ((LPPOINT)&rect) + 1 );
	if( bUseDefaultWH )
	{	// Set width or height to system standard scroll bar width/height.
		if( dwStyle & SBS_VERT )
		{
			// Get width of 'sys' vert. SB.
			int cxSysSbV = ::GetSystemMetrics( SM_CXVSCROLL );
			rect.right = rect.left + cxSysSbV;	// Set width of vert. SB to 'normal'.
		}
		else
		{
			// Get height of 'sys' horz. SB.
			int cySysSbH = ::GetSystemMetrics( SM_CYHSCROLL );
			rect.bottom = rect.top + cySysSbH;	// Set height of horz. SB to 'normal'.
		}
	}

	// Get current range, page and position from existing scrollbar.
	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);     
	info.fMask = SIF_ALL;
	::SendMessage( hWndExistingSB, SBM_GETSCROLLINFO, 0, (LPARAM)&info );

	// Create new scroll bar of 'this' type. Note - Control ID = 0 until old SB destroyed.
	if( !Create( dwStyle, rect, pParentWnd, 0 ) )
		return FALSE;

	// Note - new scroll bar is now the last window in our parent window child Z-order.

	HWND hWndNewSB = GetSafeHwnd();
	ASSERT(hWndNewSB);

	// Set Z-order of new scroll bar - insert after the existing scroll bar.
	::SetWindowPos( hWndNewSB, hWndExistingSB, 0, 0, 0, 0, 
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

	// Destroy existing (old) scroll bar.
	::DestroyWindow( hWndExistingSB );

	// Set Control ID of new scroll bar.
	::SetWindowLong( hWndNewSB, GWL_ID, nID );

	// Set range, page and position parameters in scroll bar.
	SetScrollInfo( &info );

	return TRUE;
}

BOOL CXeScrollBarBase::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	// 'Our' class already registered?
	if( !(::GetClassInfo( hInst, XESCROLLBARWND_CLASSNAME, &wndcls )) )
	{	// Otherwise we need to register a new class
		wndcls.style            = CS_HREDRAW | CS_VREDRAW;
								// Note - CS_DBLCLKS style not used so don't need to
								//        process WM_LBUTTONDBLCLK message.
		wndcls.lpfnWndProc      = ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
		wndcls.hInstance        = hInst;
		wndcls.hIcon            = NULL;
		wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground    = NULL;
		wndcls.lpszMenuName     = NULL;
		wndcls.lpszClassName    = XESCROLLBARWND_CLASSNAME;

		if( !AfxRegisterClass( &wndcls ) )
		{
			TRACE(_T("Register window class %s failed\n"),XESCROLLBARWND_CLASSNAME);
			ASSERT(FALSE);
			AfxThrowResourceException();
			return FALSE;
		}
	}
	return TRUE;
}

void CXeScrollBarBase::DrawScrollBar( CDC* pDC )
{
	// Draw scrollbar as solid color rects - in debug mode ONLY!
#ifdef _DEBUG
#define X_GRBTN RGB(140,140,140)
#define X_GRCH  RGB(180,180,180)
#define X_GRTHM RGB(96,96,96)
#define X_NRMBT	RGB(128,0,128)
#define X_NRMCH RGB(128,128,0)
#define X_NRMTH RGB(0,192,0)
#define X_HOTBT	RGB(192,0,192)
#define X_HOTCH RGB(192,192,0)
#define X_HOTTH RGB(0,255,0)
#define X_DWNBT (~X_NRMBT & 0xFFFFFF)
#define X_DWNCH (~X_NRMCH & 0xFFFFFF)
#define X_DWNTH (~X_NRMTH & 0xFFFFFF)
	COLORREF rgbarr[5][6] = {
//	eNone	eTLbutton	eBRbutton	eTLchannel	eBRchannel	eThumb
//	  0		1			2			3			4			5
	{ 0,	0,			0,			0,			0,			0 },		// 0 eNotDrawn
	{ 0,	X_GRBTN,	X_GRBTN,	X_GRCH,		X_GRCH,		X_GRTHM },	// 1 eDisabled
	{ 0,	X_NRMBT,	X_NRMBT,	X_NRMCH,	X_NRMCH,	X_NRMTH },	// 2 eNormal
	{ 0,	X_DWNBT,	X_DWNBT,	X_DWNCH,	X_DWNCH,	X_DWNTH },	// 3 eDown
	{ 0,	X_HOTBT,	X_HOTBT,	X_HOTCH,	X_HOTCH,	X_HOTTH }	// 4 eHot
	};
	XSB_EDRAWELEM eState;
	const CRect *prcElem = 0;
	for( int nElem = eTLbutton; nElem <= eThumb; nElem++ )
	{
		prcElem = GetUIelementDrawState( (eXSB_AREA)nElem, eState );
		if( !prcElem || eState == eNotDrawn )
			continue;
		COLORREF rgb = rgbarr[eState][nElem];
		pDC->FillSolidRect( prcElem, rgb );
	}
#endif
}

// SBM_GETPOS message handler.
LRESULT CXeScrollBarBase::OnSbmGetPos( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	return m_nPos;
}

// SBM_SETPOS message handler.
LRESULT CXeScrollBarBase::OnSbmSetPos( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	int nPos = (int)wparam;
	BOOL bRedraw = (BOOL)lparam;
	int nOldPos = m_nPos;

	m_nPos = nPos;

	if( m_nPos < m_nMinPos )
		m_nPos = m_nMinPos;
	else if( m_nPos > m_nMaxReportedPos )
		m_nPos = m_nMaxReportedPos;

	if( m_bNoScroll && !m_bEnabled )	// SB disabled because of SIF_DISABLENOSCROLL?
	{	// SBM_SETPOS cancels SIF_DISABLENOSCROLL.
		m_bNoScroll = FALSE;
		EnableWindow( TRUE );
	}

	RecalcRects();

	if( bRedraw )
		Invalidate();

	return nOldPos;
}

// SBM_GETRANGE message handler.
LRESULT CXeScrollBarBase::OnSbmGetRange( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	LPINT lpMinPos = (LPINT)wparam;
	LPINT lpMaxPos = (LPINT)lparam;
	*lpMinPos = m_nMinPos;
	*lpMaxPos = m_nMaxPos;
	return 0;
}

// SBM_SETRANGE message handler.
LRESULT CXeScrollBarBase::OnSbmSetRange( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	int nMinPos = (int)wparam;
	int nMaxPos = (int)lparam;
	m_nMinPos = nMinPos;
	m_nMaxPos = nMaxPos;
	if( m_nMaxPos < m_nMinPos )
		m_nMaxPos = m_nMinPos;
	int nSUrange = abs(m_nMaxPos - m_nMinPos) + 1;
	if( m_uPage > (UINT)nSUrange )
		m_uPage = (UINT)nSUrange;

	if( m_uPage == 0 )
		m_nMaxReportedPos = m_nMaxPos;
	else
		m_nMaxReportedPos = m_nMaxPos - ((int)m_uPage - 1);

	int nOldPos = m_nPos;
	if( m_nPos < m_nMinPos )
		m_nPos = m_nMinPos;
	else if( m_nPos > m_nMaxReportedPos )
		m_nPos = m_nMaxReportedPos;

	if( m_bNoScroll && !m_bEnabled )	// SB disabled because of SIF_DISABLENOSCROLL?
	{	// SBM_SETRANGE cancels SIF_DISABLENOSCROLL.
		m_bNoScroll = FALSE;
		EnableWindow( TRUE );
	}

	RecalcRects();

	if( nOldPos != m_nPos )
		return nOldPos;
	return 0;
}

// SBM_SETRANGEREDRAW message handler.
LRESULT CXeScrollBarBase::OnSbmSetRangeRedraw( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	LRESULT lResult = OnSbmSetRange( wparam, lparam );
	Invalidate();
	return lResult;
}

// SBM_GETSCROLLINFO message handler.
LRESULT CXeScrollBarBase::OnSbmGetScrollInfo( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	LPSCROLLINFO lpScrollInfo = (LPSCROLLINFO)lparam;
	if( lpScrollInfo->cbSize != sizeof(SCROLLINFO) )
		return FALSE;
	lpScrollInfo->nMin = m_nMinPos;
	lpScrollInfo->nMax = m_nMaxPos;
	lpScrollInfo->nPage = m_uPage;
	lpScrollInfo->nPos = m_nPos;
	lpScrollInfo->nTrackPos = m_nTrackPos;
	return TRUE;
}

// SBM_SETSCROLLINFO message handler.
LRESULT CXeScrollBarBase::OnSbmSetScrollInfo( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	BOOL bRedraw = (BOOL)wparam;
	LPSCROLLINFO lpScrollInfo = (LPSCROLLINFO)lparam;
	if( lpScrollInfo->cbSize != sizeof(SCROLLINFO) )
		return 0;
	if( lpScrollInfo->fMask & SIF_PAGE )
	{
		m_uPage = lpScrollInfo->nPage;
		// Note - windows scrollbars can have a page size = 0.
	}
	if( lpScrollInfo->fMask & SIF_RANGE )
	{
		m_nMinPos = lpScrollInfo->nMin;
		m_nMaxPos = lpScrollInfo->nMax;
	}
	if( lpScrollInfo->fMask & SIF_POS )
	{
		m_nPos = lpScrollInfo->nPos;
	}
	if( lpScrollInfo->fMask & SIF_DISABLENOSCROLL )
	{
		BOOL bEnable = ( (int)m_uPage < (m_nMaxPos - m_nMinPos) ) ? TRUE : FALSE;
		m_bNoScroll = !bEnable;
		EnableWindow( bEnable );
	}

	if( m_nMaxPos < m_nMinPos )
		m_nMaxPos = m_nMinPos;
	int nSUrange = abs(m_nMaxPos - m_nMinPos) + 1;
	if( m_uPage > (UINT)nSUrange )
		m_uPage = (UINT)nSUrange;

	if( m_uPage == 0 )
		m_nMaxReportedPos = m_nMaxPos;
	else
		m_nMaxReportedPos = m_nMaxPos - ((int)m_uPage - 1);

	if( m_nPos < m_nMinPos )
		m_nPos = m_nMinPos;
	else if( m_nPos > m_nMaxReportedPos )
		m_nPos = m_nMaxReportedPos;

	RecalcRects();

	if( bRedraw )
		Invalidate();

	return m_nPos;
}

// SBM_ENABLE_ARROWS message handler.
LRESULT CXeScrollBarBase::OnSbmEnableArrows( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	/* ImplNote - during testing the windows scrollbar behaved strangely when only one
	button was disabled. For that reason only enable/disable both is supported here. */
	EnableWindow( ( wparam & ESB_DISABLE_BOTH ) ? FALSE : TRUE );
	// wParam Specifies whether the scroll bar arrows are enabled or disabled and 
	// indicates which arrows are enabled or disabled. 
	return TRUE;
}

// SBM_GETSCROLLBARINFO message handler.
LRESULT CXeScrollBarBase::OnSbmGetScrollBarInfo( WPARAM wparam, LPARAM lparam )
{
	ASSERT(::IsWindow(m_hWnd));
	SCROLLBARINFO *psbi = (SCROLLBARINFO *)lparam;
	if( !psbi || psbi->cbSize != sizeof(SCROLLBARINFO) )
		return FALSE;

	/* Note - information on how to implement this is a little sparse from MS.
	Need make a few educated guesses. 
	Note - testing (comparing 'this' to WIN SBs) has shown that:
			rcScrollBar is in screen coords.
			dxyLineButton is arrow button width when horz. SB.
			dxyLineButton is arrow button height when vert. SB.
	*/

	psbi->rcScrollBar = m_rectClient;				// Coordinates of the scroll bar.
	ClientToScreen( &psbi->rcScrollBar );			// In screen coords.

	if( m_bHorizontal )
	{
		psbi->dxyLineButton = m_rectTLArrow.Width();// arrow button width.
		psbi->xyThumbTop = m_rectThumb.left;		// Position of the left of the thumb. 
		psbi->xyThumbBottom = m_rectThumb.right;	// Position of the right of the thumb. 
	}
	else
	{
		psbi->dxyLineButton = m_rectTLArrow.Height();// arrow button height.
		psbi->xyThumbTop = m_rectThumb.top;			// Position of the top of the thumb. 
		psbi->xyThumbBottom = m_rectThumb.bottom;	// Position of the bottom of the thumb. 
	}

	// psbi->rgstate - An array of DWORD elements. Each element indicates the state of a 
	// scroll bar component. The following values show the scroll bar component that 
	// corresponds to each array index. Index Scroll bar component 
	// 0 The scroll bar itself. 
	// 1 The top or right arrow button. 
	// 2 The page up or page right region. 
	// 3 The scroll box (thumb). 
	// 4 The page down or page left region. 
	// 5 The bottom or left arrow button. 
	DWORD dwState = ( m_bEnabled ) ? 0 : STATE_SYSTEM_UNAVAILABLE;
	DWORD dwTLchSt = dwState, dwBRchSt = dwState, dwThumbSt = dwState;
	DWORD dwTLbtnSt = dwState, dwBRbtnSt = dwState;
	if( m_rectTLChannel.IsRectEmpty() )
		dwTLchSt |= STATE_SYSTEM_INVISIBLE;
	if( m_rectBRChannel.IsRectEmpty() )
		dwBRchSt |= STATE_SYSTEM_INVISIBLE;
	if( m_rectThumb.IsRectEmpty() )
		dwThumbSt |= STATE_SYSTEM_INVISIBLE;
	if( m_eMouseDownArea.IsTLButton() )
		dwTLbtnSt |= STATE_SYSTEM_PRESSED;
	if( m_eMouseDownArea.IsTLChannel() )
		dwTLchSt |= STATE_SYSTEM_PRESSED;
	if( m_eMouseDownArea.IsThumb() )
		dwThumbSt |= STATE_SYSTEM_PRESSED;
	if( m_eMouseDownArea.IsBRChannel() )
		dwBRchSt |= STATE_SYSTEM_PRESSED;
	if( m_eMouseDownArea.IsBRButton() )
		dwBRbtnSt |= STATE_SYSTEM_PRESSED;
	psbi->rgstate[0] = dwState;
	psbi->rgstate[1] = dwTLbtnSt;
	psbi->rgstate[2] = dwTLchSt;
	psbi->rgstate[3] = dwThumbSt;
	psbi->rgstate[4] = dwBRchSt;
	psbi->rgstate[5] = dwBRbtnSt;

	// The DWORD element for each scroll bar component can include a combination of the 
	// following bit flags.
	// STATE_SYSTEM_INVISIBLE - For the scroll bar itself, indicates the specified 
	//		vertical or horizontal scroll bar does not exist. For the page up or page 
	//		down regions, indicates the thumb is positioned such that the region does 
	//		not exist.
	// STATE_SYSTEM_OFFSCREEN - For the scroll bar itself, indicates the window is sized 
	//		such that the specified vertical or horizontal scroll bar is not currently 
	//		displayed. (SK note - applies to NC scroll bars only).
	// STATE_SYSTEM_PRESSED - The arrow button or page region is pressed.
	// STATE_SYSTEM_UNAVAILABLE - The component is disabled.

	return TRUE;
}

void CXeScrollBarBase::OnPaint()
{	// WM_PAINT message handler.
	ASSERT(::IsWindow(m_hWnd));
	CPaintDC dc(this); // device context for painting

	DrawScrollBar( &dc );
	// Do not call CScrollBar::OnPaint() for painting messages
}

BOOL CXeScrollBarBase::OnEraseBkgnd( CDC* pDC )
{	// WM_ERASEBKGND message handler.
	return TRUE;	// All painting done in OnPaint.
}

LRESULT CXeScrollBarBase::OnKeyDown( WPARAM wParam, LPARAM lParam )
{	// WM_KEYDOWN message handler.
	ASSERT(::IsWindow(m_hWnd));
	WORD wSBcode = 0xFFFF;
	// Note - SB_PAGELEFT == SB_PAGEUP etc. see winuser.h
	switch( wParam )
	{
	case VK_PRIOR:
		wSBcode = SB_PAGELEFT;
		break;

	case VK_NEXT:
		wSBcode = SB_PAGERIGHT;
		break;

	case VK_HOME:
		wSBcode = SB_LEFT;
		break;

	case VK_END:
		wSBcode = SB_RIGHT;
		break;

	case VK_LEFT:
		wSBcode = SB_LINELEFT;
		break;

	case VK_RIGHT:
		wSBcode = SB_LINERIGHT;
		break;

	case VK_UP:
		wSBcode = SB_LINEUP;
		break;

	case VK_DOWN:
		wSBcode = SB_LINEDOWN;
		break;
	}

	if( wSBcode != 0xFFFF )
	{
		SendScrollMsg( wSBcode );
		m_bNeedEndScroll = TRUE;	// Send SB_ENDSCROLL on WM_KEYUP.
	}

	return 0;	// Indicate we processed this message (eat all key msgs).
	// Note - testing shows that windows looks for another child control to process
	// keyboard input if we don't return 0 here (and we lose focus).
}

LRESULT CXeScrollBarBase::OnKeyUp( WPARAM wParam, LPARAM lParam )
{	// WM_KEYUP message handler.
	if( m_bNeedEndScroll )
	{
		SendScrollMsg( SB_ENDSCROLL );
		m_bNeedEndScroll = FALSE;
	}
	return 0;	// Indicate we processed this message (eat all key msgs).
}

void CXeScrollBarBase::OnLButtonDown( UINT nFlags, CPoint point )
{	// WM_LBUTTONDOWN message handler.
	ASSERT(::IsWindow(m_hWnd));
	SetCapture();
	BOOL bHasTabStop = ( ::GetWindowLong( m_hWnd, GWL_STYLE ) & WS_TABSTOP ) ? TRUE : FALSE;
	if( bHasTabStop )
		SetFocus();		// Only 'steal' focus if 'this' has Tab stop.

	m_eMouseDownArea = GetAreaFromPoint( point );

	WORD wSBcode = 0xFFFF;
	// Note - SB_PAGELEFT == SB_PAGEUP etc. see winuser.h
	if( m_eMouseDownArea.eArea == eTLbutton )
		wSBcode = SB_LINELEFT;
	else if( m_eMouseDownArea.eArea == eTLchannel )
		wSBcode = SB_PAGELEFT;
	else if( m_eMouseDownArea.eArea == eBRchannel )
		wSBcode = SB_PAGERIGHT;
	else if( m_eMouseDownArea.eArea == eBRbutton )
		wSBcode = SB_LINERIGHT;

	if( wSBcode != 0xFFFF )
	{
		SendScrollMsg( wSBcode );
		m_bNeedEndScroll = TRUE;	// Send SB_ENDSCROLL on WM_LBUTTONUP message.
	}
	
	if( m_eMouseDownArea.IsThumb() )	// Store X or Y offset from thumb edge.
		m_xyThumbDragOffset = ( m_bHorizontal ) ? point.x - m_rectThumb.left 
		: point.y - m_rectThumb.top;

	// Set timer for first auto repeat - when button or channel clicked.
	if( m_eMouseDownArea.IsButton() || m_eMouseDownArea.IsChannel() )
		SetTimer( XSB_LBTN_DOWN_TIMERID, XSB_LBTN_DOWN_TIME, 0 );

	CScrollBar::OnLButtonDown(nFlags, point);
}

void CXeScrollBarBase::OnLButtonUp( UINT nFlags, CPoint point )
{	// WM_LBUTTONUP message handler.
	ASSERT(::IsWindow(m_hWnd));
	ReleaseCapture();
	KillTimer( XSB_LBTN_DOWN_TIMERID );
	KillTimer( XSB_LBTN_REPT_TIMERID );

	m_eMouseDownArea = eNone;

	if( m_bDragging )					// Did we send any SB_THUMBTRACK messages?
	{
		SendScrollMsg( SB_THUMBPOSITION, m_nTrackPos );
		m_bDragging = FALSE;
		RecalcRects();	// Reset thumb pos. to current scroll pos.
	}

	if( m_bNeedEndScroll )
	{
		SendScrollMsg( SB_ENDSCROLL );	// Let parent know scrolling has ended.
		m_bNeedEndScroll = FALSE;
	}

	Invalidate();

	CScrollBar::OnLButtonUp(nFlags, point);
}

/* updated to newer api and reversing return value to avoid mousewheel propagating*/
/* remove restriction on vert scrollbar, as both are valid in win32*/
BOOL CXeScrollBarBase::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
    ASSERT(::IsWindow(m_hWnd));
    short xPos = pt.x;
    short yPos = pt.y;

    //if (!m_bHorizontal)	// Mouse wheel messages only apply to vertical scrollbar.

    {
        WORD wSBcode = 0xFFFF;
        if (zDelta >= WHEEL_DELTA) {
            wSBcode = SB_LINEUP;
        } else if (zDelta <= -WHEEL_DELTA) {
            wSBcode = SB_LINEDOWN;
            zDelta = -zDelta;
        }
        if (wSBcode != 0xFFFF) {
            do {
                for (UINT i=0; i<scrollLines; i++) //windows scrolls more than 1 line per tick
                    SendScrollMsg(wSBcode);
            } while ((zDelta -= WHEEL_DELTA) >= WHEEL_DELTA);
            SendScrollMsg(SB_ENDSCROLL);
        }
        return 1;	// Message was processed. (was 0, but per https://msdn.microsoft.com/en-us/data/eff58fe7(v=vs.85) should be 1 if scrolling enabled
    }
    return 0;	// Message not processed.  (was 1, but per https://msdn.microsoft.com/en-us/data/eff58fe7(v=vs.85) should be 0 if scrolling not enabled
}


void CXeScrollBarBase::OnMouseMove( UINT nFlags, CPoint point )
{	// WM_MOUSEMOVE message handler.
	ASSERT(::IsWindow(m_hWnd));
	if( !m_bTrackMouseLeave )
	{	// We want a WM_MOUSELEAVE message when mouse leaves our client area.
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 0;

		// Note - _TrackMouseEvent is from comctl32.dll - emulates TrackMouseEvent
		// if it does not exist.
		m_bTrackMouseLeave = _TrackMouseEvent( &tme );
	}

	stXSB_AREA eOldMouseArea = m_eMouseOverArea;

	m_eMouseOverArea = GetAreaFromPoint( point );	// Update mouse over area.

	if( m_eMouseDownArea.IsThumb() && m_dblPx_SU > 0 )
	{	// User is dragging the thumb.
		BOOL bStartDrag = FALSE;
		if( !m_bDragging )
		{
			bStartDrag = TRUE;	// Start of thumb dragging.
			m_bDragging = TRUE;
		}
		int nTrackPos;
		double dblScrollPos;
		if( m_bHorizontal )
		{	// X pos of left edge of thumb (0...?)
			int xPos = point.x - m_xyThumbDragOffset - m_rectTLArrow.right;
			dblScrollPos = (double)xPos / m_dblPx_SU;
			nTrackPos = round_ud_dbl( dblScrollPos ) + m_nMinPos;
		}
		else
		{	// Y pos of top edge of thumb (0...?)
			int yPos = point.y - m_xyThumbDragOffset - m_rectTLArrow.bottom;
			dblScrollPos = (double)yPos / m_dblPx_SU;
			nTrackPos = round_ud_dbl( dblScrollPos ) + m_nMinPos;
		}
		if( nTrackPos < m_nMinPos )
			nTrackPos = m_nMinPos;
		else if( nTrackPos > m_nMaxReportedPos )
			nTrackPos = m_nMaxReportedPos;

        //adipose: moved this block to before sending scroll message.  otherwise scrollbar updates poorly on slower windows (listctrl)
        //also call updatewindow to redraw immediately
        // Recalculate thumb XY pos. and redraw if pos. changed.
        if (RecalcRectsThumbTrack(point)) {
            Invalidate();
            UpdateWindow();
        }
            
        if( bStartDrag || m_nTrackPos != nTrackPos )
		{	// Send scroll message when user starts dragging
			// OR when track pos has changed.
			m_nTrackPos = nTrackPos;
			SendScrollMsg( SB_THUMBTRACK, m_nTrackPos );
			m_bNeedEndScroll = TRUE;
		}
	}

	if( m_eMouseOverArea != eOldMouseArea )
		Invalidate();

	CScrollBar::OnMouseMove(nFlags, point);
}

LRESULT CXeScrollBarBase::OnMouseLeave( WPARAM wparam, LPARAM lparam )
{	// WM_MOUSELEAVE message handler.
	ASSERT(::IsWindow(m_hWnd));
	m_bTrackMouseLeave = FALSE;
	m_eMouseOverArea = eNone;
	Invalidate();
	return 0;
}

LRESULT CXeScrollBarBase::OnGetDlgCode( WPARAM wParam, LPARAM lParam )
{	// WM_GETDLGCODE message handler.
	ASSERT(::IsWindow(m_hWnd));
	BOOL bHasTabStop = ( ::GetWindowLong( m_hWnd, GWL_STYLE ) & WS_TABSTOP ) ? TRUE : FALSE;

	LRESULT lResult = Default();
	if( lParam )	// lParam points to an MSG structure?
	{
		LPMSG lpmsg = (LPMSG)lParam;
		if( (lpmsg->message == WM_KEYDOWN	// Keyboard input?
			|| lpmsg->message == WM_KEYUP)
			&& lpmsg->wParam != VK_TAB )	// AND NOT TAB key?
		{
			if( bHasTabStop )				// 'this' window has Tab stop?
			{
				lResult |= DLGC_WANTMESSAGE;	// We want keyboard input (except TAB).
				// Note - windows will set focus to 'this' (and send WM_SETFOCUS)
				//        if we return DLGC_WANTMESSAGE here.
			}
			else
			{	// 'this' windows does NOT have TAB stop.
				// Note - windows standard scroll bar implements a special behaviour
				// for scroll bars when no tab stop for the UP, DOWN, LEFT, RIGHT keys.
				if( m_bHorizontal )
				{
					if( lpmsg->wParam == VK_LEFT || lpmsg->wParam == VK_RIGHT )
						lResult |= DLGC_WANTMESSAGE;
				}
				else
				{
					if( lpmsg->wParam == VK_UP || lpmsg->wParam == VK_DOWN )
						lResult |= DLGC_WANTMESSAGE;
				}
			}
		}
	}
	else
	{
		if( bHasTabStop )
			lResult |= DLGC_WANTTAB;	// 'this' has WS_TABSTOP style - we want focus.
		else
			lResult |= DLGC_STATIC;		// no tab stop - we don't want focus.
	}
	return lResult;
}

void CXeScrollBarBase::OnCancelMode()
{	// WM_CANCELMODE message handler.
	ASSERT(::IsWindow(m_hWnd));
	CScrollBar::OnCancelMode();

	// Need to handle WM_CANCELMODE message.
	// See -> http://support.microsoft.com/kb/74548/en-us

	if( !m_eMouseDownArea.IsNone() )	// Mouse L button down?
		OnLButtonUp( 0, CPoint(0,0) );	// Do L btn up processing now.
}

void CXeScrollBarBase::OnContextMenu( CWnd* pWnd, CPoint point )
{	// WM_CONTEXTMENU message handler.
	ASSERT(::IsWindow(m_hWnd));

	m_ptMenu = point;
	ScreenToClient( &m_ptMenu );

	// Try to load scroll bar menu from user32.dll - to get menu in 'local' language.
	CString strDllPathName;
	GetWindowsDirectory( strDllPathName.GetBuffer(MAX_PATH), MAX_PATH );
	strDllPathName.ReleaseBuffer();
	strDllPathName += _T("\\system32\\user32.dll");

	HMODULE hUser32dllModule = ::LoadLibrary( strDllPathName );
	if( hUser32dllModule ) 
	{	// Get menu #64 (horz.) or menu #80 (vert.) from user32.dll.
		HMENU hMenu = 0, hSubMenu = 0;
		if( m_bHorizontal )
			hMenu = ::LoadMenu( hUser32dllModule, MAKEINTRESOURCE(64) );
		else
			hMenu = ::LoadMenu( hUser32dllModule, MAKEINTRESOURCE(80) );
		if( hMenu )
		{
			hSubMenu = ::GetSubMenu( hMenu, 0 );
			if( hSubMenu )
			{
				::TrackPopupMenu( hSubMenu, TPM_LEFTALIGN, point.x, point.y, 0,
					GetSafeHwnd(), 0 );
				// Note - TrackPopupMenu does not return until menu has been dismissed.
				::DestroyMenu( hMenu );
			}
		}
		::FreeLibrary( hUser32dllModule );
		if( hSubMenu )
			return;	// Using menu from user32.dll was successful.
	}

	// If using menu from user32.dll was unsuccessful - create menu (in english).
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu( MF_STRING,			XSB_IDM_SCRHERE,	_T("Scroll Here") );
	menu.AppendMenu( MF_SEPARATOR,		0,					_T("") );
	if( m_bHorizontal )
	{
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_TL,		_T("Left Edge") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_BR,		_T("Right Edge") );
		menu.AppendMenu( MF_SEPARATOR,	0,					_T("") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_PG_UL,	_T("Page Left") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_PG_DR,	_T("Page Right") );
		menu.AppendMenu( MF_SEPARATOR,	0,					_T("") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_UL,		_T("Scroll Left") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_DR,		_T("Scroll Right") );
	}
	else
	{
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_TL,		_T("Top") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_BR,		_T("Bottom") );
		menu.AppendMenu( MF_SEPARATOR,	0,					_T("") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_PG_UL,	_T("Page Up") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_PG_DR,	_T("Page Down") );
		menu.AppendMenu( MF_SEPARATOR,	0,					_T("") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_UL,		_T("Scroll Up") );
		menu.AppendMenu( MF_STRING,		XSB_IDM_SCR_DR,		_T("Scroll Down") );
	}
	menu.TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, pWnd );
	// Note - TrackPopupMenu does not return until menu has been dismissed.
	menu.DestroyMenu();
}

void CXeScrollBarBase::OnMenuCommands( UINT uID )
{
	ASSERT(::IsWindow(m_hWnd));
	int xyOfs, nScrollHerePos;
	double dblScrollPos;
	WORD wSBcode = 0xFFFF;
	switch( uID )
	{
	case XSB_IDM_SCRHERE:
		// Calculate pos (in scroll units)
		if( m_bHorizontal )
			xyOfs = m_ptMenu.x - m_rectTLArrow.right;
		else
			xyOfs = m_ptMenu.y - m_rectTLArrow.bottom;
		if( xyOfs < 0 )
			xyOfs = 0;
		if( m_rectThumb.IsRectEmpty() || !(m_dblPx_SU > 0) )
			break;	// Can't 'Scroll Here'.
		dblScrollPos = (double)xyOfs / m_dblPx_SU;
		nScrollHerePos = m_nMinPos + round_ud_dbl( dblScrollPos );
		if( nScrollHerePos < m_nMinPos )
			nScrollHerePos = m_nMinPos;
		else if( nScrollHerePos > m_nMaxReportedPos )
			nScrollHerePos = m_nMaxReportedPos;
		m_nTrackPos = nScrollHerePos;
		SendScrollMsg( SB_THUMBPOSITION, m_nTrackPos );
		SendScrollMsg( SB_ENDSCROLL );
		break;
	case XSB_IDM_SCR_TL:
		wSBcode = SB_TOP;
		break;
	case XSB_IDM_SCR_BR:
		wSBcode = SB_BOTTOM;
		break;
	case XSB_IDM_SCR_PG_UL:
		wSBcode = SB_PAGEUP;
		break;
	case XSB_IDM_SCR_PG_DR:
		wSBcode = SB_PAGEDOWN;
		break;
	case XSB_IDM_SCR_UL:
		wSBcode = SB_LINEUP;
		break;
	case XSB_IDM_SCR_DR:
		wSBcode = SB_LINEDOWN;
		break;
	}
	if( wSBcode != 0xFFFF )
	{
		SendScrollMsg( wSBcode );
		SendScrollMsg( SB_ENDSCROLL );
	}
}

void CXeScrollBarBase::OnTimer( UINT_PTR nIDEvent )
{	// WM_TIMER message handler.
	ASSERT(::IsWindow(m_hWnd));
	if( nIDEvent == XSB_LBTN_DOWN_TIMERID )
	{	// First auto repeat timer event.
		KillTimer( XSB_LBTN_DOWN_TIMERID );
		SetTimer( XSB_LBTN_REPT_TIMERID, XSB_LBTN_REPT_TIME, 0 );
	}
	if( nIDEvent == XSB_LBTN_DOWN_TIMERID || nIDEvent == XSB_LBTN_REPT_TIMERID )
	{	// Auto repeat
		CPoint ptCurMouse;
		if( ::GetCursorPos( &ptCurMouse ) )
		{
			::ScreenToClient( GetSafeHwnd(), &ptCurMouse );
			m_eMouseOverArea = GetAreaFromPoint( ptCurMouse );	// Update mouse over area.
		}
		if( m_eMouseDownArea.IsTLButton() )
		{
			if( m_eMouseOverArea.IsTLButton() )	// Mouse still over button?
				SendScrollMsg( SB_LINELEFT );
		}
		else if( m_eMouseDownArea.IsBRButton() )
		{
			if( m_eMouseOverArea.IsBRButton() )	// Mouse still over button?
				SendScrollMsg( SB_LINERIGHT );
		}
		else if( m_eMouseDownArea.IsTLChannel() )
		{
			if( m_eMouseOverArea.IsTLChannel() )	// Mouse still over channel?
				SendScrollMsg( SB_PAGELEFT );
		}
		else if( m_eMouseDownArea.IsBRChannel() )
		{
			if( m_eMouseOverArea.IsBRChannel() )	// Mouse still over channel?
				SendScrollMsg( SB_PAGERIGHT );
		}
		// Note - SB_LINELEFT == SB_LINEUP etc. see winuser.h
	}
	if( nIDEvent == XSB_FOCUS_TIMERID )
	{	// Blinking focus timer.
		if( m_bNeedEndScroll )
		{	// Draw normal thumb box while user is scrolling.
			if( !m_bDrawGripper )
			{	// Redraw scroll bar if currently drawn without 'gripper'.
				m_bDrawGripper = TRUE;
				Invalidate();	// Draw 'blinking' focus.
			}
		}
		else
		{	// Draw blinking 'gripper' to indicate 'focus'.
			m_bDrawGripper = !m_bDrawGripper;
			Invalidate();	// Draw 'blinking' focus.
		}
	}

	CScrollBar::OnTimer( nIDEvent );
}

void CXeScrollBarBase::OnSize( UINT nType, int cx, int cy ) 
{	// WM_SIZE message handler.
	ASSERT(::IsWindow(m_hWnd));
	CScrollBar::OnSize( nType, cx, cy );
	
	if( m_hWnd )
		RecalcRects();
}

void CXeScrollBarBase::OnSetFocus( CWnd* pOldWnd ) 
{	// WM_SETFOCUS message handler.
	ASSERT(::IsWindow(m_hWnd));
	CScrollBar::OnSetFocus( pOldWnd );
	m_bHasFocus = TRUE;
	m_bDrawGripper = FALSE;
	SetTimer( XSB_FOCUS_TIMERID, XSB_FOCUS_TIME, 0 );
	Invalidate();	
}

void CXeScrollBarBase::OnKillFocus( CWnd* pNewWnd ) 
{	// WM_KILLFOCUS message handler.
	ASSERT(::IsWindow(m_hWnd));
	CScrollBar::OnKillFocus( pNewWnd );
	m_bHasFocus = FALSE;
	m_bDrawGripper = TRUE;
	KillTimer( XSB_FOCUS_TIMERID );
	Invalidate();	
}

void CXeScrollBarBase::OnEnable( BOOL bEnable )
{	// WM_ENABLE message handler
	ASSERT(::IsWindow(m_hWnd));
	CScrollBar::OnEnable(bEnable);

	m_bEnabled = bEnable;

	RecalcRects();	// Need to recalc. because no thumb is shown when disabled.
	
	Invalidate();	
}

void CXeScrollBarBase::SendScrollMsg( WORD wSBcode, WORD wHiWPARAM /*= 0*/ )
{
	ASSERT(::IsWindow(m_hWnd));
	if( m_pParent && ::IsWindow( m_pParent->m_hWnd ) )
	{
		m_pParent->SendMessage( ( m_bHorizontal ) ? WM_HSCROLL : WM_VSCROLL, 
			MAKELONG(wSBcode,wHiWPARAM), (LPARAM)m_hWnd );
	}
}

eXSB_AREA CXeScrollBarBase::GetAreaFromPoint( CPoint point )
{
	ASSERT(::IsWindow(m_hWnd));
	if( !m_rectClient.PtInRect( point ) )
		return eNone;
	if( m_rectThumb.PtInRect( point ) )
		return eThumb;
	if( m_rectTLArrow.PtInRect( point ) )
		return eTLbutton;
	if( m_rectBRArrow.PtInRect( point ) )
		return eBRbutton;
	if( m_rectTLChannel.PtInRect( point ) )
		return eTLchannel;
	if( m_rectBRChannel.PtInRect( point ) )
		return eBRchannel;
	return eNone;
}

const CRect *CXeScrollBarBase::GetUIelementDrawState( eXSB_AREA eElem, 
													 XSB_EDRAWELEM &eState )
{
	ASSERT(::IsWindow(m_hWnd));
	CRect *prcElem = 0;
	eState = eNotDrawn;
	switch( eElem )
	{
	case eTLbutton:
		prcElem = &m_rectTLArrow;
		break;
	case eBRbutton:
		prcElem = &m_rectBRArrow;
		break;
	case eTLchannel:
		prcElem = &m_rectTLChannel;
		break;
	case eBRchannel:
		prcElem = &m_rectBRChannel;
		break;
	case eThumb:
		prcElem = &m_rectThumb;
		break;
	}
	if( !prcElem || prcElem->IsRectEmpty() )
		eState = eNotDrawn;
	if( !m_bEnabled )
		eState = eDisabled;
	else if( m_eMouseDownArea.eArea == eElem )
		eState = eDown;
	else if( m_eMouseOverArea.eArea == eElem && !m_bDragging)
		eState = eHot;
	else
		eState = eNormal;
	return prcElem;
}

BOOL CXeScrollBarBase::CalcThumb( int cxyChannel, int &xyThumb, int &cxyThumb )
{
	// Range in 'scroll units' (SU) - Note +1 because min/max are 'inclusive' values.
	int nSU_Range = abs(m_nMaxPos - m_nMinPos) + 1;
	if( nSU_Range == 0							// No thumb when scroll range is 0
		|| cxyChannel <= (int)m_uThumbMinHW		// No space for thumb.
		|| !m_bEnabled )						// No thumb when disabled.
		return FALSE;	

	// We have space for thumb.

	// thumb size = scroll bar size * page size      / scroll bar range 
	// (pixels)     (pixels)          (scroll units)   (scroll units)

	// When page size is 0 thumb size is set to minimum size.

	m_dblPx_SU = (double)cxyChannel / (double)nSU_Range;	// Pixels per scroll unit.

	double dblXY = (double)(m_nPos - m_nMinPos) * m_dblPx_SU;
	xyThumb = (int)dblXY;
	if( fabs_dbl( dblXY - (double)xyThumb ) > 0.5 )
		xyThumb++;

	double dblCXY = (double)m_uPage * m_dblPx_SU;
	cxyThumb = (int)dblCXY;
	if( fabs_dbl( dblCXY - (double)cxyThumb ) > 0.5 )
		cxyThumb++;

	//if( m_uPage == 0 )
	//	cxyThumb = GetCXYarrow();	// Thumb is same as arrow button when page = 0.
	// Note - WIN SBs show thumb box same size as arrow button when PAGE = 0.

	if( cxyThumb < (int)m_uThumbMinHW )
	{
		int nErrCXY = (int)m_uThumbMinHW - cxyThumb;
		cxyThumb = (int)m_uThumbMinHW;

		// Calculate new thumb X or Y position when 'error' in position.
		double dblErr_Px = (double)nErrCXY / (double)cxyChannel;
		double dblXYoffs = dblErr_Px * xyThumb;
		int xyOffs = (int)dblXYoffs;
		if( fabs_dbl( dblXYoffs - (double)xyOffs ) > 0.5 )
			xyOffs++;
		xyThumb -= xyOffs;
	}

	// Sometimes it's needed to adjust the size and or position because scroll bar
	// parameters are in error. 

	// Calculate last possible X or Y for thumb.
	int xyLastPossible = cxyChannel - cxyThumb;
	if( xyThumb > xyLastPossible )
		xyThumb = xyLastPossible;

	if( xyThumb < 0 )
		xyThumb = 0;

	if( (xyThumb + cxyThumb) > cxyChannel )
		cxyThumb = cxyChannel - xyThumb;

	return TRUE;
}

void CXeScrollBarBase::RecalcRects()
{
	if( !GetSafeHwnd() )
		return;

	GetClientRect( &m_rectClient );	// Update client rect.

	BOOL bHasThumb = FALSE;
	m_rectThumb.SetRectEmpty();

	if( m_bHorizontal )
	{	// Calc. arrows
		int cxClient = m_rectClient.Width();
		int cxArrow = GetCXYarrow();	// Arrow button width.

		if( cxClient < (2 * cxArrow) )
			cxArrow = cxClient / 2;	// Limit arrow width to available area.

		m_rectTLArrow.SetRect( m_rectClient.left, m_rectClient.top,
			m_rectClient.left + cxArrow, m_rectClient.bottom);

		m_rectBRArrow.SetRect( m_rectClient.right - cxArrow, m_rectClient.top,
			m_rectClient.right, m_rectClient.bottom );

		// Calc. thumb size and position
		int xThumb, cxThumb, cxChannel = cxClient - (2 * cxArrow);
		bHasThumb = CalcThumb( cxChannel, xThumb, cxThumb );
		if( bHasThumb )
		{	// We have space for thumb.
			xThumb += m_rectTLArrow.right;
			m_rectThumb = m_rectTLArrow;
			m_rectThumb.left = xThumb;
			m_rectThumb.right = xThumb + cxThumb;
		}

		// Calc. channels
		m_rectTLChannel = m_rectTLArrow;
		m_rectBRChannel = m_rectBRArrow;
		m_rectTLChannel.left = m_rectTLArrow.right;
		if( bHasThumb )
		{
			m_rectTLChannel.right = m_rectThumb.left;
			m_rectBRChannel.left = m_rectThumb.right;
			m_rectBRChannel.right = m_rectBRArrow.left;
		}
		else
		{
			m_rectTLChannel.right = m_rectBRArrow.left;	// L channel reaches to R arrow.
			m_rectBRChannel.SetRectEmpty();				// No thumb - so R channel not needed.
		}
	}
	else // Vertical scroll bar.
	{	// Calc. arrows
		int cyClient = m_rectClient.Height();
		int cyArrow = GetCXYarrow();	// Arrow button height.

		if( cyClient < (2 * cyArrow) )
			cyArrow = cyClient / 2;	// Limit arrow height to available area.

		m_rectTLArrow.SetRect( m_rectClient.left, m_rectClient.top,
			m_rectClient.right, m_rectClient.top + cyArrow );

		m_rectBRArrow.SetRect( m_rectClient.left, m_rectClient.bottom - cyArrow,
			m_rectClient.right, m_rectClient.bottom );

		// Calc. thumb size and position
		int yThumb, cyThumb, cyChannel = cyClient - (2 * cyArrow);
		bHasThumb = CalcThumb( cyChannel, yThumb, cyThumb );
		if( bHasThumb )
		{	// We have space for thumb.
			yThumb +=m_rectTLArrow.bottom ;
			m_rectThumb = m_rectTLArrow;
			m_rectThumb.top = yThumb;
			m_rectThumb.bottom = yThumb + cyThumb;
		}

		// Calc. channels
		m_rectTLChannel = m_rectTLArrow;
		m_rectBRChannel = m_rectBRArrow;
		m_rectTLChannel.top = m_rectTLArrow.bottom;
		if( bHasThumb )
		{
			m_rectTLChannel.bottom = m_rectThumb.top;
			m_rectBRChannel.top = m_rectThumb.bottom;
			m_rectBRChannel.bottom = m_rectBRArrow.top;
		}
		else
		{
			m_rectTLChannel.bottom = m_rectBRArrow.top;	// T channel reaches to B arrow.
			m_rectBRChannel.SetRectEmpty();				// No thumb - so T channel not needed.
		}
	}
}

BOOL CXeScrollBarBase::RecalcRectsThumbTrack( CPoint point )
{
	ASSERT(m_bDragging && !m_rectThumb.IsRectEmpty());	// Sanity check.
	if( m_bHorizontal )
	{	// Horizontal scroll bar.
		// X pos of left edge of thumb (0...?)
		int xPos = point.x - m_xyThumbDragOffset;
		if( xPos < m_rectTLArrow.right )
			xPos = m_rectTLArrow.right;
		int nThumbWidth = m_rectThumb.Width();
		if( xPos > (m_rectBRArrow.left - nThumbWidth) )
			xPos = (m_rectBRArrow.left - nThumbWidth);
		if( xPos == m_rectThumb.left )
			return FALSE;						// No change.
		m_rectThumb.left = xPos;
		m_rectThumb.right = m_rectThumb.left + nThumbWidth;
		m_rectTLChannel.right = m_rectThumb.left;
		m_rectBRChannel.left = m_rectThumb.right;
	}
	else
	{	// Vertical scroll bar.
		// Y pos of top edge of thumb (0...?)
		int yPos = point.y - m_xyThumbDragOffset;
		if( yPos < m_rectTLArrow.bottom )
			yPos = m_rectTLArrow.bottom;
		int nThumbHeight = m_rectThumb.Height();
		if( yPos > (m_rectBRArrow.top - nThumbHeight) )
			yPos = (m_rectBRArrow.top - nThumbHeight);
		if( yPos == m_rectThumb.top )
			return FALSE;						// No change.
		m_rectThumb.top = yPos;
		m_rectThumb.bottom = m_rectThumb.top + nThumbHeight;
		m_rectTLChannel.bottom = m_rectThumb.top;
		m_rectBRChannel.top = m_rectThumb.bottom;
	}
	return TRUE;
}

UINT CXeScrollBarBase::GetCXYarrow()
{
	if( m_uArrowWH )		// Has derived class set this value?
		return m_uArrowWH;	// Use arrow button width/height set by derived class.

	// If m_uArrowWH == 0 we must assume the arrow button is same width or height as
	// the scrollbar window.
	if( m_bHorizontal )
		return m_rectClient.Height();	// Horz. arrow button is same height as window.
	return m_rectClient.Width();		// Vert. arrow button is same width as window.
}