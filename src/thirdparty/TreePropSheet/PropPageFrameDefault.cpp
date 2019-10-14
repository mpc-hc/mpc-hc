/********************************************************************
*
* Copyright (c) 2002 Sven Wiegand <mail@sven-wiegand.de>
*
* You can use this and modify this in any way you want,
* BUT LEAVE THIS HEADER INTACT.
*
* Redistribution is appreciated.
*
* $Workfile:$
* $Revision:$
* $Modtime:$
* $Author:$
*
* Revision History:
*   $History:$
*
*********************************************************************/

#include "stdafx.h"
#include "PropPageFrameDefault.h"
// <MPC-HC Custom Code>
#include "../../DSUtil/WinAPIUtils.h"
// </MPC-HC Custom Code>


namespace TreePropSheet
{

#include <uxtheme.h>
#include <vssym32.h>

//-------------------------------------------------------------------
// class CThemeLib
//-------------------------------------------------------------------


/**
Helper class for loading the uxtheme DLL and providing their
functions.

One global object of this class exists.

@author Sven Wiegand
*/
class CThemeLib
{
// construction/destruction
public:
    CThemeLib();
    ~CThemeLib();

// operations
public:
    /**
    Returns TRUE if the call wrappers are available, FALSE otherwise.
    */
    BOOL IsAvailable() const;

// call wrappers
public:
    BOOL IsThemeActive() const
    {return (*m_pIsThemeActive)();}

    HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList) const
    {return (*m_pOpenThemeData)(hwnd, pszClassList);}

    HRESULT CloseThemeData(HTHEME hTheme) const
    {return (*m_pCloseThemeData)(hTheme);}

    HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, OUT RECT *pContentRect) const
    {return (*m_pGetThemeBackgroundContentRect)(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);}

    HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect) const
    {return (*m_pDrawThemeBackground)(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);}

    HRESULT DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, UINT uEdge, UINT uFlags, OPTIONAL const RECT * pContentRect) const
    {return (*m_pDrawThemeEdge)(hTheme, hdc, iPartId, iStateId, pRect, uEdge, uFlags, pContentRect);}

// function pointers
private:
    typedef BOOL (__stdcall *_IsThemeActive)();
    _IsThemeActive m_pIsThemeActive;

    typedef HTHEME (__stdcall *_OpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
    _OpenThemeData m_pOpenThemeData;

    typedef HRESULT(__stdcall *_CloseThemeData)(HTHEME hTheme);
    _CloseThemeData m_pCloseThemeData;

    typedef HRESULT(__stdcall *_GetThemeBackgroundContentRect)(HTHEME hTheme, OPTIONAL HDC hdc, int iPartId, int iStateId, const RECT *pBoundingRect, OUT RECT *pContentRect);
    _GetThemeBackgroundContentRect m_pGetThemeBackgroundContentRect;

    typedef HRESULT(__stdcall* _DrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, OPTIONAL const RECT* pClipRect);
    _DrawThemeBackground m_pDrawThemeBackground;

    typedef HRESULT(__stdcall* _DrawThemeEdge)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, UINT uEdge, UINT uFlags, OPTIONAL const RECT* pContentRect);
    _DrawThemeEdge m_pDrawThemeEdge;

// properties
private:
    /** instance handle to the library or NULL. */
    HINSTANCE m_hThemeLib;
};

/**
One and only instance of CThemeLib.
*/
static CThemeLib g_ThemeLib;


CThemeLib::CThemeLib()
    : m_pIsThemeActive(NULL)
    , m_pOpenThemeData(NULL)
    , m_pCloseThemeData(NULL)
    , m_pGetThemeBackgroundContentRect(NULL)
    , m_pDrawThemeBackground(NULL)
    , m_hThemeLib(NULL)
{
    m_hThemeLib = LoadLibrary(_T("uxtheme.dll"));
    if (!m_hThemeLib)
        return;

    m_pIsThemeActive = (_IsThemeActive)GetProcAddress(m_hThemeLib, "IsThemeActive");
    m_pOpenThemeData = (_OpenThemeData)GetProcAddress(m_hThemeLib, "OpenThemeData");
    m_pCloseThemeData = (_CloseThemeData)GetProcAddress(m_hThemeLib, "CloseThemeData");
    m_pGetThemeBackgroundContentRect = (_GetThemeBackgroundContentRect)GetProcAddress(m_hThemeLib, "GetThemeBackgroundContentRect");
    m_pDrawThemeBackground = (_DrawThemeBackground)GetProcAddress(m_hThemeLib, "DrawThemeBackground");
    m_pDrawThemeEdge = (_DrawThemeEdge)GetProcAddress(m_hThemeLib, "DrawThemeEdge");
}


