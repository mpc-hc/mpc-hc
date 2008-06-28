/* ==========================================================================
	CLineNumberEdit
	Author :		Johan Rosengren, Abstrakt Mekanik AB
	Date :			2004-03-09
	Purpose :		CLineNumberEdit is a CEdit-derived class that displays 
					line numbers to the left of the text.
	Description :	The class uses the edit rect to make space for the line 
					numbers. The line numbers are relized through a special 
					CStatic-derived class, CLineNumberStatic. As soon as the 
					text is updated, the CLineNumberStatic is updated as 
					well.
	Usage :			The control can be dynamically created, or created from 
					a dialog template. The formatting string for the line 
					numbers can be set by calling SetLineNumberFormat (the 
					same format string as for CString::Format). By calling 
					SetMarginForegroundColor or SetMarginBackgroundColor 
					the fore- and background colors for the line number 
					display is set.
   ========================================================================
	Update :        Keith Bowes
	Date :          2004-04-13
	Purpose :       1. To allow CLineNumberEdit to properly change colour when
                       Enabled/Disabled or when system colours change.
                       Changing system colours only has a noticable effect when
                       a scheme such as Marine or Plum is chosen.
                    2. To allow a line number delta to be applied to the first
                       line number so the index does not have to start at zero.
                    3. To allow a max value to be specified to stop the line
                       count and to allow smarter size formatting.
	Description :   1. Added OnEnable and OnSysColorChange to detect when
                       a colour change is required. This allows the line number
                       area and CEdit area to update colours properly.
                       Added colour ref variables to hold enabled/disabled states
                       of the background/foreground colours.
                       In an attempt to allow previous functionality to take
                       precedence, if the colours are changed explicitly, the
                       system colours are no longer queried.
                    2. Added m_LineDelta, applied when line numbers are formatted.
                    3. Using m_maxval when > 0 to limit the max values and when
                       formatting colomn width.
						JRO: Added m_lineDelta as well.
    Usage :         1. Default behaviour is to change colours to reflect CEdit.
                       manually changing the colour will cause the colours to
                       only change to the specified colours.
                    2. SetLineNumberRange sets both min and max values.
                    3. SetLineNumberRange sets both min and max values.

    Comments :      - Perhaps line values should be stored as UINT's as negative
                      values may have unexpected results.
                    - CLineNumberEdit::m_format creates a duplicate of
                      CLineNumberStatic::m_format, is this needed?
						  JRO: Even though the the two classes are thightly coupled, 
						  this duplication of data makes it easier to decouple them. 
						  A small matter, but code reuse is Politically Correct,
						  and as such A Desirable Feature.
                    - Added options could allow different system colours to be
                      chosen and updated as system attributes are changed.
                    - if m_maxval is exceeded in the edit box, new lines
                      are added without line numbers. This might not be the
                      desired behaviour.
						JRO: I think this is rather nifty, actually. If I, as a 
						developer, sets the max number of lines to be numbered, 
						I also expect this to be the case :-)))
                    - It's not spelled wrong, just differently. ;0)
   ========================================================================
	Update :        Johan Rosengren
	Date :          2004-04-14
	Purpose		:	1. Allow deriving of CLineNumberEdit. 
	Description	:	1. Made the message handlers virtual.
	Usage :			1. Declare message handlers as virtual in derived 
					   classes. Note that CLineNumberEdit is not built to 
					   be derived from, however.
   ========================================================================
	Update :		Keith Bowes
	Date :			2004-04-22
	Purpose :		To allow processing of WM_LINESCROLL messages. 
	Description :	Added OnLineScroll to handle the message.
	Usage :			Now will call UpdateTopAndBottom if the message is
					received.
   ========================================================================
	Update :		Johan Rosengren
	Date :			2004-05-02
	Purpose :		Select complete line when a line-number is clicked
	Description :	Added registered user message, sent when the line-
					number static is clicked.
	Usage :			See urm_SELECTLINE in the code.
   ========================================================================*/

