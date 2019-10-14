#pragma once

/****************************************************************************************
CXeScrollBarBase class.
This class implements a very close copy of the standard windows scroll bar.
This class is designed as a base class for another class that will do all painting.
This class handles all the business logic.
No painting is done in this class - except in debug mode.

The derived class needs to do the following:
*	MUST override DrawScrollBar member function to do all the painting.
	Example:
	Create memory DC - same size as client area.
	XSB_EDRAWELEM eState;
	const CRect *prcElem = 0;
	stXSB_AREA stArea;
	for( int nElem = eTLbutton; nElem <= eThumb; nElem++ )	// loop through all UI elements.
	{
		stArea.eArea = (eXSB_AREA)nElem;

		// Get bounding rect of UI element to draw (client coords.)
		prcElem = GetUIelementDrawState( stArea.eArea, eState );
		if( !prcElem || eState == eNotDrawn )	// Rect empty or area not drawn?
			continue;

		// stArea.eArea identifies UI element to draw:
		//     eTLbutton or eTLchannel or eThumb or eBRchannel or eBRbutton.

		// eState identifes in what state the UI element is drawn:
		//     eDisabled or eNormal or eDown or eHot.

		// m_bHorizontal is TRUE if 'this' is a horizontal scroll bar.

		// Draw UI element to memory DC. (using prcElem rect).
		// Note - use m_bDrawGripper to determine if 'gripper' is drawn on thumb box.
		//        This is used to implement 'blinking' to show scroll bar has input
		//        focus. (every 500mS).
	}
	Copy memory DC to pDC.

*	(optional) Set m_uArrowWH to the width of the arrow button in horizontal scrollbar,
	note - height of arrow button in vertical scrollbar is the same.
	If this member is left unchanged (= 0) the arrow button width in horizontal scrollbar
	is assumed to be equal to the height of the window. Same for vertical scroll bar 
	except the width of the window is arrow button height.
	This number is needed to calculate sizes of other UI elements of the scroll bar.

*	(optional) Set m_uThumbMinHW to the minimum allowed width of the thumb box in a
	horizontal scroll bar, this value is also the minimum height of the thumb box in a 
	vertical scroll bar. The default is 8 pixels.
****************************************************************************************/

// Enum - UI element state - helps with drawing scrollbar.
typedef enum tagXSB_EDRAWELEM
{
	eNotDrawn = 0,	// UI element is not visible. (UI Rect empty).
	eDisabled,		// Element should be drawn in disabled state.
	eNormal,		// Element should be drawn in normal state.
	eDown,			// Element should be drawn in down state.
	eHot			// Element should be drawn in hot state.
					// Note - Scroll bar channel is usually not drawn 'hot'.
} XSB_EDRAWELEM;

// Enum - For Scrollbar five UI elements:
//		Top (Left) arrow button.
//		Top (Left) channel (or shaft).
//		Thumb track button.
//		Bottom (Right) channel (or shaft).
//		Bottom (Right) arrow button.
typedef enum tagXSB_EAREA
{
	eNone = 0,
	eTLbutton,	// Top (Left) arrow button.
	eBRbutton,	// Bottom (Right) arrow button.
	eTLchannel,	// Top (Left) channel (or shaft).
	eBRchannel,	// Bottom (Right) channel (or shaft).
	eThumb		// Thumb track button.
} eXSB_AREA;

