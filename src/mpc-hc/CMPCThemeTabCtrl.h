#pragma once
#include "stdafx.h"
#include "CMPCThemeUtil.h"

class CMPCThemeTabCtrl : public CTabCtrl, public CMPCThemeUtil
{
public:
    CMPCThemeTabCtrl();
    virtual ~CMPCThemeTabCtrl();
    void PreSubclassWindow();
    DECLARE_DYNAMIC(CMPCThemeTabCtrl)
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
protected:
    void doDrawItem(int nItem, CRect rText, bool isSelected, CDC* pDC);
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
public:
    BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
};