#include "stdafx.h"
#include "LineNumberEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Registered message to allow selection of complete 
// lines by clicking the line number
UINT urm_SELECTLINE = ::RegisterWindowMessage( _T("_LINE_NUMBER_EDIT_SELECTLINE_") );

/////////////////////////////////////////////////////////////////////////////
// CLineNumberEdit
CLineNumberEdit::CLineNumberEdit()
/* ============================================================
	Function :		CLineNumberEdit::CLineNumberEdit
	Description :	constructor
     
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{

	m_hWnd = NULL;
	m_line.m_hWnd = NULL;
	m_zero.cx = 0;
	m_zero.cy = 0;
	m_format = _T( "%03i" );
	m_LineDelta = 1;

	// Could default m_maxval to 99,999, but may cause problems 
	// if m_format is changed and m_maxval is not...
	m_maxval = 998;

	// Setting up so by defult the original hard-coded colour 
	// scheme is used when enabled and the system colours are 
	// used when disabled.
	m_bUseEnabledSystemColours = FALSE;
	m_bUseDisabledSystemColours = TRUE;
	m_EnabledFgCol = RGB( 0, 0, 0 );
	m_EnabledBgCol = RGB( 200, 200, 200 );
	m_DisabledFgCol = GetSysColor( COLOR_GRAYTEXT );
	m_DisabledBgCol = GetSysColor( COLOR_3DFACE );

	SetWindowColour();

}

CLineNumberEdit::~CLineNumberEdit()
/* ============================================================
	Function :		CLineNumberEdit::~CLineNumberEdit
	Description :	destructor
 
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

BEGIN_MESSAGE_MAP(CLineNumberEdit, CEdit)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_VSCROLL()
	ON_CONTROL_REFLECT(EN_VSCROLL, OnVscroll)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_WM_SIZE()
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_ENABLE()
	ON_MESSAGE(EM_LINESCROLL, OnLineScroll)
	ON_REGISTERED_MESSAGE(urm_SELECTLINE, OnSelectLine)
END_MESSAGE_MAP()

void CLineNumberEdit::PreSubclassWindow() 
/* ============================================================
	Function :		CLineNumberEdit::PreSubclassWindow
	Description :	This function is called before the control 
					is subclassed for a control on a dialog 
					template, and during creation for 
					dynamically created controls.

	Return :		void
	Parameters :	none

	Usage :			Called from MFC

   ============================================================*/
{

	// Unfortunately, we can't change to ES_MULTILINE
	// during run-time.
	ASSERT( GetStyle() & ES_MULTILINE );

	// Creating the line number control
	SetLineNumberFormat( m_format );

}

/////////////////////////////////////////////////////////////////////////////
// CLineNumberEdit message handlers

void CLineNumberEdit::OnSysColorChange() 
/* ============================================================
	Function :		CLineNumberEdit::OnSysColorChange
	Description :	Handles WM_SYSCOLORCHANGE. User has changed
					the system colours, want to refresh.
 
	Return :		void
	Parameters :	void

	Usage :			Called from Windows

   ============================================================*/
{

	CEdit::OnSysColorChange();

    // update the CStatic with the new colours
    SetWindowColour( IsWindowEnabled() );

}

LRESULT CLineNumberEdit::OnSetText( WPARAM wParam, LPARAM lParam )
/* ============================================================
	Function :		CLineNumberEdit::OnSetText
	Description :	Handles WM_SETTEXT. We must update the line 
					numbers in the line number control as well.
 
	Return :		LRESULT			- From Def proc
	Parameters :	WPARAM wParam	- From Windows
					LPARAM lParam	- From Windows

	Usage :			Called from Windows

   ============================================================*/
{

	// Default processing
	LRESULT retval = DefWindowProc( WM_SETTEXT, wParam, lParam );
	UpdateTopAndBottom();
	return retval;

}

