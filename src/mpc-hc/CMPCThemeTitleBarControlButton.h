#pragma once
#include "CMPCThemeFrameUtil.h"

class CMPCThemeTitleBarControlButton :
    public CMFCButton
{
public:
    CMPCThemeTitleBarControlButton(WPARAM _buttonType);
    void setParentFrame(CMPCThemeFrameUtil* parent);
    WPARAM getButtonType();
protected:
    void drawTitleBarButton(CDC* pDC, CRect iconRect, std::vector<CMPCTheme::pathPoint> icon, double dpiScaling, bool antiAlias = false);
    WPARAM buttonType;
    COLORREF hoverColor, pushedColor, hoverInactiveColor;
    CMPCThemeFrameUtil* parent;
public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnBnClicked();
};

