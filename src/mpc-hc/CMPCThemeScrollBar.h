#pragma once
#include <afxwin.h>
#include "XeScrollBar/XeScrollBarBase.h"

class CMPCThemeScrollBar :	public CXeScrollBarBase
{
    DECLARE_DYNAMIC(CMPCThemeScrollBar)
public:
    CMPCThemeScrollBar();
    virtual ~CMPCThemeScrollBar();
    void DrawScrollBar(CDC * pDC);
    virtual void SendScrollMsg(WORD wSBcode, WORD wHiWPARAM);
    void setScrollWindow(CWnd* window);
    void updateScrollInfo();
    BOOL PreTranslateMessage(MSG* pMsg);
    void updateScrollInfo(int nPos);
protected:
    CWnd* m_scrollWindow; //real parent is window we overlay the SB
    DECLARE_MESSAGE_MAP()
    UINT scrollLines;
    bool haveInitScrollInfo;
    bool disableNoScroll;
};