void CLineNumberEdit::OnChange() 
/* ============================================================
	Function :		CLineNumberEdit::OnChange
	Description :	Mapped to EN_CHANGE. We must handle 
					EN_CHANGE to let the line-number control 
					reflect changes to the edit box content.
 
	Return :		void
	Parameters :	none

	Usage :			Called from Windows

   ============================================================*/
{

	UpdateTopAndBottom();

}

void CLineNumberEdit::OnVscroll() 
/* ============================================================
	Function :		CLineNumberEdit::OnVscroll
	Description :	Mapped to EN_VSCROLL. We update the line 
					numbers in the line number control
 
	Return :		void
	Parameters :	none

	Usage :			Called from Windows

   ============================================================*/
{

	UpdateTopAndBottom();

}

void CLineNumberEdit::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) 
/* ============================================================
	Function :		CLineNumberEdit::OnVScroll
	Description :	Handles WM_VSCROLL. We handle WM_VSCROLL 
					in addition to the notification EN_VSCROLL, 
					to handle scrollbar dragging as well
 
	Return :		void
	Parameters :	UINT nSBCode			- From Windows
					UINT nPos				- From Windows
					CScrollBar* pScrollBar	- From Windows

	Usage :			Called from Windows

   ============================================================*/
{

	CEdit::OnVScroll( nSBCode, nPos, pScrollBar );
	UpdateTopAndBottom();

}

LRESULT CLineNumberEdit::OnLineScroll( WPARAM wParam, LPARAM lParam ) 
/* ============================================================
	Function	:	CLineNumberEdit::OnLineScroll
	Description	:	Mapped to EM_LINESCROLL. We update the line 
					numbers in the line number control.
     
	Return		:	void
	Parameters	:	none
	Usage		:	Called from Windows
   ============================================================*/
{

	// Default processing
	LRESULT retval = DefWindowProc( EM_LINESCROLL, wParam, lParam );
	UpdateTopAndBottom();
	return retval;

}

LRESULT CLineNumberEdit::OnSetFont( WPARAM wParam, LPARAM lParam )
/* ============================================================
	Function :		CLineNumberEdit::OnSetFont
	Description :	Mapped to WM_SETFONT. We must recalculate 
					the line number control size as well.
 
	Return :		LRESULT			- Always 0
	Parameters :	WPARAM wParam	- From Windows
					LPARAM lParam	- From Windows

	Usage :			Called from Windows

   ============================================================*/
{

	DefWindowProc( WM_SETFONT, wParam, lParam );
	// We resize the line-number
	// field
	Prepare();
	return 0;

}

void CLineNumberEdit::OnSize( UINT nType, int cx, int cy ) 
/* ============================================================
	Function :		CLineNumberEdit::OnSize
	Description :	Handles WM_SIZE. Recalculates the line 
					number control size as well.
 
	Return :		void
	Parameters :	UINT nType	- From Windows
					int cx		- From Windows
					int cy		- From Windows

	Usage :			Called from Windows

   ============================================================*/
{

	CEdit::OnSize( nType, cx, cy );

	// If we have the line-number
	// control, it must be resized 
	// as well.
	if( m_line.m_hWnd )
		Prepare();
 
}

void CLineNumberEdit::OnEnable( BOOL bEnable ) 
/* ============================================================
	Function :		CLineNumberEdit::OnEnable
	Description :	Handles WM_ENABLE. Calls to set colours.
 
	Return :		void
	Parameters :	BOOL bEnable - From Windows

	Usage :			Called from Windows.

   ============================================================*/
{

	CEdit::OnEnable( bEnable );
    SetWindowColour( bEnable );

}