// 'Helper' data structure - helps make the code readable.
typedef struct tagXSB_STAREA
{
	tagXSB_STAREA() { eArea = eNone; }
	void operator=( eXSB_AREA e ) { eArea = e; }
	void operator=( tagXSB_STAREA &st ) { eArea = st.eArea; }

	bool operator==( tagXSB_STAREA &stB ) { return ( eArea == stB.eArea ); }
	bool operator!=( tagXSB_STAREA &stB ) { return ( eArea != stB.eArea ); }

	bool IsNone() { return ( eArea == eNone ); }

	bool IsButton() { return ( eArea == eTLbutton || eArea == eBRbutton ); }

	bool IsThumb() { return ( eArea == eThumb ); }

	bool IsChannel() { return ( eArea == eTLchannel || eArea == eBRchannel ); }

	bool IsLeftButton() { return ( eArea == eTLbutton ); }

	bool IsRightButton() { return ( eArea == eBRbutton ); }

	bool IsUpButton() { return ( eArea == eTLbutton ); }

	bool IsDownButton() { return ( eArea == eBRbutton ); }

	bool IsLeftChannel() { return ( eArea == eTLchannel ); }

	bool IsRightChannel() { return ( eArea == eBRchannel ); }

	bool IsUpChannel() { return ( eArea == eTLchannel ); }

	bool IsDownChannel() { return ( eArea == eBRchannel ); }

	bool IsTLButton() { return ( eArea == eTLbutton ); }

	bool IsBRButton() { return ( eArea == eBRbutton ); }

	bool IsTLChannel() { return ( eArea == eTLchannel ); }

	bool IsBRChannel() { return ( eArea == eBRchannel ); }

	eXSB_AREA eArea;	// <- the only data member of this structure.
} stXSB_AREA;

///////////////////////////////////////////////////////////////////////////
// CXeScrollBarBase class.

class CXeScrollBarBase : public CScrollBar
{
	DECLARE_DYNAMIC(CXeScrollBarBase)

public:
	///////////////////////////////////////////////////////////////////////////
	// Construction / destruction

	CXeScrollBarBase();
	virtual ~CXeScrollBarBase();

	///////////////////////////////////////////////////////////////////////////
	// Create

	// Create 'this' type of scroll bar.
	// [IN]  dwStyle = Window style. (if SBS_VERT present a vertical scroll bar is created).
	// [IN]  rect = position and size of window in parent client coords.
	// [IN]  pParentWnd = Parent window. ASSERTs if not valid.
	// [IN]  uID = Control identifier.
	// Returns TRUE if create was successful.
	// returns FALSE if create failed.
	virtual BOOL Create( DWORD dwStyle, const RECT& rect, CWnd *pParentWnd, UINT nID );

	// Create 'this' type of scroll bar from existing (windows standard) scroll bar.
	// New scroll bar is created by using the following from the existing scroll bar:
	//		* Window style. (if SBS_VERT present a vertical scroll bar is created).
	//		* Window size and position.
	//		* Control ID.
	//		* Z-order.
	//		* Scroll bar parameters (range, page and position).
	// [IN]  pParentWnd = Parent window. ASSERTs if not valid.
	// [IN]  nID = Control identifier of exisiting scroll bar. ASSERTs if not found.
	// [IN]  bUseDefaultWH = TRUE to use system default scroll bar width/height.
	//                     = FALSE to use width/height from existing scroll bar.
	// Returns TRUE if create was successful, note - existing scroll has been destroyed.
	// returns FALSE if create failed.
	BOOL CreateFromExisting( CWnd *pParentWnd, UINT nID, BOOL bUseDefaultWH = TRUE );

	BOOL RegisterWindowClass();

	///////////////////////////////////////////////////////////////////////////
	// Overrides

	// This is the only member a derived class needs to override.
	// Note - derived class should NOT call DrawScrollBar in this class.
	virtual void DrawScrollBar( CDC* pDC );

protected:
	///////////////////////////////////////////////////////////////////////////
	// Message map functions

	///////////////////////////////////////////////////////////////////////////
	// Scroll bar message (SBM_XXX) handlers

	// SBM_ENABLE_ARROWS message handler.
	afx_msg LRESULT OnSbmEnableArrows( WPARAM wparam, LPARAM lparam );

	// SBM_GETPOS message handler.
	afx_msg LRESULT OnSbmGetPos( WPARAM wparam, LPARAM lparam );

	// SBM_GETRANGE message handler.
	afx_msg LRESULT OnSbmGetRange( WPARAM wparam, LPARAM lparam );

	// SBM_GETSCROLLBARINFO message handler.
	afx_msg LRESULT OnSbmGetScrollBarInfo( WPARAM wparam, LPARAM lparam );

	// SBM_GETSCROLLINFO message handler.
	afx_msg LRESULT OnSbmGetScrollInfo( WPARAM wparam, LPARAM lparam );

	// SBM_SETPOS message handler.
	afx_msg LRESULT OnSbmSetPos( WPARAM wparam, LPARAM lparam );

	// SBM_SETRANGE message handler.
	afx_msg LRESULT OnSbmSetRange( WPARAM wparam, LPARAM lparam );

