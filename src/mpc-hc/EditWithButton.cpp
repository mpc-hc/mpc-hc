#include "stdafx.h"
#include "EditWithButton.h"

#define WM_EDITWITHBUTTON_RECALCNCSIZE      (WM_USER + 200)     // arbitrary number. hopefully no conflicts


// CEditWithButton_Base ---------------------------------------------------------------------------

CEditWithButton_Base::CEditWithButton_Base()
    : m_TopBorder(0)
    , m_BottomBorder(0)
    , m_LeftBorder(0)
    , m_RightBorder(0)
    , m_ButtonWidth(1)
    , m_IsButtonPressed(false)
    , m_IsMouseActive(false)
    , m_IsButtonHot(false)
{
}

BEGIN_MESSAGE_MAP(CEditWithButton_Base, CEdit)
    ON_WM_NCCALCSIZE()
    ON_WM_NCPAINT()
    ON_WM_NCLBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_NCMOUSEMOVE()
    ON_WM_NCMOUSELEAVE()
    ON_WM_LBUTTONUP()
    ON_WM_NCHITTEST()
    ON_MESSAGE(WM_EDITWITHBUTTON_RECALCNCSIZE, OnRecalcNcSize)
    ON_WM_NCDESTROY()
    ON_WM_ENABLE()
    ON_MESSAGE(EM_SETREADONLY, OnSetReadOnly)
END_MESSAGE_MAP()

CRect CEditWithButton_Base::GetButtonRect(const CRect& rectWindow) const
{
    CRect rectButton(rectWindow);
    rectButton.top += m_TopBorder;
    rectButton.bottom -= m_BottomBorder;
    rectButton.right -= m_RightBorder;
    rectButton.left = rectButton.right - m_ButtonWidth;

    // take into account any scrollbars in the edit control
    if (rectButton.right > rectButton.left) {
        rectButton.OffsetRect(m_RightBorder - m_LeftBorder, 0);
    }

    return rectButton;
}

int CEditWithButton_Base::GetButtonThemeState() const
{
    if (GetStyle() & (ES_READONLY | WS_DISABLED)) {
        return PBS_DISABLED;
    } else if (m_IsButtonPressed) {
        return PBS_PRESSED;
    } else if (m_IsButtonHot) {
        return PBS_HOT;
    } else {
        return PBS_NORMAL;
    }
}

void CEditWithButton_Base::DrawButton(CRect rectButton)
{
    CWindowDC dc(this);

    HTHEME hButtonTheme = OpenThemeData(m_hWnd, _T("Button"));
    if (hButtonTheme) {
        int ButtonState = GetButtonThemeState();

        // If necessary, first fill with the edit control's background color.
        if (IsThemeBackgroundPartiallyTransparent(hButtonTheme, BP_PUSHBUTTON, ButtonState)) {
            HTHEME hEditTheme = OpenThemeDataEx(m_hWnd, _T("Edit"), OTD_NONCLIENT);

            COLORREF BgColor = GetThemeSysColor(hEditTheme, (GetStyle() & (ES_READONLY | WS_DISABLED)) ? COLOR_3DFACE : COLOR_WINDOW);
            dc.FillSolidRect(rectButton, BgColor);

            CloseThemeData(hEditTheme);
        }

        DrawThemeBackground(hButtonTheme, dc, BP_PUSHBUTTON, ButtonState, rectButton, nullptr);

        DrawButtonContent(dc, rectButton, hButtonTheme);

        CloseThemeData(hButtonTheme);
    } else {
        UINT uState = DFCS_BUTTONPUSH;
        if (GetStyle() & (ES_READONLY | WS_DISABLED)) {
            uState |= DFCS_INACTIVE;
        } else if (m_IsButtonPressed) {
            uState |= DFCS_PUSHED;
        }
        dc.DrawFrameControl(rectButton, DFC_BUTTON, uState);

        // If the button is in a pressed state, then contents should move slightly as part of the "push" effect.
        if (m_IsButtonPressed) {
            rectButton.OffsetRect(1, 1);
        }

        DrawButtonContent(dc, rectButton, nullptr);
    }
}

void CEditWithButton_Base::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
    CRect rectOld = lpncsp->rgrc[0];

    // Let the default processing setup space of the usual screen elements (borders, etc)
    CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);

    // Store the current size of the borders, so we know where to put the button
    m_TopBorder = lpncsp->rgrc[0].top - rectOld.top;
    m_BottomBorder = rectOld.bottom - lpncsp->rgrc[0].bottom;
    m_LeftBorder = lpncsp->rgrc[0].left - rectOld.left;
    m_RightBorder = rectOld.right - lpncsp->rgrc[0].right;

    m_ButtonWidth = CalculateButtonWidth();

    // Deflate the right side, making room for our button
    lpncsp->rgrc[0].right -= m_ButtonWidth;
}

void CEditWithButton_Base::OnNcPaint()
{
    // Allow default processing
    CEdit::OnNcPaint();

    CRect rectWindow;
    GetWindowRect(rectWindow);
    // Adjust coords to start at 0,0
    rectWindow.OffsetRect(-rectWindow.TopLeft());

    CRect rectButton = GetButtonRect(rectWindow);

    DrawButton(rectButton);
}

void CEditWithButton_Base::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
    if (!(GetStyle() & (ES_READONLY | WS_DISABLED))) {
        CRect rectWindow;
        GetWindowRect(rectWindow);
        CRect rectButton = GetButtonRect(rectWindow);

        if (rectButton.PtInRect(point)) {
            SetCapture();

            m_IsButtonPressed = true;
            m_IsMouseActive = true;

            // Redraw the button to reflect the change
            SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
        }
    }

    CEdit::OnNcLButtonDown(nHitTest, point);
}