LRESULT CLineNumberEdit::OnSelectLine(WPARAM wParam, LPARAM /*lParam*/ )
/* ============================================================
	Function :		CLineNumberEdit::OnSelectLine
	Description :	Handler for the urm_SELECTLINE registered
					message. Will select the line in wParam.
					
	Return :		LRESULT			-	Not used
	Parameters :	WPARAM wParam	-	The line to select
					LPARAM lParam	-	Not used
					
	Usage :			Called from MFC. Use 
					SendMessage( urm_SELECTLINE, line ) from 
					code.

   ============================================================*/
{

	// Calc start and end position of the line
	int lineno = wParam + GetScrollPos( SB_VERT );
	int start = LineIndex( lineno );
	int end = LineIndex( lineno + 1 );
	SetSel( start, end - 1 );
	return 0;

}

void CLineNumberEdit::SetWindowColour( BOOL bEnable /*= TRUE*/ )
/* ============================================================
	Function :		CLineNumberEdit::SetWindowColour
	Description :	Handles changing window colours.
 
	Return :		void
	Parameters :	BOOL bEnable -	flag if set enabled/disabled 
									colours

	Usage :			Called to change colours in the edit box.

   ============================================================*/
{

    if (m_bUseEnabledSystemColours)
    {
		// re-query the system colours in case they have changed.
		m_EnabledFgCol = GetSysColor( COLOR_WINDOWTEXT );
		m_EnabledBgCol = GetSysColor( COLOR_WINDOW );
    }

    if (m_bUseDisabledSystemColours)
    {
		// re-query the system colours in case they have changed.
		m_DisabledFgCol = GetSysColor( COLOR_GRAYTEXT );
		m_DisabledBgCol = GetSysColor( COLOR_3DFACE );
    }

    // change the colour based on bEnable
    if (bEnable)
    {
        m_line.SetFgColor( m_EnabledFgCol, TRUE );
        m_line.SetBgColor( m_EnabledBgCol, TRUE );
    } else {
        m_line.SetFgColor( m_DisabledFgCol, TRUE );
        m_line.SetBgColor( m_DisabledBgCol, TRUE );
    }

}

void CLineNumberEdit::UseSystemColours( BOOL bUseEnabled /*= TRUE*/, BOOL bUseDisabled /*= TRUE*/ )
/* ============================================================
	Function :		CLineNumberEdit::UseSystemColours
	Description :	Sets the Use*SystemColours flags.
 
	Return :		void
	Parameters :	BOOL bEnabled	-	flag if to use enabled 
										system colours
					BOOL bDisabled	-	flag if to use disabled 
										system colours

	Usage :			Called to change colours in the edit box

   ============================================================*/
{

    m_bUseEnabledSystemColours = bUseEnabled;
    m_bUseDisabledSystemColours = bUseDisabled;
    BOOL bEnable = TRUE;
    if (::IsWindow(m_hWnd))
        bEnable = IsWindowEnabled();

    SetWindowColour( bEnable );

}

/////////////////////////////////////////////////////////////////////////////
// CLineNumberEdit private implementation

void CLineNumberEdit::Prepare()
/* ============================================================
	Function :		CLineNumberEdit::Prepare
	Description :	Setting the edit rect for the control and 
					either create or move the line number 
					control. Also sets the top- and bottom 
					line numbers.
 
	Return :		void
	Parameters :	none

	Usage :			Must be called to (re)establish the edit 
					rect, must also be called as soon as the 
					control changes size.

   ============================================================*/
{

	// Calc sizes
	int width = CalcLineNumberWidth();
	CRect rect;
	GetClientRect( &rect );
	CRect rectEdit( rect );
	rect.right = width;
	rectEdit.left = rect.right + 3;

	// Setting the edit rect and 
	// creating or moving child control
	SetRect( &rectEdit );
	if( m_line.m_hWnd )
		m_line.MoveWindow( 0, 0, width, rect.Height() );
	else
		m_line.Create(NULL,WS_CHILD | WS_VISIBLE | SS_NOTIFY, rect, this, 1 );

	GetRect( &rectEdit );

	// Update line number control data
	m_line.SetTopMargin( rectEdit.top );
	UpdateTopAndBottom();

}

