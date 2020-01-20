#include "stdafx.h"
#include "CMPCThemeHeaderCtrl.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"
#include <lcms2\library\include\lcms2.h>

CMPCThemeHeaderCtrl::CMPCThemeHeaderCtrl()
{
    hotItem = -2;
}


CMPCThemeHeaderCtrl::~CMPCThemeHeaderCtrl()
{
}
BEGIN_MESSAGE_MAP(CMPCThemeHeaderCtrl, CHeaderCtrl)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CMPCThemeHeaderCtrl::OnNMCustomdraw)
    ON_NOTIFY(HDN_TRACKA, 0, &CMPCThemeHeaderCtrl::OnHdnTrack)
    ON_NOTIFY(HDN_TRACKW, 0, &CMPCThemeHeaderCtrl::OnHdnTrack)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CMPCThemeHeaderCtrl::drawSortArrow(CDC* dc, COLORREF arrowClr, CRect arrowRect, bool ascending)
{
    DpiHelper dpiWindow;
    dpiWindow.Override(GetSafeHwnd());

    Gdiplus::Color clr;
    clr.SetFromCOLORREF(arrowClr);

    int dpi = dpiWindow.DPIX();
    float steps;

    if (dpi < 120) {
        steps = 3.5;
    } else if (dpi < 144) {
        steps = 4;
    } else if (dpi < 168) {
        steps = 5;
    } else if (dpi < 192) {
        steps = 5;
    } else {
        steps = 6;
    }

    int xPos = arrowRect.left + (arrowRect.Width() - (steps * 2 + 1)) / 2;
    int yPos = arrowRect.top;

    Gdiplus::Graphics gfx(dc->m_hDC);
    gfx.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias8x4);
    Gdiplus::Pen pen(clr, 1);
    for (int i = 0; i < 2; i++) {
        Gdiplus::GraphicsPath path;
        Gdiplus::PointF vertices[3];

        if (ascending) {
            vertices[0] = Gdiplus::PointF(xPos, yPos);
            vertices[1] = Gdiplus::PointF(steps + xPos, yPos + steps);
            vertices[2] = Gdiplus::PointF(steps * 2 + xPos, yPos);
        } else {
            vertices[0] = Gdiplus::PointF(xPos, yPos + steps);
            vertices[1] = Gdiplus::PointF(steps + xPos, yPos);
            vertices[2] = Gdiplus::PointF(steps * 2 + xPos, yPos + steps);
        }

        path.AddLines(vertices, 3);
        gfx.DrawPath(&pen, &path);
    }
}

void CMPCThemeHeaderCtrl::drawItem(int nItem, CRect rText, CDC* pDC)
{

    COLORREF textColor = CMPCTheme::TextFGColor;
    COLORREF bgColor = CMPCTheme::ContentBGColor;

    COLORREF oldTextColor = pDC->GetTextColor();
    COLORREF oldBkColor = pDC->GetBkColor();

    CRect rGrid;
    rGrid = rText;


    rGrid.top -= 1;
    rGrid.bottom -= 1;

    CPoint ptCursor;
    ::GetCursorPos(&ptCursor);
    ScreenToClient(&ptCursor);
    checkHot(ptCursor);

    if (nItem == hotItem) {
        bgColor = CMPCTheme::ColumnHeaderHotColor;
    }
    pDC->FillSolidRect(rGrid, bgColor);

    CPen gridPen, *oldPen;
    gridPen.CreatePen(PS_SOLID, 1, CMPCTheme::HeaderCtrlGridColor);
    oldPen = pDC->SelectObject(&gridPen);
    if (nItem != 0) {
        //we will draw left border, which lines up with grid.  this differs from native widget
        //which draws the right border which consequently does not line up with the grid (ugly)
        //we only draw the left border starting from the second column
        pDC->MoveTo(rGrid.left, rGrid.top);
        pDC->LineTo(rGrid.left, rGrid.bottom);
    } else {
        pDC->MoveTo(rGrid.left, rGrid.bottom);
    }
    pDC->LineTo(rGrid.BottomRight());
    //pDC->LineTo(rGrid.right, rGrid.top);
    pDC->SelectObject(oldPen);

    if (nItem != -1) {
        HDITEM hditem = { 0 };
        hditem.mask = HDI_FORMAT | HDI_TEXT | HDI_STATE;
        const int c_cchBuffer = 1024;
        TCHAR  lpBuffer[c_cchBuffer];
        hditem.pszText = lpBuffer;
        hditem.cchTextMax = c_cchBuffer;

        GetItem(nItem, &hditem);
        int align = hditem.fmt & HDF_JUSTIFYMASK;
        UINT textFormat = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
        if (align == HDF_CENTER) {
            textFormat |= DT_CENTER;
        } else if (align == HDF_LEFT) {
            textFormat |= DT_LEFT;
            rText.left += 6;
        } else {
            textFormat |= DT_RIGHT;
            rText.right -= 6;
        }
        CString text = hditem.pszText;
        pDC->SetTextColor(textColor);
        pDC->SetBkColor(bgColor);

        CMPCThemeUtil::DrawBufferedText(pDC, text, rText, textFormat);
        if (hditem.fmt & HDF_SORTUP) {
            drawSortArrow(pDC, CMPCTheme::HeaderCtrlSortArrowColor, rText, true);
        } else if (hditem.fmt & HDF_SORTDOWN) {
            drawSortArrow(pDC, CMPCTheme::HeaderCtrlSortArrowColor, rText, false);
        }
    }

    pDC->SetTextColor(oldTextColor);
    pDC->SetBkColor(oldBkColor);
}

