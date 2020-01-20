#include "stdafx.h"
#include "CMPCThemePlayerListCtrl.h"
#include "CMPCTheme.h"
#include "CMPCThemeUtil.h"
#include "mplayerc.h"
#undef SubclassWindow

CMPCThemePlayerListCtrl::CMPCThemePlayerListCtrl(int tStartEditingDelay) : CPlayerListCtrl(tStartEditingDelay)
{
    themeGridLines = false;
    fullRowSelect = false;
    themedSBHelper = nullptr;
    hasCheckedColors = false;
    hasCBImages = false;
    customThemeInterface = nullptr;
    if (!CMPCThemeUtil::canUseWin10DarkTheme()) {
        themedSBHelper = DEBUG_NEW CMPCThemeScrollBarHelper(this);
    }
}


CMPCThemePlayerListCtrl::~CMPCThemePlayerListCtrl()
{
    if (nullptr != themedSBHelper) {
        delete themedSBHelper;
    }
}


void CMPCThemePlayerListCtrl::PreSubclassWindow()
{
    if (!AfxGetAppSettings().bMPCThemeLoaded) {
        EnableToolTips(TRUE);
    } else {
        if (CMPCThemeUtil::canUseWin10DarkTheme()) {
            SetWindowTheme(GetSafeHwnd(), L"DarkMode_Explorer", NULL);
        } else {
            SetWindowTheme(GetSafeHwnd(), L"", NULL);
        }
        CToolTipCtrl* t = GetToolTips();
        if (nullptr != t) {
            lvsToolTip.SubclassWindow(t->m_hWnd);
        }
        subclassHeader();
    }
    CPlayerListCtrl::PreSubclassWindow();
}

IMPLEMENT_DYNAMIC(CMPCThemePlayerListCtrl, CPlayerListCtrl)

BEGIN_MESSAGE_MAP(CMPCThemePlayerListCtrl, CPlayerListCtrl)
    ON_WM_NCPAINT()
    ON_WM_CREATE()
    ON_NOTIFY_REFLECT_EX(LVN_ENDSCROLL, OnLvnEndScroll)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
    ON_WM_NCCALCSIZE()
    ON_NOTIFY_REFLECT_EX(NM_CUSTOMDRAW, OnCustomDraw)
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
    ON_NOTIFY(HDN_ENDTRACKA, 0, &OnHdnEndtrack)
    ON_NOTIFY(HDN_ENDTRACKW, 0, &OnHdnEndtrack)
    ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, &OnLvnItemchanged)
    ON_MESSAGE(PLAYER_PLAYLIST_LVN_ITEMCHANGED, OnDelayed_updateListCtrl)
END_MESSAGE_MAP()

void CMPCThemePlayerListCtrl::subclassHeader()
{
    CHeaderCtrl* t = GetHeaderCtrl();
    if (nullptr != t && IsWindow(t->m_hWnd) && themedHdrCtrl.m_hWnd == NULL) {
        themedHdrCtrl.SubclassWindow(t->GetSafeHwnd());
    }
}

void CMPCThemePlayerListCtrl::setAdditionalStyles(DWORD styles)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        DWORD stylesToAdd = styles, stylesToRemove = 0;
        if (styles & LVS_EX_GRIDLINES) {
            stylesToAdd &= ~LVS_EX_GRIDLINES;
            stylesToRemove |= LVS_EX_GRIDLINES;
            themeGridLines = true;
        }
        if (styles & LVS_EX_FULLROWSELECT) {
            //we need these to remain, or else other columns may not get refreshed on a selection change.
            //no regressions observed yet, but unclear why we removed this style for custom draw previously
            //error was observed with playersubresyncbar
            //            stylesToAdd &= ~LVS_EX_FULLROWSELECT;
            //            stylesToRemove |= LVS_EX_FULLROWSELECT;
            fullRowSelect = true;
        }
        if (styles & LVS_EX_DOUBLEBUFFER) { //we will buffer ourselves
            stylesToAdd &= ~LVS_EX_DOUBLEBUFFER;
            stylesToRemove |= LVS_EX_DOUBLEBUFFER;
        }
        SetExtendedStyle((GetExtendedStyle() | stylesToAdd) & ~stylesToRemove);
    } else {
        SetExtendedStyle(GetExtendedStyle() | styles);
    }
}

void CMPCThemePlayerListCtrl::setHasCBImages(bool on)
{
    hasCBImages = on;
}

