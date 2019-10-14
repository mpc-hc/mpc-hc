#pragma once
#include <afxcmn.h>
#include "CMPCThemeScrollBarHelper.h"
#include "CMPCThemeToolTipCtrl.h"

class CMPCThemeTreeCtrl : public CTreeCtrl
    , public CMPCThemeScrollable
{
public:
	CMPCThemeTreeCtrl();
	virtual ~CMPCThemeTreeCtrl();
    BOOL PreCreateWindow(CREATESTRUCT & cs);
    void fulfillThemeReqs();
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    void updateToolTip(CPoint point);
    BOOL PreTranslateMessage(MSG* pMsg);
    DECLARE_DYNAMIC(CMPCThemeTreeCtrl)
    DECLARE_MESSAGE_MAP()
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnNcPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
protected:
    CBrush m_brBkgnd;
    CFont font;
    CMPCThemeScrollBarHelper *themedSBHelper;
    CMPCThemeToolTipCtrl themedToolTip, tvsTooltip;
    UINT_PTR themedToolTipCid;
    void doEraseBkgnd(CDC* pDC);
public:
    void doDefault() { Default(); }
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};

