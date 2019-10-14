#pragma once
#include <afxcmn.h>
#include "CMPCThemeToolTipCtrl.h"

class CMPCThemeSliderCtrl :	public CSliderCtrl {
public:
	CMPCThemeSliderCtrl();
	virtual ~CMPCThemeSliderCtrl();
    virtual void PreSubclassWindow();
    DECLARE_DYNAMIC(CMPCThemeSliderCtrl)
    DECLARE_MESSAGE_MAP()
protected:
    CBrush bgBrush;
    bool m_bDrag, m_bHover;
    CMPCThemeToolTipCtrl themedToolTip;
public:
    afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
    void invalidateThumb();
    void checkHover(CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
};

