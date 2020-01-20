#include "stdafx.h"
#include "CMPCThemeSliderCtrl.h"
#include "CMPCTheme.h"
#include "mplayerc.h"
#undef SubclassWindow

CMPCThemeSliderCtrl::CMPCThemeSliderCtrl()
    : m_bDrag(false), m_bHover(false)
{

}


CMPCThemeSliderCtrl::~CMPCThemeSliderCtrl()
{
}

void CMPCThemeSliderCtrl::PreSubclassWindow()
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CToolTipCtrl* pTip = GetToolTips();
        if (nullptr != pTip) {
            themedToolTip.SubclassWindow(pTip->m_hWnd);
        }
    }
}

IMPLEMENT_DYNAMIC(CMPCThemeSliderCtrl, CSliderCtrl)

BEGIN_MESSAGE_MAP(CMPCThemeSliderCtrl, CSliderCtrl)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CMPCThemeSliderCtrl::OnNMCustomdraw)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()


void CMPCThemeSliderCtrl::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    LRESULT lr = CDRF_DODEFAULT;

    const CAppSettings& s = AfxGetAppSettings();
    if (s.bMPCThemeLoaded) {
        switch (pNMCD->dwDrawStage) {
            case CDDS_PREPAINT:
                lr = CDRF_NOTIFYITEMDRAW;
                break;

            case CDDS_ITEMPREPAINT:

                if (pNMCD->dwItemSpec == TBCD_CHANNEL) {
                    CDC dc;
                    dc.Attach(pNMCD->hdc);

                    CRect rect;
                    GetClientRect(rect);
                    dc.FillSolidRect(&rect, CMPCTheme::WindowBGColor);

                    CRect channelRect;
                    GetChannelRect(channelRect);
                    CRect thumbRect;
                    GetThumbRect(thumbRect);

                    CopyRect(&pNMCD->rc, CRect(channelRect.left, thumbRect.top + 2, channelRect.right - 2, thumbRect.bottom - 2));

                    CPen shadow;
                    CPen light;
                    shadow.CreatePen(PS_SOLID, 1, CMPCTheme::ShadowColor);
                    light.CreatePen(PS_SOLID, 1, CMPCTheme::LightColor);
                    CRect r(pNMCD->rc);
                    r.DeflateRect(0, 6, 0, 6);
                    dc.FillSolidRect(r, CMPCTheme::SliderChannelColor);
                    CBrush fb;
                    fb.CreateSolidBrush(CMPCTheme::NoBorderColor);
                    dc.FrameRect(r, &fb);

                    dc.Detach();
                    lr = CDRF_SKIPDEFAULT;
                } else if (pNMCD->dwItemSpec == TBCD_THUMB) {
                    CDC dc;
                    dc.Attach(pNMCD->hdc);
                    pNMCD->rc.bottom--;
                    CRect r(pNMCD->rc);
                    r.DeflateRect(0, 0, 1, 0);

                    if (s.bMPCThemeLoaded) {
                        CBrush fb;
                        if (m_bDrag) {
                            dc.FillSolidRect(r, CMPCTheme::ScrollThumbDragColor);
                        } else if (m_bHover) {
                            dc.FillSolidRect(r, CMPCTheme::ScrollThumbHoverColor);
                        } else {
                            dc.FillSolidRect(r, CMPCTheme::ScrollThumbColor);
                        }
                        fb.CreateSolidBrush(CMPCTheme::NoBorderColor);
                        dc.FrameRect(r, &fb);
                    }

                    dc.Detach();
                    lr = CDRF_SKIPDEFAULT;
                }

                break;
        };
    }

    *pResult = lr;
}

void CMPCThemeSliderCtrl::invalidateThumb()
{
    int max = GetRangeMax();
    SetRangeMax(max, TRUE);
}


void CMPCThemeSliderCtrl::checkHover(CPoint point)
{
    CRect thumbRect;
    GetThumbRect(thumbRect);
    bool oldHover = m_bHover;
    m_bHover = false;
    if (thumbRect.PtInRect(point)) {
        m_bHover = true;
    }

    if (m_bHover != oldHover) {
        invalidateThumb();
    }
}

void CMPCThemeSliderCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    checkHover(point);
    CSliderCtrl::OnMouseMove(nFlags, point);
}


void CMPCThemeSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_bDrag = false;
    invalidateThumb();
    checkHover(point);
    CSliderCtrl::OnLButtonUp(nFlags, point);
}


void CMPCThemeSliderCtrl::OnMouseLeave()
{
    checkHover(CPoint(-1, -1));
    CSliderCtrl::OnMouseLeave();
}