int CLineNumberEdit::CalcLineNumberWidth()
/* ============================================================
	Function :		CLineNumberEdit::CalcLineNumberWidth
	Description :	Calculates the desired width of the line 
					number control, using the current format 
					string and the max number of chars allowed 
					(pessimistic - assumes one character per 
					line).
 
	Return :		int - The width in pixels
	Parameters :	none

	Usage :			Called as soon as the format string is 
					changed.

   ============================================================*/
{

	CClientDC dc( this );

	// If a new font is set during runtime,
	// we must explicitly select the font into
	// the CClientDC to measure it.
	CFont* font = GetFont();
	CFont* oldFont = dc.SelectObject( font );

	m_zero=dc.GetTextExtent( _T( "0" ) );
	CString format;

    // GetLimitText returns the number of bytes the edit box may contain,
    // not the max number of lines...
	//... which is the max number of lines, given one character per d:o :-)
	int maxval = GetLimitText();
    if (m_maxval > 0)
        maxval = m_maxval + m_LineDelta;

	format.Format( m_format, maxval );
	CSize fmt = dc.GetTextExtent( format );
	dc.SelectObject( oldFont );

	// Calculate the size of the line-
	// number field. We add a 5 pixel margin
	// to the max size of the format string
	return fmt.cx + 5;

}

void CLineNumberEdit::UpdateTopAndBottom() 
/* ============================================================
	Function :		CLineNumberEdit::UpdateTopAndBottom
	Description :	Updates the top- and bottom line number 
					for the line number control.
 
	Return :		void
	Parameters :	none
	Usage :			Should be called as soon as the contents of 
					the control is changed.

   ============================================================*/
{

	CRect rect;
	GetClientRect( &rect );
	int maxline = GetLineCount() + m_LineDelta;

	// Height for individual lines
	int lineheight = m_zero.cy;

	// Calculate the number of lines to draw
	int topline = GetFirstVisibleLine() + m_LineDelta;
	if( ( topline + ( rect.Height() / lineheight ) ) < maxline )
		maxline = topline + ( rect.Height() / lineheight );

    if ( m_maxval > 0 && maxline > m_maxval + m_LineDelta )
        maxline = m_maxval + m_LineDelta;

	m_line.SetTopAndBottom( topline, maxline );

}

/////////////////////////////////////////////////////////////////////////////
// CLineNumberEdit public implementation

void CLineNumberEdit::SetMarginForegroundColor( COLORREF col, BOOL redraw, BOOL bEnabled /*= TRUE*/ )
/* ============================================================
	Function :		CLineNumberEdit::SetMarginForegroundColor
	Description :	Sets the text color for the number 
					margin.
 
	Return :		void
	Parameters :	COLORREF col	-	The new text color 
					BOOL redraw		-	TRUE if the control 
										should be redrawn 
										(default)

	Usage :			Call to set a new text color for the line
					number margin. The control will be redrawn
					if it exists.

   ============================================================*/
{

	m_line.SetFgColor( col, redraw );
    if (bEnabled)
    {
        m_bUseEnabledSystemColours = FALSE;
        m_EnabledFgCol = col;
    } else {
        m_bUseDisabledSystemColours = FALSE;
        m_DisabledFgCol = col;
    }

}

void CLineNumberEdit::SetMarginBackgroundColor( COLORREF col, BOOL redraw, BOOL bEnabled /*= TRUE*/ )
/* ============================================================
	Function :		CLineNumberEdit::SetMarginBackgroundColor
	Description :	Sets the background color for the number 
					margin.
 
	Return :		void
	Parameters :	COLORREF col	-	The new background color 
					BOOL redraw		-	TRUE if the control 
										should be redrawn 
										(default)

	Usage :			Call to set a new background color for the 
					line number margin. The control will be 
					redrawn if it exists.

   ============================================================*/
{

	m_line.SetBgColor( col, redraw );
    if (bEnabled)
    {
        m_bUseEnabledSystemColours = FALSE;
        m_EnabledBgCol = col;
    } else {
        m_bUseDisabledSystemColours = FALSE;
        m_DisabledBgCol = col;
    }

}

