#include "stdafx.h"
#include "CMPCThemePlayerBar.h"
#include "mplayerc.h"
#include "CMPCTheme.h"

CMPCThemePlayerBar::CMPCThemePlayerBar()
{
}

CMPCThemePlayerBar::~CMPCThemePlayerBar()
{
}

IMPLEMENT_DYNAMIC(CMPCThemePlayerBar, CPlayerBar)

BEGIN_MESSAGE_MAP(CMPCThemePlayerBar, CPlayerBar)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


BOOL CMPCThemePlayerBar::OnEraseBkgnd(CDC* pDC)
{
    const CAppSettings& s = AfxGetAppSettings();
    if (s.bMPCThemeLoaded) {
        CRect rect;
        pDC->GetClipBox(&rect);
        pDC->FillSolidRect(rect.left, rect.top, rect.Width(), rect.Height(), CMPCTheme::WindowBGColor);

        return TRUE;
    } else {
        return __super::OnEraseBkgnd(pDC);
    }
}

void paintHideButton(CDC* pDC, CSCBButton b) //derived from CSCBButton::Paint
{
    CRect rc = b.GetRect();

    if (b.bPushed) {
        pDC->FillSolidRect(rc, CMPCTheme::ClosePushColor);
    } else if (b.bRaised) {
        pDC->FillSolidRect(rc, CMPCTheme::CloseHoverColor);
    }

    COLORREF clrOldTextColor = pDC->GetTextColor();
    pDC->SetTextColor(CMPCTheme::TextFGColor);
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);
    CFont font;
    int ppi = pDC->GetDeviceCaps(LOGPIXELSX);
    int pointsize = MulDiv(60, 96, ppi); // 6 points at 96 ppi
    font.CreatePointFont(pointsize, _T("Marlett"));
    CFont* oldfont = pDC->SelectObject(&font);

    //mpc-hc custom code start
    // TextOut is affected by the layout so we need to account for that
    DWORD dwLayout = pDC->GetLayout();
    CRect pt = b.GetRect();
    pDC->TextOut(pt.left + (dwLayout == LAYOUT_LTR ? 2 : -1), pt.top + 2, CString(_T("r"))); // x-like
    //mpc-hc custom code end

    pDC->SelectObject(oldfont);
    pDC->SetBkMode(nPrevBkMode);
    pDC->SetTextColor(clrOldTextColor);
}


void CMPCThemePlayerBar::NcPaintGripper(CDC* pDC, CRect rcClient)   //derived from CSizingControlBarG base implementation
{
    const CAppSettings& s = AfxGetAppSettings();
    if (!s.bMPCThemeLoaded) {
        __super::NcPaintGripper(pDC, rcClient);
        return;
    }

    if (!HasGripper()) {
        return;
    }

    CRect gripper = rcClient;
    CRect rcbtn = m_biHide.GetRect();
    BOOL bHorz = IsHorzDocked();
    CBitmap patternBMP;

    gripper.DeflateRect(1, 1);

    if (bHorz) {   // gripper at left
        gripper.left -= m_cyGripper;
        gripper.right = gripper.left + CMPCTheme::gripPatternLong;
        gripper.top = rcbtn.bottom + 3;
        patternBMP.CreateBitmap(CMPCTheme::gripPatternLong, CMPCTheme::gripPatternShort, 1, 1, CMPCTheme::GripperBitsV);
    } else {   // gripper at top
        gripper.top -= m_cyGripper;
        gripper.bottom = gripper.top + CMPCTheme::gripPatternLong;
        gripper.right = rcbtn.left - 3;
        patternBMP.CreateBitmap(CMPCTheme::gripPatternShort, CMPCTheme::gripPatternLong, 1, 1, CMPCTheme::GripperBitsH);
    }

    CBrush brush;
    brush.CreatePatternBrush(&patternBMP);

    CDC dcMemory;
    CBitmap gb;
    CRect memRect(0, 0, gripper.Width(), gripper.Height());
    gb.CreateCompatibleBitmap(pDC, memRect.right, memRect.bottom);
    dcMemory.CreateCompatibleDC(pDC);
    dcMemory.SelectObject(&gb);
    dcMemory.SetTextColor(CMPCTheme::WindowBGColor);
    dcMemory.SetBkColor(CMPCTheme::GripperPatternColor);
    dcMemory.FillRect(memRect, &brush);

    pDC->BitBlt(gripper.left, gripper.top, gripper.Width(), gripper.Height(), &dcMemory, 0, 0, SRCCOPY);

    paintHideButton(pDC, m_biHide);
}

void CMPCThemePlayerBar::mpc_fillNcBG(CDC* mdc, CRect rcDraw)
{
    const CAppSettings& s = AfxGetAppSettings();
    if (s.bMPCThemeLoaded) {
        if (IsFloating()) {
            rcDraw.DeflateRect(1, 1);
        }
        mdc->FillSolidRect(rcDraw, CMPCTheme::WindowBGColor);
    } else {
        __super::mpc_fillNcBG(mdc, rcDraw);
    }
}
