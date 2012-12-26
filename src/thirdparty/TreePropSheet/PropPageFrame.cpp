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
#include "PropPageFrame.h"

namespace TreePropSheet
{


//-------------------------------------------------------------------
// class CPropPageFrame
//-------------------------------------------------------------------

CPropPageFrame::CPropPageFrame()
:   m_bShowCaption(FALSE),
    m_nCaptionHeight(0),
    m_hCaptionIcon(NULL),
    m_dwMsgFormat(DT_CENTER|DT_VCENTER|DT_NOPREFIX|DT_SINGLELINE)
{
}


CPropPageFrame::~CPropPageFrame()
{
}


/////////////////////////////////////////////////////////////////////
// Operations


void CPropPageFrame::ShowCaption(BOOL bEnable)
{
    m_bShowCaption = bEnable;
    SafeUpdateWindow(CalcCaptionArea());
}


BOOL CPropPageFrame::GetShowCaption() const
{
    return m_bShowCaption;
}


void CPropPageFrame::SetCaption(LPCTSTR lpszCaption, HICON hIcon /*= NULL*/)
{
    m_strCaption = lpszCaption;
    m_hCaptionIcon = hIcon;
    SafeUpdateWindow(CalcCaptionArea());
}


CString CPropPageFrame::GetCaption(HICON *pIcon /* = NULL */) const
{
    if (pIcon)
        *pIcon = m_hCaptionIcon;
    return m_strCaption;
}


void CPropPageFrame::SetCaptionHeight(int nCaptionHeight)
{
    m_nCaptionHeight = nCaptionHeight;
    SafeUpdateWindow(CalcCaptionArea());
}


int CPropPageFrame::GetCaptionHeight() const
{
    return m_nCaptionHeight;
}


void CPropPageFrame::SetMsgText(LPCTSTR lpszMsg)
{
    m_strMsg = lpszMsg;
    SafeUpdateWindow(CalcMsgArea());
}


CString CPropPageFrame::GetMsgText() const
{
    return m_strMsg;
}


void CPropPageFrame::SetMsgFormat(DWORD dwFormat)
{
    m_dwMsgFormat = dwFormat;
    SafeUpdateWindow(CalcMsgArea());
}


DWORD CPropPageFrame::GetMsgFormat() const
{
    return m_dwMsgFormat;
}


/////////////////////////////////////////////////////////////////////
// Overridable implementation helpers

void CPropPageFrame::Draw(CDC *pDc)
{
    if (GetShowCaption())
        DrawCaption(pDc, CalcCaptionArea(), m_strCaption, m_hCaptionIcon);
    DrawMsg(pDc, CalcMsgArea(), m_strMsg, m_dwMsgFormat);
}


CRect CPropPageFrame::CalcMsgArea()
{
    ASSERT(IsWindow(GetWnd()->GetSafeHwnd()));

    CRect   rectMsg;
    GetWnd()->GetClientRect(rectMsg);
    if (GetShowCaption())
        rectMsg.top+= GetCaptionHeight();

    return rectMsg;
}


void CPropPageFrame::DrawMsg(CDC *pDc, CRect rect, LPCTSTR /*lpszMsg*/, DWORD /*dwFormat*/)
{
    CFont   *pPrevFont = dynamic_cast<CFont*>(pDc->SelectStockObject(DEFAULT_GUI_FONT));
    int     nPrevBkMode = pDc->SetBkMode(TRANSPARENT);

    pDc->DrawText(GetMsgText(), rect, GetMsgFormat());

    pDc->SetBkMode(nPrevBkMode);
    pDc->SelectObject(pPrevFont);
}


CRect   CPropPageFrame::CalcCaptionArea()
{
    ASSERT(IsWindow(GetWnd()->GetSafeHwnd()));

    CRect   rectCaption;
    GetWnd()->GetClientRect(rectCaption);
    if (!GetShowCaption())
        rectCaption.bottom = rectCaption.top;
    else
        rectCaption.bottom = rectCaption.top+GetCaptionHeight();

    return rectCaption;
}


void CPropPageFrame::DrawCaption(CDC* /*pDc*/, CRect /*rect*/, LPCTSTR /*lpszCaption*/, HICON /*hIcon*/)
{
    // should be implemented by specialized classes
}


/////////////////////////////////////////////////////////////////////
// Implementation helpers

void CPropPageFrame::SafeUpdateWindow(LPCRECT lpRect /* = NULL */)
{
    if (!IsWindow(GetWnd()->GetSafeHwnd()))
        return;

    GetWnd()->InvalidateRect(lpRect, TRUE);
}



} //namespace TreePropSheet