void CLineNumberEdit::SetLineNumberFormat( CString format )
/* ============================================================
	Function :		CLineNumberEdit::SetLineNumberFormat
	Description :	Changes the way line numbers are presented 
					on screen. 
 
	Return :		void
	Parameters :	CString format - The new format string

	Usage :			Call with a format string using the same 
					format as CString::Format. It should contain 
					one and only one numeric type.

   ============================================================*/
{

	m_format = format;
	m_line.SetLineNumberFormat( format );
	if( m_hWnd )
		Prepare();

}

void CLineNumberEdit::SetLineNumberRange( UINT nMin, UINT nMax /*= 0*/ )
/* ============================================================
	Function :		CLineNumberEdit::SetLineNumberRange
	Description :	Changes the default min and max line numbers. 
 
	Return :		void
	Parameters :	int nMin - changes the line offset
					int nMax - changes the max line number

	Usage :			Call to set up the min and max line numbers.

   ============================================================*/
{

    m_LineDelta = ( int ) nMin;
    m_maxval = ( int ) nMax;

}

/////////////////////////////////////////////////////////////////////////////
// CLineNumberStatic

CLineNumberStatic::CLineNumberStatic()
/* ============================================================
	Function :		CLineNumberStatic::CLineNumberStatic
	Description :	constructor
 
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{

	m_bgcol = RGB( 255, 255, 248 );
	m_fgcol = RGB( 0, 0, 0 );
    m_format = _T( "%05i" );
    m_topline = 0;
    m_bottomline = 0;
}

CLineNumberStatic::~CLineNumberStatic()
/* ============================================================
	Function :		CLineNumberStatic::~CLineNumberStatic
	Description :	destructor
 
	Return :		void
	Parameters :	none

	Usage :			

   ============================================================*/
{
}