void CMPCThemePlayerListCtrl::setItemTextWithDefaultFlag(int nItem, int nSubItem, LPCTSTR lpszText, bool flagged)
{
    SetItemText(nItem, nSubItem, lpszText);
    setFlaggedItem(nItem, flagged);
}

void CMPCThemePlayerListCtrl::setFlaggedItem(int iItem, bool flagged)
{
    flaggedItems[iItem] = flagged;
}

bool CMPCThemePlayerListCtrl::getFlaggedItem(int iItem)
{
    auto it = flaggedItems.find(iItem);
    if (it != flaggedItems.end()) {
        return it->second;
    } else {
        return false;
    }
}


BOOL CMPCThemePlayerListCtrl::PreTranslateMessage(MSG* pMsg)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (!IsWindow(themedToolTip.m_hWnd)) {
            themedToolTip.Create(this, TTS_ALWAYSTIP);
            themedToolTip.enableFlickerHelper();
        }
        if (IsWindow(themedToolTip.m_hWnd)) {
            themedToolTip.RelayEvent(pMsg);
        }
    }
    return __super::PreTranslateMessage(pMsg);
}

void CMPCThemePlayerListCtrl::setCheckedColors(COLORREF checkedBG, COLORREF checkedText, COLORREF uncheckedText)
{
    checkedBGClr = checkedBG;
    checkedTextClr = checkedText;
    uncheckedTextClr = uncheckedText;
    hasCheckedColors = true;
}

void CMPCThemePlayerListCtrl::OnNcPaint()
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (nullptr != themedSBHelper) {
            themedSBHelper->themedNcPaintWithSB();
        } else {
            CMPCThemeScrollBarHelper::themedNcPaint(this, this);
        }
    } else {
        __super::OnNcPaint();
    }
}


int CMPCThemePlayerListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (__super::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    if (AfxGetAppSettings().bMPCThemeLoaded) {
        SetBkColor(CMPCTheme::ContentBGColor);
        subclassHeader();
    }

    return 0;
}

BOOL CMPCThemePlayerListCtrl::OnLvnEndScroll(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (nullptr != themedSBHelper) {
            themedSBHelper->updateScrollInfo();
        }
        *pResult = 0;
    }
    return FALSE;
}

void CMPCThemePlayerListCtrl::updateSB()
{
    if (nullptr != themedSBHelper) {
        themedSBHelper->hideSB();
    }
}

void CMPCThemePlayerListCtrl::updateScrollInfo()
{
    if (nullptr != themedSBHelper) {
        themedSBHelper->updateScrollInfo();
    }
}

LRESULT CMPCThemePlayerListCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (AfxGetAppSettings().bMPCThemeLoaded && nullptr != themedSBHelper) {
        if (themedSBHelper->WindowProc(this, message, wParam, lParam)) {
            return 1;
        }
    }
    return __super::WindowProc(message, wParam, lParam);
}

void CMPCThemePlayerListCtrl::updateToolTip(CPoint point)
{
    if (AfxGetAppSettings().bMPCThemeLoaded && nullptr != themedToolTip) {
        TOOLINFO ti = { 0 };
        UINT_PTR tid = OnToolHitTest(point, &ti);
        //OnToolHitTest returns -1 on failure but doesn't update uId to match

        if (tid == -1 || themedToolTipCid != ti.uId) { //if no tooltip, or id has changed, remove old tool
            if (themedToolTip.GetToolCount() > 0) {
                themedToolTip.DelTool(this);
                themedToolTip.Activate(FALSE);
            }
            themedToolTipCid = (UINT_PTR) - 1;
        }

        if (tid != -1 && themedToolTipCid != ti.uId && 0 != ti.uId) {

            themedToolTipCid = ti.uId;

            CRect cr;
            GetClientRect(&cr); //we reset the tooltip every time we move anyway, so this rect is adequate

            themedToolTip.AddTool(this, LPSTR_TEXTCALLBACK, &cr, ti.uId);
            themedToolTip.Activate(TRUE);
        }
    }
}

void CMPCThemePlayerListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    __super::OnMouseMove(nFlags, point);
    updateToolTip(point);
}


BOOL CMPCThemePlayerListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    BOOL ret = __super::OnMouseWheel(nFlags, zDelta, pt);
    ScreenToClient(&pt);
    updateToolTip(pt);
    return ret;
}


void CMPCThemePlayerListCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
    __super::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CMPCThemePlayerListCtrl::drawItem(CDC* pDC, int nItem, int nSubItem)
{
    if (IsItemVisible(nItem)) {

        CRect rect, rRow, rIcon, rText, rTextBG, rectDC, rClient;
        GetClientRect(rClient);
        GetItemRect(nItem, rRow, LVIR_BOUNDS);
        GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rText);
        GetSubItemRect(nItem, nSubItem, LVIR_ICON, rIcon);
        GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);
        DWORD dwStyle = GetStyle() & LVS_TYPEMASK;

        if (0 == nSubItem) { //getsubitemrect gives whole row for 0/LVIR_BOUNDS.  but LVIR_LABEL is limited to text bounds.  MSDN undocumented behavior
            rect.right = rText.right;
        }

        //issubitemvisible
        if (rClient.left <= rect.right && rClient.right >= rect.left && rClient.top <= rect.bottom && rClient.bottom >= rect.top) {
            COLORREF textColor = CMPCTheme::TextFGColor;
            COLORREF bgColor = CMPCTheme::ContentBGColor;

            COLORREF oldTextColor = pDC->GetTextColor();
            COLORREF oldBkColor = pDC->GetBkColor();

            CString text = GetItemText(nItem, nSubItem);
            if (nullptr != customThemeInterface) { //subclasses can override colors here
                customThemeInterface->GetCustomTextColors(nItem, nSubItem, textColor, bgColor);
            }

            pDC->SetTextColor(textColor);
            pDC->SetBkColor(bgColor);

            rectDC = rRow;
            CDC dcMem;
            CBitmap bmMem;
            CMPCThemeUtil::initMemDC(pDC, dcMem, bmMem, rectDC);
            rect.OffsetRect(-rectDC.TopLeft());
            rText.OffsetRect(-rectDC.TopLeft());
            rIcon.OffsetRect(-rectDC.TopLeft());
            rRow.OffsetRect(-rectDC.TopLeft());

            if (!IsWindowEnabled() && 0 == nSubItem) { //no gridlines, bg for full row
                dcMem.FillSolidRect(rRow, CMPCTheme::ListCtrlDisabledBGColor);
            } else {
                dcMem.FillSolidRect(rect, CMPCTheme::ContentBGColor); //no flicker because we have a memory dc
            }

            rTextBG = rText;
            CHeaderCtrl* hdrCtrl = GetHeaderCtrl();
            int align = DT_LEFT;
            if (nullptr != hdrCtrl) {
                HDITEM hditem = { 0 };
                hditem.mask = HDI_FORMAT;
                hdrCtrl->GetItem(nSubItem, &hditem);
                align = hditem.fmt & HDF_JUSTIFYMASK;
            }
            UINT textFormat = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
            if (align == HDF_CENTER) {
                textFormat |= DT_CENTER;
            } else if (align == HDF_LEFT) {
                textFormat |= DT_LEFT;
                if (nSubItem == 0) {//less indent for first column
                    rText.left += 2;
                } else {
                    rText.left += 6;
                }
            } else {
                textFormat |= DT_RIGHT;
                rText.right -= 6;
            }

            bool isChecked = false;
            int contentLeft = rText.left;
            if (rIcon.Width() > 0) {
                LVITEM lvi = { 0 };
                lvi.iItem = nItem;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_IMAGE;
                GetItem(&lvi);

                if (nSubItem == 0) {
                    contentLeft = rIcon.left;
                    if (hasCBImages) { //draw manually to match theme
                        rIcon.DeflateRect(0, 0, 1, 0);
                        if (rIcon.Height() > rIcon.Width()) {
                            rIcon.DeflateRect(0, (rIcon.Height() - rIcon.Width()) / 2); //as tall as wide
                        }

                        CMPCThemeUtil::drawCheckBox(lvi.iImage, false, false, rIcon, &dcMem);
                    } else {
                        if (dwStyle == LVS_ICON) {
                        } else if (dwStyle == LVS_SMALLICON || dwStyle == LVS_LIST || dwStyle == LVS_REPORT) {
                            CImageList* ilist = GetImageList(LVSIL_SMALL);
                            int cx, cy;
                            ImageList_GetIconSize(ilist->m_hImageList, &cx, &cy);
                            rIcon.top += (rIcon.Height() - cy) / 2;
                            ilist->Draw(&dcMem, lvi.iImage, rIcon.TopLeft(), ILD_TRANSPARENT);
                        }
                    }
                    if (align == HDF_LEFT) {
                        rText.left += 2;    //more ident after image
                    }
                }
            }
            if (0 != (GetExtendedStyle() & LVS_EX_CHECKBOXES) && INDEXTOSTATEIMAGEMASK(0) != GetItemState(nItem, LVIS_STATEIMAGEMASK)) {
                isChecked = (TRUE == GetCheck(nItem));
                if (nSubItem == 0) {
                    int cbSize = GetSystemMetrics(SM_CXMENUCHECK);
                    int cbYMargin = (rect.Height() - cbSize - 1) / 2;
                    int cbXMargin = (contentLeft - rect.left - cbSize) / 2;
                    CRect rcb = { rect.left + cbXMargin, rect.top + cbYMargin, rect.left + cbXMargin + cbSize, rect.top + cbYMargin + cbSize };
                    CMPCThemeUtil::drawCheckBox(isChecked, false, true, rcb, &dcMem);
                }
            }

            if (IsWindowEnabled()) {
                if (GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED && (nSubItem == 0 || fullRowSelect)) {
                    bgColor = CMPCTheme::ContentSelectedColor;
                    if (LVS_REPORT != dwStyle) { //in list mode we don't fill the "whole" column
                        CRect tmp = rText;
                        dcMem.DrawText(text, tmp, textFormat | DT_CALCRECT); //end of string
                        rTextBG.right = tmp.right + (rText.left - rTextBG.left); //end of string plus same indent from the left side
                    }
                } else if (hasCheckedColors) {
                    if (isChecked && checkedBGClr != -1) {
                        bgColor = checkedBGClr;
                    }
                    if (isChecked && checkedTextClr != -1) {
                        dcMem.SetTextColor(checkedTextClr);
                    }
                    if (!isChecked && uncheckedTextClr != -1) {
                        dcMem.SetTextColor(uncheckedTextClr);
                    }
                }
                dcMem.FillSolidRect(rTextBG, bgColor);

                if (themeGridLines || nullptr != customThemeInterface) {
                    CRect rGrid = rect;
                    rGrid.bottom -= 1;
                    CPen gridPenV, gridPenH, *oldPen;
                    if (nullptr != customThemeInterface) {
                        COLORREF horzGridColor, vertGridColor;
                        customThemeInterface->GetCustomGridColors(nItem, horzGridColor, vertGridColor);
                        gridPenV.CreatePen(PS_SOLID, 1, vertGridColor);
                        gridPenH.CreatePen(PS_SOLID, 1, horzGridColor);
                    } else {
                        gridPenV.CreatePen(PS_SOLID, 1, CMPCTheme::ListCtrlGridColor);
                        gridPenH.CreatePen(PS_SOLID, 1, CMPCTheme::ListCtrlGridColor);
                    }

                    oldPen = dcMem.SelectObject(&gridPenV);
                    if (nSubItem != 0) {
                        dcMem.MoveTo(rGrid.TopLeft());
                        dcMem.LineTo(rGrid.left, rGrid.bottom);
                    } else {
                        dcMem.MoveTo(rGrid.left, rGrid.bottom);
                    }

                    dcMem.SelectObject(&gridPenH);
                    dcMem.LineTo(rGrid.BottomRight());

                    dcMem.SelectObject(&gridPenV);
                    dcMem.LineTo(rGrid.right, rGrid.top);

                    dcMem.SelectObject(oldPen);
                }
            }

            if (getFlaggedItem(nItem)) { //could be a setting, but flagged items are bold for now
                if (!listMPCThemeFontBold.m_hObject) {
                    listMPCThemeFont = GetFont();
                    LOGFONT lf;
                    listMPCThemeFont->GetLogFont(&lf);
                    lf.lfWeight = FW_BOLD;
                    listMPCThemeFontBold.CreateFontIndirect(&lf);
                }

                dcMem.SelectObject(listMPCThemeFontBold);
            }
            dcMem.DrawText(text, rText, textFormat);
            CMPCThemeUtil::flushMemDC(pDC, dcMem, rectDC);
            pDC->SetTextColor(oldTextColor);
            pDC->SetBkColor(oldBkColor);
        }
    }
}

