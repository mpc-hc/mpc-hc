#pragma once
#include "stdafx.h"
#include "CMPCThemeScrollBar.h"
class CMPCThemeTreeCtrl;

class CMPCThemeScrollable
{
public:
    CMPCThemeScrollable() {};
    ~CMPCThemeScrollable() {};
    virtual void doDefault() {};
};

class CMPCThemeScrollBarHelper
{
protected:
    CWnd* window, *pParent;
    CMPCThemeScrollBar vertSB, horzSB;
    bool hasVSB;
    bool hasHSB;
    static void doNcPaint(CWnd* window);
public:
    CMPCThemeScrollBarHelper(CWnd* scrollWindow);
    ~CMPCThemeScrollBarHelper();
    void createSB();
    void setDrawingArea(CRect& cr, CRect& wr, bool clipping);
    void hideSB();
    void updateScrollInfo();
    bool WindowProc(CListCtrl* list, UINT message, WPARAM wParam, LPARAM lParam);
    bool WindowProc(CTreeCtrl* tree, UINT message, WPARAM wParam, LPARAM lParam);
    void themedNcPaintWithSB();
    static void themedNcPaint(CWnd* window, CMPCThemeScrollable* swindow);
};