BEGIN_MESSAGE_MAP(CLineNumberStatic, CStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLineNumberStatic message handlers

void CLineNumberStatic::OnPaint() 
/* ============================================================
	Function :		CLineNumberStatic::OnPaint
	Description :	Handler for WM_PAINT. 
 
	Return :		void
	Parameters :	none

	Usage :			Called from Windows.

   ============================================================*/
{

	CPaintDC dcPaint( this );

	CRect rect;
	GetClientRect( &rect );

	// We double buffer the drawing - 
	// preparing the memory CDC
	CDC dc;
	dc.CreateCompatibleDC( &dcPaint );
	int saved = dc.SaveDC();

	// Create GDI and select objects
	CBitmap bmp;
	CPen pen;
	bmp.CreateCompatibleBitmap( &dcPaint, rect.Width(), rect.Height() );
	pen.CreatePen( PS_SOLID, 1, m_fgcol );
	dc.SelectObject( &bmp );
	dc.SelectObject( &pen );

	// Painting the background
	dc.FillSolidRect( &rect, m_bgcol );
	dc.MoveTo( rect.right - 1, 0 );
	dc.LineTo( rect.right - 1, rect.bottom );

	// Setting other attributes
	dc.SetTextColor( m_fgcol );
	dc.SetBkColor( m_bgcol );
	dc.SelectObject( GetParent()->GetFont() );

	// Output the line numbers
	if( m_bottomline )
	{
		int lineheight = dc.GetTextExtent( _T( "0" ) ).cy;
		for( int t = m_topline ; t < m_bottomline ; t++ )
		{
			CString output;
			output.Format( m_format, t );
			int topposition = m_topmargin + lineheight * ( t - m_topline );
			dc.TextOut( 2, topposition, output );
		}
	}

	dcPaint.BitBlt( 0, 0, rect. right, rect.bottom, &dc, 0, 0, SRCCOPY );
	dc.RestoreDC( saved );

}

BOOL CLineNumberStatic::OnEraseBkgnd( CDC* ) 
/* ============================================================
	Function :		CLineNumberStatic::OnEraseBkgnd
	Description :	Mapped to WM_ERASEBKGND. Handled to avoid
					flicker, as we redraw the complete control 
					in OnPaint
 
	Return :		BOOL  -	Always TRUE
	Parameters :	CDC*  -	From Windows

	Usage :			Called from Windows.

   ============================================================*/
{

	return TRUE;

}

void CLineNumberStatic::OnLButtonDown( UINT nFlags, CPoint point )
/* ============================================================
	Function :		CLineNumberStatic::OnLButtonDown
	Description :	Called when the control is clicked. Will
					send the urm_SELECTLINE registered message 
					to the parent to select the line clicked on.
					
	Return :		void
	Parameters :	UINT nFlags		-	Not used
					CPoint point	-	Position of cursor
					
	Usage :			Called from Windows.

   ============================================================*/
{

	// Find the line clicked on
	CClientDC	dc( this );
	dc.SelectObject( GetParent()->GetFont() );
	int lineheight = dc.GetTextExtent( _T( "0" ) ).cy;
	int lineno = ( int ) ( ( double ) point.y / ( double ) lineheight );

	// Select this line in the edit control
	GetParent()->SendMessage( urm_SELECTLINE, lineno );

	CStatic::OnLButtonDown( nFlags, point );

}

/////////////////////////////////////////////////////////////////////////////
// CLineNumberStatic public implementation

void CLineNumberStatic::SetBgColor( COLORREF col, BOOL redraw )
/* ============================================================
	Function :		CLineNumberStatic::SetBgColor
	Description :	This function sets the panel background 
					color
 
	Return :		void
	Parameters :	COLORREF col -	New background color
					BOOL redraw  -	TRUE if the control 
									should be redrawn 
									(default)

	Usage :			Called from the parent.

   ============================================================*/
{

	m_bgcol = col;
	if( m_hWnd && redraw )
		RedrawWindow();

}

void CLineNumberStatic::SetFgColor( COLORREF col, BOOL redraw )
/* ============================================================
	Function :		CLineNumberStatic::SetFgColor
	Description :	This function sets the panel foreground 
					color
 
	Return :		void
	Parameters :	COLORREF col -	New text color
					BOOL redraw  -	TRUE if the control 
									should be redrawn 
									(default)
 
	Usage :			Called from the parent.

   ============================================================*/
{

	m_fgcol = col;
	if( m_hWnd && redraw )
		RedrawWindow();

}

void CLineNumberStatic::SetTopAndBottom( int topline, int bottomline )
/* ============================================================
	Function :		CLineNumberStatic::SetTopAndBottom
	Description :	Sets the top- and bottom line and redraw 
					the control (if it exists)
 
	Return :		void
	Parameters :	int topline		-	The top line number
					int bottomline	-	The bottom line number
 
	Usage :			Called when the top and bottom line is 
					changed in the parent.

   ============================================================*/
{

	m_topline = topline;
	m_bottomline = bottomline;
	if( m_hWnd )
		RedrawWindow();

}

void CLineNumberStatic::SetTopMargin( int topmargin )
/* ============================================================
	Function :		CLineNumberStatic::SetTopMargin
	Description :	Sets the top margin for painting.
 
	Return :		void
	Parameters :	int topmargin -	The top margin to set
 
	Usage :			Will be called with the value of GetRect 
					from the parent.

   ============================================================*/
{

	m_topmargin = topmargin;

}

void CLineNumberStatic::SetLineNumberFormat( CString format )
/* ============================================================
	Function :		CLineNumberStatic::SetLineNumberFormat
	Description :	Sets the format string of the control
 
	Return :		void
	Parameters :	CString format -	Format string to use 
 
	Usage :			Called from the parent when the format 
					string is changed.

   ============================================================*/
{

	m_format = format;
	if( m_hWnd )
		RedrawWindow();

}