/* custom draw doesn't handle empty areas! code is no longer used in favor of OnPaint() */
void CMPCThemeHeaderCtrl::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

    *pResult = CDRF_DODEFAULT;
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
            *pResult = CDRF_NOTIFYITEMDRAW;
        } else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
            int nItem = pLVCD->nmcd.dwItemSpec;
            CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
            CRect rText;
            GetItemRect(nItem, rText);

            drawItem(nItem, rText, pDC);
            *pResult = CDRF_SKIPDEFAULT;
        }
    }
}


void CMPCThemeHeaderCtrl::OnHdnTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
    //    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    *pResult = 0;
}

void CMPCThemeHeaderCtrl::checkHot(CPoint point)
{
    HDHITTESTINFO hdHitTestInfo;
    hdHitTestInfo.pt = point;

    int prevHotItem = hotItem;
    hotItem = (int)SendMessage(HDM_HITTEST, 0, (LPARAM)&hdHitTestInfo);

    if ((hdHitTestInfo.flags & HHT_ONHEADER) == 0) {
        hotItem = -2;
    }
    if (hotItem != prevHotItem) {
        RedrawWindow();
    }
}


void CMPCThemeHeaderCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    if ((nFlags & MK_LBUTTON) == 0) {
        checkHot(point);
    }

    __super::OnMouseMove(nFlags, point);
}

void CMPCThemeHeaderCtrl::OnMouseLeave()
{
    if (hotItem >= 0) {
        hotItem = -1;
        RedrawWindow();
    }
    __super::OnMouseLeave();
}


#define max(a,b)            (((a) > (b)) ? (a) : (b))
void CMPCThemeHeaderCtrl::OnPaint()
{
    if (GetStyle() & HDS_FILTERBAR) {
        Default();
        return;
    }

    CPaintDC dc(this); // device context for painting
    CMemDC memDC(dc, this);
    CDC* pDC = &memDC.GetDC();
    CFont* font = GetFont();
    CFont* pOldFont = pDC->SelectObject(font);

    CRect rectClip;
    dc.GetClipBox(rectClip);

    CRect rect;
    GetClientRect(rect);

    CRect rectItem;
    int nCount = GetItemCount();

    int xMax = 0;

    for (int i = 0; i < nCount; i++) {
        CPoint ptCursor;
        ::GetCursorPos(&ptCursor);
        ScreenToClient(&ptCursor);

        GetItemRect(i, rectItem);

        CRgn rgnClip;
        rgnClip.CreateRectRgnIndirect(&rectItem);
        pDC->SelectClipRgn(&rgnClip);

        // Draw item:
        drawItem(i, rectItem, pDC);

        pDC->SelectClipRgn(NULL);

        xMax = max(xMax, rectItem.right);
    }

    // Draw "tail border":
    if (nCount == 0) {
        rectItem = rect;
        rectItem.right++;
    } else {
        rectItem.left = xMax;
        rectItem.right = rect.right + 1;
    }

    drawItem(-1, rectItem, pDC);
    pDC->SelectObject(pOldFont);
}