	// SBM_SETRANGEREDRAW message handler.
	afx_msg LRESULT OnSbmSetRangeRedraw( WPARAM wparam, LPARAM lparam );

	// SBM_SETSCROLLINFO message handler.
	afx_msg LRESULT OnSbmSetScrollInfo( WPARAM wparam, LPARAM lparam );

	///////////////////////////////////////////////////////////////////////////
	// Paint messages

	// WM_PAINT message handler.
	afx_msg void OnPaint();

	// WM_ERASEBKGND message handler.
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );

	///////////////////////////////////////////////////////////////////////////
	// Keyboard messages

	// WM_KEYDOWN message handler.
	afx_msg LRESULT OnKeyDown( WPARAM wParam, LPARAM lParam );

	// WM_KEYUP message handler.
	afx_msg LRESULT OnKeyUp( WPARAM wParam, LPARAM lParam );

	///////////////////////////////////////////////////////////////////////////
	// Mouse messages

	// WM_LBUTTONDOWN message handler.
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );

	// WM_LBUTTONUP message handler.
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );

	// WM_MOUSEWHEEL message handler.
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	// WM_MOUSEMOVE message handler.
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );

	// WM_MOUSELEAVE message handler.
	afx_msg LRESULT OnMouseLeave( WPARAM wparam, LPARAM lparam );

	///////////////////////////////////////////////////////////////////////////
	// Other messages

	// WM_GETDLGCODE message handler.
	afx_msg LRESULT OnGetDlgCode( WPARAM wParam, LPARAM lParam );

	// WM_CANCELMODE message handler.
	afx_msg void OnCancelMode();

	// WM_CONTEXTMENU message handler.
	afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

	// ON_COMMAND_RANGE message map handler - for menu commands.
	afx_msg void OnMenuCommands( UINT uID );

	// WM_TIMER message handler.
	afx_msg void OnTimer(UINT_PTR nIDEvent );

	// WM_SIZE message handler.
	afx_msg void OnSize( UINT nType, int cx, int cy );

	// WM_SETFOCUS message handler.
	afx_msg void OnSetFocus( CWnd* pOldWnd );

	// WM_KILLFOCUS message handler.
	afx_msg void OnKillFocus( CWnd* pNewWnd );

	// WM_ENABLE message handler.
	afx_msg void OnEnable( BOOL bEnable );

	///////////////////////////////////////////////////////////////////////////
	// Helpers

	// Send WM_HSCROLL or WM_VSCROLL message to parent window.
	// [IN]  wSBcode = SB_XXX message (LOWORD of WPARAM).
	// [IN]  wHiWPARAM = Scroll pos when SB_THUMBTRACK or SB_THUMBPOSITION.
	virtual void SendScrollMsg( WORD wSBcode, WORD wHiWPARAM = 0 );

	// Get UI area (element) from point.
	// Returns UI area enum if point is within a UI element else eNone.
	eXSB_AREA GetAreaFromPoint( CPoint point );

	// Get 'state' of UI element.
	// [IN]  eElem = UI area (element).
	// [OUT] eState = enumerated 'state' of requested area.
	// Returns pointer to CRect of UI element or NULL if eElem invalid enum.
	const CRect *GetUIelementDrawState( eXSB_AREA eElem, XSB_EDRAWELEM &eState );

	// Calculate thumb size and position - used by RecalcRects().
	// [IN]  cxyChannel = channel width or height (excluding arrow buttons).
	// [OUT] xyThumb = Thumb x or y pos. (0 = first pixel in channel).
	// [OUT] cxyThumb = Thumb width or height.
	// Returns TRUE if channel W/H big enough for a thumb.
	// Returns FALSE if no space for thumb (xyThumb and cxyThumb unchanged).
	BOOL CalcThumb( int cxyChannel, int &xyThumb, int &cxyThumb );

	// Recalculate UI elements size and positions.
	// Note - thumb XY position is calculated from current scroll pos (m_nPos).
	// Called from Create(...), SetScrollPos(...), SetScrollRange(...), 
	//             SetScrollInfo(...), OnSize(...).
	void RecalcRects();

	// Recalculate thumb and channel rects - used while user is dragging the thumb.
	// [IN]  point = current mouse coords. in client coords.
	// Returns TRUE if thumb XY position has changed, else FALSE.
	// Note - Only thumb position changes, not the width/height.
	// Note - called from within OnMouseMove(...).
	BOOL RecalcRectsThumbTrack( CPoint point );

	///////////////////////////////////////////////////////////////////////////
	// Data members
	CWnd		*m_pParent;			// Parent window.

	BOOL		m_bEnabled;			// Window enabled state - TRUE = enabled.
									// Note - Updated on WM_ENABLE message.

	BOOL		m_bHasFocus;		// TRUE when 'this' has input focus.
									// Set TRUE in OnSetFocus.
									// Set FALSE in OnKillFocus.

	BOOL		m_bDrawGripper;		// TRUE when 'gripper' shown on thumb box.
									// FALSE when 'gripper' not drawn.
									// Note - helps implement 'blinking' focus.

	BOOL		m_bHorizontal;		// TRUE = 'this' is horizontal scroll bar

	int			m_nPos;				// Current thumb position in scroll units
	int			m_nTrackPos;		// Current thumb position (while dragging).
	UINT		m_uPage;			// Scroll 'page' size in scroll units (min = 0).
	int			m_nMinPos;			// Minimum scrolling position
	int			m_nMaxPos;			// Maximum scrolling position

	BOOL		m_bNoScroll;		// Set TRUE if 'this' scroll bar was disabled because
									// of SIF_DISABLENOSCROLL flag in SBM_SETSCROLLINFO msg.

	int			m_nMaxReportedPos;	// Max. pos scrollbar can report.
									// = m_nMaxPos - (m_uPage - 1).
									// = m_nMaxPos when m_uPage == 0.

	BOOL		m_bDragging;		// TRUE = thumb is being dragged

	BOOL		m_bTrackMouseLeave;	// TRUE when TrackMouseEvent called.
									// Set FALSE when OnMouseLeave called.

	BOOL		m_bNeedEndScroll;	// TRUE if sending SB_ENDSCROLL is needed.
									// Set TRUE in OnKeyDown if a SB_XXX message was sent.
									// Set TRUE in OnLButtonDown if a SB_XXX message was sent.
									// Set TRUE in OnMouseMove if a SB_XXX message was sent.
									// Note - SB_ENDSCROLL is sent in OnKeyDown and in
									//        OnLButtonUp if m_bNeedEndScroll is TRUE.

	stXSB_AREA	m_eMouseOverArea;	// Where mouse is 'now'.
	stXSB_AREA	m_eMouseDownArea;	// Where mouse is when L btn down.

	CPoint		m_ptMenu;			// Client coords. of context menu.
									// Needed for 'Scroll Here' command.

	//===================================================================================
	// These vars. are calculated when RecalcRects() called.
	CRect		m_rectClient;		// client rect - updated when RecalcRects() called.
	CRect		m_rectThumb;		// current rect for thumb
	CRect		m_rectTLArrow;		// top or left arrow rect
	CRect		m_rectBRArrow;		// bottom or right arrow rect
	CRect		m_rectTLChannel;	// top or left channel rect
	CRect		m_rectBRChannel;	// bottom or right channel rect

	double		m_dblPx_SU;			// Number of pixels in one scroll unit.
									// Note - not valid unless m_rectThumb is not empty.
	//===================================================================================

	UINT		m_uArrowWH;			// width or height of arrow button
									// - set by derived class.
									// If = 0 and Horz
									//		button width = client area height.
									// If = 0 and Vert
									//		button height = client area width.
									// Set to 0 by ctor.
	UINT GetCXYarrow();

	UINT		m_uThumbMinHW;		// Minimum width or height of thumb (pixels).
									// Set to 8 pixels by ctor.

	int			m_xyThumbDragOffset;// Offset (x or y) into m_rectThumb.
	// When user presses L button down in the thumb - we need to capture the
	// relative X or Y position of the mouse cursor in m_rectThumb.
	// This is used while dragging so mouse x or y rel. pos. in thumb is unchanged.

	///////////////////////////////////////////////////////////////////////////

    UINT scrollLines; //added to support multiple rows per mouse notch

	DECLARE_MESSAGE_MAP()
};


