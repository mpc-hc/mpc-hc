#pragma once
#include <vector>
#include "mplayerc.h"
#include <afxcmn.h>
#include "CMPCTheme.h"

class CMPCThemeTitleBarControlButton;
class CMPCThemeUtil
{
public:
    CMPCThemeUtil();
    virtual ~CMPCThemeUtil();
    static bool ModifyTemplates(CPropertySheet* sheet, CRuntimeClass* pageClass, DWORD id, DWORD addStyle, DWORD removeStyle = 0);

    void enableFileDialogHook();

    static HBRUSH getCtlColorFileDialog(HDC hDC, UINT nCtlColor);
    HBRUSH getCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    static bool MPCThemeEraseBkgnd(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    void subClassFileDialog(CWnd* wnd, HWND hwnd, bool findSink = true);

protected:
    static CBrush contentBrush, windowBrush, controlAreaBrush, W10DarkThemeFileDialogInjectedBGBrush;
    static CFont dialogFont;
    std::vector<CWnd*> allocatedWindows;

    void fulfillThemeReqs(CWnd* wnd);
    void initHelperObjects(CWnd* wnd);
    void makeThemed(CWnd* pObject, CWnd* tChild);

public:
    static bool getFontByFace(CFont& font, CDC* pDC, wchar_t* fontName, int size, LONG weight = FW_REGULAR);
    static bool getFixedFont(CFont& font, CDC* pDC);
    static bool getFontByType(CFont& font, CDC* pDC, int type, bool underline = false, bool bold = false);
    enum fontType {
        CaptionFont,
        SmallCaptionFont,
        MenuFont,
        StatusFont,
        MessageFont,
        DialogFont,
        fixedFont,
    };

    static CSize GetTextSize(CString str, HDC hDC, CFont* font);
    static CSize GetTextSize(CString str, CDC* pDC, CFont* font);
    static CSize GetTextSize(CString str, HDC hDC, int type);
    static CSize GetTextSizeDiff(CString str, HDC hDC, int type, CFont* curFont);


    static void GetMetrics(NONCLIENTMETRICS* ncMetrics);
    static void initMemDC(CDC* pDC, CDC& dcMem, CBitmap& bmMem, CRect rect);
    static void flushMemDC(CDC* pDC, CDC& dcMem, CRect rect);
    static void DrawBufferedText(CDC* pDC, CString text, CRect rect, UINT format);
    static void Draw2BitTransparent(CDC& dc, int left, int top, int width, int height, CBitmap& bmp, COLORREF fgColor);
    static void dbg(CString text, ...);
    static float getConstantFByDPI(CWnd* window, const float* constants);
    static int getConstantByDPI(CWnd* window, const int* constants);
    static UINT getResourceByDPI(CDC* pDC, const UINT* resources);
    static const std::vector<CMPCTheme::pathPoint> getIconPathByDPI(CMPCThemeTitleBarControlButton* button);
    static void drawCheckBox(UINT checkState, bool isHover, bool useSystemSize, CRect rectCheck, CDC* pDC, bool isRadio = false);
    static bool canUseWin10DarkTheme();
    static UINT defaultLogo();
    static void drawParentDialogBGClr(CWnd* wnd, CDC* pDC, CRect r, bool fill = true);
    static void fulfillThemeReqs(CProgressCtrl* ctl);
    static void enableWindows10DarkFrame(CWnd* window);

    enum CheckBoxStyle {
        CheckBoxRegular = 0,
        CheckBoxHover = 1,
    };

    enum RadioStyle {
        RadioRegular = 0,
        RadioRegularSet = 1,
        RadioHover = 2,
        RadioHoverSet = 3
    };
};