CThemeLib::~CThemeLib()
{
    if (m_hThemeLib)
        FreeLibrary(m_hThemeLib);
}


BOOL CThemeLib::IsAvailable() const
{
    return m_hThemeLib!=NULL;
}


//-------------------------------------------------------------------
// class CPropPageFrameDefault
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CPropPageFrameDefault, CWnd)
    //{{AFX_MSG_MAP(CPropPageFrameDefault)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


CPropPageFrameDefault::CPropPageFrameDefault()
{
}


CPropPageFrameDefault::~CPropPageFrameDefault()
{
    if (m_Images.GetSafeHandle())
        m_Images.DeleteImageList();
}


/////////////////////////////////////////////////////////////////////
// Overridings

BOOL CPropPageFrameDefault::Create(DWORD dwWindowStyle, const RECT &rect, CWnd *pwndParent, UINT nID)
{
    return CWnd::Create(
        AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, AfxGetApp()->LoadStandardCursor(IDC_ARROW), GetSysColorBrush(COLOR_3DFACE)),
        _T("Page Frame"),
        dwWindowStyle, rect, pwndParent, nID);
}


CWnd* CPropPageFrameDefault::GetWnd()
{
    return static_cast<CWnd*>(this);
}


void CPropPageFrameDefault::SetCaption(LPCTSTR lpszCaption, HICON hIcon /*= NULL*/)
{
    CPropPageFrame::SetCaption(lpszCaption, hIcon);

    // build image list
    if (m_Images.GetSafeHandle())
        m_Images.DeleteImageList();
    if (hIcon)
    {
        ICONINFO    ii;
        if (!GetIconInfo(hIcon, &ii))
            return;

        CBitmap bmMask;
        bmMask.Attach(ii.hbmMask);
        if (ii.hbmColor) DeleteObject(ii.hbmColor);

        BITMAP  bm;
        bmMask.GetBitmap(&bm);

        if (!m_Images.Create(bm.bmWidth, bm.bmHeight, ILC_COLOR32|ILC_MASK, 0, 1))
            return;

        if (m_Images.Add(hIcon) == -1)
            m_Images.DeleteImageList();
    }
}


CRect CPropPageFrameDefault::CalcMsgArea()
{
    CRect   rect;
    GetClientRect(rect);
    if (g_ThemeLib.IsAvailable() && g_ThemeLib.IsThemeActive())
    {
        HTHEME  hTheme = g_ThemeLib.OpenThemeData(m_hWnd, L"Tab");
        if (hTheme)
        {
            CRect   rectContent;
            CDC     *pDc = GetDC();
            g_ThemeLib.GetThemeBackgroundContentRect(hTheme, pDc->m_hDC, TABP_PANE, 0, rect, rectContent);
            ReleaseDC(pDc);
            g_ThemeLib.CloseThemeData(hTheme);

            if (GetShowCaption())
                rectContent.top = rect.top+GetCaptionHeight()+1;
            rect = rectContent;
        }
    }
    else if (GetShowCaption())
        rect.top+= GetCaptionHeight()+1;

    return rect;
}


CRect CPropPageFrameDefault::CalcCaptionArea()
{
    CRect   rect;
    GetClientRect(rect);
    if (g_ThemeLib.IsAvailable() && g_ThemeLib.IsThemeActive())
    {
        HTHEME  hTheme = g_ThemeLib.OpenThemeData(m_hWnd, L"Tab");
        if (hTheme)
        {
            CRect   rectContent;
            CDC     *pDc = GetDC();
            g_ThemeLib.GetThemeBackgroundContentRect(hTheme, pDc->m_hDC, TABP_PANE, 0, rect, rectContent);
            ReleaseDC(pDc);
            g_ThemeLib.CloseThemeData(hTheme);

            if (GetShowCaption())
                rectContent.bottom = rect.top+GetCaptionHeight();
            else
                rectContent.bottom = rectContent.top;

            rect = rectContent;
        }
    }
    else
    {
        if (GetShowCaption())
            rect.bottom = rect.top+GetCaptionHeight();
        else
            rect.bottom = rect.top;
    }

    return rect;
}