BOOL CMPCThemePlayerListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

        *pResult = CDRF_DODEFAULT;
        if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
            if (nullptr != customThemeInterface) {
                customThemeInterface->DoCustomPrePaint();
            }
            *pResult = CDRF_NOTIFYITEMDRAW;
        } else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
            DWORD dwStyle = GetStyle() & LVS_TYPEMASK;
            if (LVS_REPORT == dwStyle) {
                *pResult = CDRF_NOTIFYSUBITEMDRAW;
            } else {
                int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
                CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
                drawItem(pDC, nItem, 0);
                *pResult = CDRF_SKIPDEFAULT;
            }
        } else if (pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
            int nItem = static_cast<int>(pLVCD->nmcd.dwItemSpec);
            if (IsItemVisible(nItem)) {
                int nSubItem = pLVCD->iSubItem;
                CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
                drawItem(pDC, nItem, nSubItem);
            }
            *pResult = CDRF_SKIPDEFAULT;
        }
        return TRUE;
    }
    return FALSE;
}


BOOL CMPCThemePlayerListCtrl::OnEraseBkgnd(CDC* pDC)
{
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        CRect r;
        GetClientRect(r);
        int dcState = pDC->SaveDC();
        for (int y = 0; y < GetItemCount(); y++) {
            CRect clip;
            GetItemRect(y, clip, LVIR_BOUNDS);
            pDC->ExcludeClipRect(clip);
        }
        pDC->FillSolidRect(r, CMPCTheme::ContentBGColor);

        if (themeGridLines || nullptr != customThemeInterface) {

            CPen gridPen, *oldPen;
            gridPen.CreatePen(PS_SOLID, 1, CMPCTheme::ListCtrlGridColor);
            oldPen = pDC->SelectObject(&gridPen);

            if (GetItemCount() > 0) {
                CRect gr;
                for (int x = 0; x < themedHdrCtrl.GetItemCount(); x++) {
                    themedHdrCtrl.GetItemRect(x, gr);
                    pDC->MoveTo(gr.right, r.top);
                    pDC->LineTo(gr.right, r.bottom);
                }
                gr.bottom = 0;
                for (int y = 0; y < GetItemCount() || gr.bottom < r.bottom; y++) {
                    if (y >= GetItemCount()) {
                        gr.OffsetRect(0, gr.Height());
                    } else {
                        GetItemRect(y, gr, LVIR_BOUNDS);
                    }
                    {
                        CPen horzPen;
                        pDC->MoveTo(r.left, gr.bottom - 1);
                        if (nullptr != customThemeInterface) {
                            COLORREF horzGridColor, tmp;
                            customThemeInterface->GetCustomGridColors(y, horzGridColor, tmp);
                            horzPen.CreatePen(PS_SOLID, 1, horzGridColor);
                            pDC->SelectObject(&horzPen);
                            pDC->LineTo(r.right, gr.bottom - 1);
                            pDC->SelectObject(&gridPen);
                            horzPen.DeleteObject();
                        } else {
                            pDC->LineTo(r.right, gr.bottom - 1);
                        }
                    }
                }
            }
            pDC->SelectObject(oldPen);
        }
        pDC->RestoreDC(dcState);
    } else {
        return __super::OnEraseBkgnd(pDC);
    }
    return TRUE;
}


HBRUSH CMPCThemePlayerListCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH ret;
    ret = getCtlColor(pDC, pWnd, nCtlColor);
    if (nullptr != ret) {
        return ret;
    } else {
        return __super::OnCtlColor(pDC, pWnd, nCtlColor);
    }
}


void CMPCThemePlayerListCtrl::OnHdnEndtrack(NMHDR* pNMHDR, LRESULT* pResult)
{
    //    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        if (nullptr != themedSBHelper) {
            themedSBHelper->updateScrollInfo();
        }
    }
    *pResult = 0;
}

LRESULT CMPCThemePlayerListCtrl::OnDelayed_updateListCtrl(WPARAM, LPARAM)
{
    updateScrollInfo();
    return 0;
}

BOOL CMPCThemePlayerListCtrl::OnLvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    //LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    const CAppSettings& s = AfxGetAppSettings();
    if (s.bMPCThemeLoaded) {
        ::PostMessage(m_hWnd, PLAYER_PLAYLIST_LVN_ITEMCHANGED, 0, 0);
    }
    *pResult = 0;
    return FALSE;
}