void CEditWithButton_Base::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_IsMouseActive) {
        ClientToScreen(&point);

        CRect rectWindow;
        GetWindowRect(rectWindow);
        CRect rectButton = GetButtonRect(rectWindow);

        bool OldState = m_IsButtonPressed;

        m_IsButtonPressed = rectButton.PtInRect(point) != FALSE;

        // If the button state has changed, redraw it to reflect the change
        if (OldState != m_IsButtonPressed) {
            SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
        }
    }

    CEdit::OnMouseMove(nFlags, point);
}

void CEditWithButton_Base::OnNcMouseMove(UINT nHitTest, CPoint point)
{
    CRect rectWindow;
    GetWindowRect(rectWindow);
    CRect rectButton = GetButtonRect(rectWindow);

    bool OldState = m_IsButtonHot;
    m_IsButtonHot = rectButton.PtInRect(point) != FALSE;
    // If the button state has changed, redraw it to reflect the change
    if (OldState != m_IsButtonHot) {
        SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

        // If the state has changed to hot, register to get the WM_NCMOUSELEAVE notification.
        if (m_IsButtonHot) {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
            tme.hwndTrack = m_hWnd;
            tme.dwHoverTime = HOVER_DEFAULT;
            _TrackMouseEvent(&tme);
        }
    }

    CEdit::OnNcMouseMove(nHitTest, point);
}

void CEditWithButton_Base::OnNcMouseLeave()
{
    CPoint point;
    GetCursorPos(&point);

    CRect rectWindow;
    GetWindowRect(rectWindow);
    CRect rectButton = GetButtonRect(rectWindow);

    // We may get this message either when the mouse actually leaves the client area
    // or when the user clicks the mouse on the button. So we must check whether or
    // not the cursor has actually left the button area. If so, then update the hot
    // state and prompt a redraw of the button.
    if (!rectButton.PtInRect(point)) {
        m_IsButtonHot = false;
        SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
    }

    CEdit::OnNcMouseLeave();
}

void CEditWithButton_Base::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_IsMouseActive) {
        ReleaseCapture();

        ClientToScreen(&point);

        CRect rectWindow;
        GetWindowRect(rectWindow);
        CRect rectButton = GetButtonRect(rectWindow);

        // Reset the button to a "normal" state.
        m_IsButtonHot = false;
        m_IsButtonPressed = false;
        m_IsMouseActive = false;

        // Redraw the button to reflect the changes.
        SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

        // Run the on-click logic if appropriate.
        if (rectButton.PtInRect(point)) {
            OnLeftClick();
        }
    }

    CEdit::OnLButtonUp(nFlags, point);
}

LRESULT CEditWithButton_Base::OnNcHitTest(CPoint point)
{
    CRect rectWindow;
    GetWindowRect(rectWindow);
    CRect rectButton = GetButtonRect(rectWindow);

    if (rectButton.PtInRect(point)) {
        return HTBORDER;
    }

    return CEdit::OnNcHitTest(point);
}

void CEditWithButton_Base::PreSubclassWindow()
{
    CEdit::PreSubclassWindow();

    // Because our WindowProc is not yet in place, we need to post a message
    PostMessage(WM_EDITWITHBUTTON_RECALCNCSIZE);
}

LRESULT CEditWithButton_Base::OnRecalcNcSize(WPARAM wParam, LPARAM lParam)
{
    // Prompt a WM_NCCALCSIZE to be issued
    SetWindowPos(nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

    return 0;
}

void CEditWithButton_Base::OnEnable(BOOL bEnable)
{
    // Let all the default handling happen.
    CEdit::OnEnable(bEnable);

    // Prompt the button area to redraw.
    SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

LRESULT CEditWithButton_Base::OnSetReadOnly(WPARAM wParam, LPARAM lParam)
{
    // Let all the default handling happen.
    LRESULT r = DefWindowProc(EM_SETREADONLY, wParam, lParam);

    // Prompt the button area to redraw.
    SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

    return r;
}

void CEditWithButton_Base::OnLeftClick()
{
    PostMessage(EDIT_BUTTON_LEFTCLICKED);
}



// CEditWithButton --------------------------------------------------------------------------------

CEditWithButton::CEditWithButton(LPCTSTR pszButtonText)
    : m_ButtonText(pszButtonText)
{
}

CString CEditWithButton::GetButtonText() const
{
    return m_ButtonText;
}

void CEditWithButton::SetButtonText(LPCTSTR buttonText)
{
    m_ButtonText = buttonText;

    // If this is a live window, then prompt the button area to redraw.
    if (IsWindow(m_hWnd)) {
        SetWindowPos(nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
    }
}

void CEditWithButton::DrawButtonContent(CDC& dc, CRect rectButton, HTHEME hButtonTheme)
{
    CFont* pOldFont = dc.SelectObject(GetFont());

    if (hButtonTheme) {
        DrawThemeText(hButtonTheme, dc.m_hDC, BP_PUSHBUTTON, GetButtonThemeState(),
                      m_ButtonText, m_ButtonText.GetLength(),
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE, 0, rectButton);
    } else {
        if (GetStyle() & (ES_READONLY | WS_DISABLED)) {
            dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
        }

        dc.SetBkMode(TRANSPARENT);
        dc.DrawText(m_ButtonText, m_ButtonText.GetLength(),
                    rectButton, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    dc.SelectObject(pOldFont);
}

int CEditWithButton::CalculateButtonWidth()
{
    CWindowDC dc(this);
    return dc.GetTextExtent(' ' + m_ButtonText + ' ').cx;
    // Note: For readability, we need some space between the text and the side borders of the button.
    //  A simple way to accomplish this is to pad the string with spaces when calculating the width.
}