void CPropPageFrameDefault::DrawCaption(CDC *pDc, CRect rect, LPCTSTR lpszCaption, HICON hIcon)
{
    // <MPC-HC Custom Code>
    COLORREF    clrLeft = GetSysColor(COLOR_ACTIVECAPTION);
    // </MPC-HC Custom Code>
    COLORREF    clrRight = pDc->GetPixel(rect.right-1, rect.top);
    FillGradientRectH(pDc, rect, clrLeft, clrRight);

    // draw icon
    if (hIcon && m_Images.GetSafeHandle() && m_Images.GetImageCount() == 1)
    {
        IMAGEINFO   ii;
        m_Images.GetImageInfo(0, &ii);
        CPoint      pt(3, rect.CenterPoint().y - (ii.rcImage.bottom-ii.rcImage.top)/2);
        m_Images.Draw(pDc, 0, pt, ILD_TRANSPARENT);
        rect.left+= (ii.rcImage.right-ii.rcImage.left) + 3;
    }

    // draw text
    rect.left+= 2;

    COLORREF    clrPrev = pDc->SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
    int             nBkStyle = pDc->SetBkMode(TRANSPARENT);
    CFont           *pFont = (CFont*)pDc->SelectStockObject(SYSTEM_FONT);

    // <MPC-HC Custom Code>
    LOGFONT lf;
    GetMessageFont(&lf);
    lf.lfHeight = rect.Height();
    lf.lfWeight = FW_BOLD;
    // <MPC-HC Custom Code>
    CFont f;
    f.CreateFontIndirect(&lf);
    pDc->SelectObject(&f);

    pDc->DrawText(lpszCaption, rect, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

    pDc->SetTextColor(clrPrev);
    pDc->SetBkMode(nBkStyle);
    pDc->SelectObject(pFont);
}


/////////////////////////////////////////////////////////////////////
// Implementation helpers

void CPropPageFrameDefault::FillGradientRectH(CDC *pDc, const RECT &rect, COLORREF clrLeft, COLORREF clrRight)
{
    // pre calculation
    int nSteps = rect.right-rect.left;
    int nRRange = GetRValue(clrRight)-GetRValue(clrLeft);
    int nGRange = GetGValue(clrRight)-GetGValue(clrLeft);
    int nBRange = GetBValue(clrRight)-GetBValue(clrLeft);

    double  dRStep = (double)nRRange/(double)nSteps;
    double  dGStep = (double)nGRange/(double)nSteps;
    double  dBStep = (double)nBRange/(double)nSteps;

    double  dR = (double)GetRValue(clrLeft);
    double  dG = (double)GetGValue(clrLeft);
    double  dB = (double)GetBValue(clrLeft);

    CPen    *pPrevPen = NULL;
    for (int x = rect.left; x <= rect.right; ++x)
    {
        CPen    Pen(PS_SOLID, 1, RGB((BYTE)dR, (BYTE)dG, (BYTE)dB));
        pPrevPen = pDc->SelectObject(&Pen);
        pDc->MoveTo(x, rect.top);
        pDc->LineTo(x, rect.bottom);
        pDc->SelectObject(pPrevPen);

        dR+= dRStep;
        dG+= dGStep;
        dB+= dBStep;
    }
}


/////////////////////////////////////////////////////////////////////
// message handlers

void CPropPageFrameDefault::OnPaint()
{
    CPaintDC dc(this);
    Draw(&dc);
}


BOOL CPropPageFrameDefault::OnEraseBkgnd(CDC* pDC)
{
    if (g_ThemeLib.IsAvailable() && g_ThemeLib.IsThemeActive())
    {
        HTHEME  hTheme = g_ThemeLib.OpenThemeData(m_hWnd, L"Tab");
        if (hTheme)
        {
            CRect   rect;
            GetClientRect(rect);
            //g_ThemeLib.DrawThemeBackground(hTheme, pDC->m_hDC, TABP_PANE, 0, rect, NULL);

            //mpc-hc: TABP_PANE draws a border 1 pixel short of bottom, and two pixels short of right.  
            //instead we fill using TABP_BODY and draw the edge separately (using BDR_SUNKENOUTER for 1 pixel only)
            g_ThemeLib.DrawThemeBackground(hTheme, pDC->m_hDC, TABP_BODY, 0, rect, NULL);
            g_ThemeLib.DrawThemeEdge(hTheme, pDC->m_hDC, TABP_BODY, 0, rect, BDR_SUNKENOUTER, BF_FLAT | BF_RECT, NULL);

            g_ThemeLib.CloseThemeData(hTheme);
        }
        return TRUE;
    }
    else
    {
        return CWnd::OnEraseBkgnd(pDC);
    }
}



} //namespace TreePropSheet
