#pragma once
#include <afxwin.h>
#include "CMPCThemeScrollBar.h"
#include "CMPCThemeToolTipCtrl.h"
#include "CMPCThemeScrollBarHelper.h"

class CMPCThemeListBox :
    public CListBox, public CMPCThemeScrollable
{
    DECLARE_DYNAMIC(CMPCThemeListBox)
private:
    CMPCThemeScrollBar vertSB;
    CMPCThemeToolTipCtrl themedToolTip;
    UINT_PTR themedToolTipCid;
    CMPCThemeScrollBarHelper* themedSBHelper;
protected:
    virtual void PreSubclassWindow();
public:
    CMPCThemeListBox();
    virtual ~CMPCThemeListBox();
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    BOOL PreTranslateMessage(MSG* pMsg);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNcPaint();
public:
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnLbnSelchange();
    void updateToolTip(CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    void setIntegralHeight();
    afx_msg void OnSize(UINT nType, int cx, int cy);
};

