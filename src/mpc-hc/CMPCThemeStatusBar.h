#pragma once
#include <afxext.h>
class CMPCThemeStatusBar :
    public CStatusBar
{
public:
    CMPCThemeStatusBar();
    virtual ~CMPCThemeStatusBar();
    void PreSubclassWindow();
    void SetText(LPCTSTR lpszText, int nPane, int nType);
    BOOL SetParts(int nParts, int* pWidths);
    int GetParts(int nParts, int* pParts);
    BOOL GetRect(int nPane, LPRECT lpRect);
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    DECLARE_MESSAGE_MAP()
protected:
    std::map<int, CString> texts;
    int numParts;
public:
    afx_msg void OnNcPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

