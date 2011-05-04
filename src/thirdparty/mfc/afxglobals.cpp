// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "comdef.h"
// #include "afxcontrolbarutil.h"
#include "afxglobals.h"

#if 0
#include "afxvisualmanager.h"
#include "afxkeyboardmanager.h"
#include "afxmenuhash.h"
#include "afxtoolbar.h"
#include "afxmenuimages.h"
#include "afxpaneframewnd.h"
#include "afxdockingmanager.h"
#include "afxvisualmanageroffice2007.h"
#include "afxribbonres.h"

#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#endif

extern CObList afxAllToolBars;

BOOL CMemDC::m_bUseMemoryDC = TRUE;

static const CString strOfficeFontName = _T("Tahoma");
static const CString strOffice2007FontName = _T("Segoe UI");
static const CString strDefaultFontName = _T("MS Sans Serif");
static const CString strVertFontName = _T("Arial");
static const CString strMarlettFontName = _T("Marlett");

HINSTANCE AFX_GLOBAL_DATA::m_hinstD2DDLL = NULL;
HINSTANCE AFX_GLOBAL_DATA::m_hinstDWriteDLL = NULL;

ID2D1Factory* AFX_GLOBAL_DATA::m_pDirect2dFactory = NULL;
IDWriteFactory* AFX_GLOBAL_DATA::m_pWriteFactory = NULL;
IWICImagingFactory* AFX_GLOBAL_DATA::m_pWicFactory = NULL;

D2D1MAKEROTATEMATRIX AFX_GLOBAL_DATA::m_pfD2D1MakeRotateMatrix = NULL;

BOOL AFX_GLOBAL_DATA::m_bD2DInitialized = FALSE;

CMemDC::CMemDC(CDC& dc, CWnd* pWnd) :
	m_dc(dc), m_bMemDC(FALSE), m_hBufferedPaint(NULL), m_pOldBmp(NULL)
{
	ASSERT_VALID(pWnd);

	pWnd->GetClientRect(m_rect);

	m_rect.right += pWnd->GetScrollPos(SB_HORZ);
	m_rect.bottom += pWnd->GetScrollPos(SB_VERT);

	if (afxGlobalData.m_pfBeginBufferedPaint != NULL && afxGlobalData.m_pfEndBufferedPaint != NULL)
	{
		HDC hdcPaint = NULL;

		if (!afxGlobalData.m_bBufferedPaintInited && afxGlobalData.m_pfBufferedPaintInit != NULL && afxGlobalData.m_pfBufferedPaintUnInit != NULL)
		{
			afxGlobalData.m_pfBufferedPaintInit();
			afxGlobalData.m_bBufferedPaintInited = TRUE;
		}

		m_hBufferedPaint = (*afxGlobalData.m_pfBeginBufferedPaint)(dc.GetSafeHdc(), m_rect, AFX_BPBF_TOPDOWNDIB, NULL, &hdcPaint);

		if (m_hBufferedPaint != NULL && hdcPaint != NULL)
		{
			m_bMemDC = TRUE;
			m_dcMem.Attach(hdcPaint);
		}
	}
	else
	{
		if (m_bUseMemoryDC && m_dcMem.CreateCompatibleDC(&m_dc) && m_bmp.CreateCompatibleBitmap(&m_dc, m_rect.Width(), m_rect.Height()))
		{
			//-------------------------------------------------------------
			// Off-screen DC successfully created. Better paint to it then!
			//-------------------------------------------------------------
			m_bMemDC = TRUE;
			m_pOldBmp = m_dcMem.SelectObject(&m_bmp);
		}
	}
}

CMemDC::CMemDC(CDC& dc, const CRect& rect) :
	m_dc(dc), m_bMemDC(FALSE), m_hBufferedPaint(NULL), m_pOldBmp(NULL), m_rect(rect)
{
	ASSERT(!m_rect.IsRectEmpty());

	if (afxGlobalData.m_pfBeginBufferedPaint != NULL && afxGlobalData.m_pfEndBufferedPaint != NULL)
	{
		HDC hdcPaint = NULL;

		if (!afxGlobalData.m_bBufferedPaintInited && afxGlobalData.m_pfBufferedPaintInit != NULL && afxGlobalData.m_pfBufferedPaintUnInit != NULL)
		{
			afxGlobalData.m_pfBufferedPaintInit();
			afxGlobalData.m_bBufferedPaintInited = TRUE;
		}

		m_hBufferedPaint = (*afxGlobalData.m_pfBeginBufferedPaint)(dc.GetSafeHdc(), m_rect, AFX_BPBF_TOPDOWNDIB, NULL, &hdcPaint);

		if (m_hBufferedPaint != NULL && hdcPaint != NULL)
		{
			m_bMemDC = TRUE;
			m_dcMem.Attach(hdcPaint);
		}
	}
	else
	{
		if (m_bUseMemoryDC && m_dcMem.CreateCompatibleDC(&m_dc) && m_bmp.CreateCompatibleBitmap(&m_dc, m_rect.Width(), m_rect.Height()))
		{
			//-------------------------------------------------------------
			// Off-screen DC successfully created. Better paint to it then!
			//-------------------------------------------------------------
			m_bMemDC = TRUE;
			m_pOldBmp = m_dcMem.SelectObject(&m_bmp);
		}
	}
}

CMemDC::~CMemDC()
{
	if (m_hBufferedPaint != NULL)
	{
		m_dcMem.Detach();
		(*afxGlobalData.m_pfEndBufferedPaint)(m_hBufferedPaint, TRUE);
	}
	else if (m_bMemDC)
	{
		//--------------------------------------
		// Copy the results to the on-screen DC:
		//--------------------------------------
		CRect rectClip;
		int nClipType = m_dc.GetClipBox(rectClip);

		if (nClipType != NULLREGION)
		{
			if (nClipType != SIMPLEREGION)
			{
				rectClip = m_rect;
			}

			m_dc.BitBlt(rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(), &m_dcMem, rectClip.left, rectClip.top, SRCCOPY);
		}

		m_dcMem.SelectObject(m_pOldBmp);
	}
}

static int CALLBACK FontFamalyProcFonts(const LOGFONT FAR* lplf, const TEXTMETRIC FAR* /*lptm*/, ULONG /*ulFontType*/, LPARAM lParam)
{
	ENSURE(lplf != NULL);
	ENSURE(lParam != NULL);

	CString strFont = lplf->lfFaceName;
	return strFont.CollateNoCase((LPCTSTR) lParam) == 0 ? 0 : 1;
}

/////////////////////////////////////////////////////////////////////////////
// DLL Load Helper

inline HMODULE AfxLoadSystemLibraryUsingFullPath(_In_z_ const WCHAR *pszLibrary)
{
	WCHAR wszLoadPath[MAX_PATH+1];
	if (::GetSystemDirectoryW(wszLoadPath, _countof(wszLoadPath)) == 0)
	{
		return NULL;
	}

	if (wszLoadPath[wcslen(wszLoadPath)-1] != L'\\')
	{
		if (wcscat_s(wszLoadPath, _countof(wszLoadPath), L"\\") != 0)
		{
			return NULL;
		}
	}

	if (wcscat_s(wszLoadPath, _countof(wszLoadPath), pszLibrary) != 0)
	{
		return NULL;
	}

	return(::AfxCtxLoadLibraryW(wszLoadPath));
}

/////////////////////////////////////////////////////////////////////////////
// Cached system metrics, etc
AFX_GLOBAL_DATA afxGlobalData;

#ifdef _AFXDLL
// Reference count on global data
DWORD g_dwAfxGlobalDataRef = 0;
#endif

// Initialization code
AFX_GLOBAL_DATA::AFX_GLOBAL_DATA()
{
	// Detect the kind of OS:
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionEx(&osvi);

	bIsRemoteSession = GetSystemMetrics(SM_REMOTESESSION);

	bIsWindowsVista = (osvi.dwMajorVersion >= 6);
	bIsWindows7 = (osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 1) || (osvi.dwMajorVersion > 6) ;
	bDisableAero = FALSE;

	m_bIsRibbonImageScale = TRUE;

	// Cached system values(updated in CWnd::OnSysColorChange)
	hbrBtnShadow = NULL;
	hbrBtnHilite = NULL;
	hbrWindow = NULL;

	UpdateSysColors();

	m_hinstUXThemeDLL = ::AfxCtxLoadLibraryW(L"UxTheme.dll");
	if (m_hinstUXThemeDLL != NULL)
	{
		m_pfDrawThemeBackground = (DRAWTHEMEPARENTBACKGROUND)::GetProcAddress(m_hinstUXThemeDLL, "DrawThemeParentBackground");
		m_pfDrawThemeTextEx = (DRAWTHEMETEXTEX)::GetProcAddress(m_hinstUXThemeDLL, "DrawThemeTextEx");

		m_pfBufferedPaintInit = (BUFFEREDPAINTINIT)::GetProcAddress(m_hinstUXThemeDLL, "BufferedPaintInit");
		m_pfBufferedPaintUnInit = (BUFFEREDPAINTUNINIT)::GetProcAddress(m_hinstUXThemeDLL, "BufferedPaintUnInit");

		m_pfBeginBufferedPaint = (BEGINBUFFEREDPAINT)::GetProcAddress(m_hinstUXThemeDLL, "BeginBufferedPaint");
		m_pfEndBufferedPaint = (ENDBUFFEREDPAINT)::GetProcAddress(m_hinstUXThemeDLL, "EndBufferedPaint");
	}
	else
	{
		m_pfDrawThemeBackground = NULL;
		m_pfDrawThemeTextEx = NULL;

		m_pfBufferedPaintInit = NULL;
		m_pfBufferedPaintUnInit = NULL;

		m_pfBeginBufferedPaint = NULL;
		m_pfEndBufferedPaint = NULL;
	}

	m_hinstDwmapiDLL = AfxLoadSystemLibraryUsingFullPath(L"dwmapi.dll");
	if (m_hinstDwmapiDLL != NULL)
	{
		m_pfDwmExtendFrameIntoClientArea = (DWMEXTENDFRAMEINTOCLIENTAREA)::GetProcAddress(m_hinstDwmapiDLL, "DwmExtendFrameIntoClientArea");
		m_pfDwmDefWindowProc = (DWMDEFWINDOWPROC) ::GetProcAddress(m_hinstDwmapiDLL, "DwmDefWindowProc");
		m_pfDwmIsCompositionEnabled = (DWMISCOMPOSITIONENABLED)::GetProcAddress(m_hinstDwmapiDLL, "DwmIsCompositionEnabled");
	}
	else
	{
		m_pfDwmExtendFrameIntoClientArea = NULL;
		m_pfDwmDefWindowProc = NULL;
		m_pfDwmIsCompositionEnabled = NULL;
	}

	m_hcurStretch = NULL;
	m_hcurStretchVert = NULL;
	m_hcurHand = NULL;
	m_hcurSizeAll = NULL;
	m_hiconTool = NULL;
	m_hiconLink = NULL;
	m_hiconColors = NULL;
	m_hcurMoveTab = NULL;
	m_hcurNoMoveTab = NULL;

	m_bUseSystemFont = FALSE;
	m_bInSettingChange = FALSE;

	UpdateFonts();
	OnSettingChange();

	m_bIsRTL = FALSE;
	m_bBufferedPaintInited = FALSE;

	m_nDragFrameThicknessFloat = 4;  // pixels
	m_nDragFrameThicknessDock = 3;   // pixels

	m_nAutoHideToolBarSpacing = 14; // pixels
	m_nAutoHideToolBarMargin  = 4;  // pixels

	m_nCoveredMainWndClientAreaPercent = 50; // percents

	m_nMaxToolTipWidth = -1;
	m_bIsBlackHighContrast = FALSE;
	m_bIsWhiteHighContrast = FALSE;

	m_bUseBuiltIn32BitIcons = TRUE;

	m_bComInitialized = FALSE;

	m_pTaskbarList = NULL;
	m_pTaskbarList3 = NULL;
	m_bTaskBarInterfacesAvailable = TRUE;

	EnableAccessibilitySupport();
}

AFX_GLOBAL_DATA::~AFX_GLOBAL_DATA()
{
	CleanUp();
}

void AFX_GLOBAL_DATA::UpdateFonts()
{
	CWindowDC dc(NULL);
	m_dblRibbonImageScale = dc.GetDeviceCaps(LOGPIXELSX) / 96.0f;

	if (m_dblRibbonImageScale > 1. && m_dblRibbonImageScale < 1.1)
	{
		m_dblRibbonImageScale = 1.;
	}

	if (fontRegular.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontRegular.Detach());
	}

	if (fontTooltip.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontTooltip.Detach());
	}

	if (fontBold.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontBold.Detach());
	}

	if (fontDefaultGUIBold.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontDefaultGUIBold.Detach());
	}

	if (fontUnderline.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontUnderline.Detach());
	}

	if (fontDefaultGUIUnderline.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontDefaultGUIUnderline.Detach());
	}

	if (fontVert.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontVert.Detach());
	}

	if (fontVertCaption.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontVertCaption.Detach());
	}

	if (fontMarlett.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontMarlett.Detach());
	}

	if (fontSmall.GetSafeHandle() != NULL)
	{
		::DeleteObject(fontSmall.Detach());
	}

	// Initialize fonts:

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);
	GetNonClientMetrics (info);

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	lf.lfCharSet = (BYTE) GetTextCharsetInfo(dc.GetSafeHdc(), NULL, 0);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	//------------------
	// Adjust font size:
	//------------------
	int nFontHeight = lf.lfHeight < 0 ? -lf.lfHeight : lf.lfHeight;
	if (nFontHeight <= 12)
	{
		nFontHeight = 11;
	}
	else
	{
		nFontHeight--;
	}

	lf.lfHeight = (lf.lfHeight < 0) ? -nFontHeight : nFontHeight;

	// Check if we should use system font
	lstrcpy(lf.lfFaceName, info.lfMenuFont.lfFaceName);

	BOOL fUseSystemFont = m_bUseSystemFont || (info.lfMenuFont.lfCharSet > SYMBOL_CHARSET);
	if (!fUseSystemFont)
	{
		// Check for "Segoe UI" or "Tahoma" font existance:
		if (::EnumFontFamilies(dc.GetSafeHdc(), NULL, FontFamalyProcFonts, (LPARAM)(LPCTSTR) strOffice2007FontName) == 0)
		{
			// Found! Use MS Office 2007 font!
			lstrcpy(lf.lfFaceName, strOffice2007FontName);
			lf.lfQuality = 5 /*CLEARTYPE_QUALITY*/;
		}
		else if (::EnumFontFamilies(dc.GetSafeHdc(), NULL, FontFamalyProcFonts, (LPARAM)(LPCTSTR) strOfficeFontName) == 0)
		{
			// Found! Use MS Office font!
			lstrcpy(lf.lfFaceName, strOfficeFontName);
		}
		else
		{
			// Not found. Use default font:
			lstrcpy(lf.lfFaceName, strDefaultFontName);
		}
	}
	
	fontRegular.CreateFontIndirect(&lf);

	// Create small font:
	LONG lfHeightSaved = lf.lfHeight;

	lf.lfHeight = (long)((1. + abs(lf.lfHeight)) * 2 / 3);
	if (lfHeightSaved < 0)
	{
		lf.lfHeight = -lf.lfHeight;
	}

	fontSmall.CreateFontIndirect(&lf);
	lf.lfHeight = lfHeightSaved;

	// Create tooltip font:
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	GetNonClientMetrics (ncm);

	lf.lfItalic = ncm.lfStatusFont.lfItalic;
	lf.lfWeight = ncm.lfStatusFont.lfWeight;
	fontTooltip.CreateFontIndirect(&lf);

	lf.lfItalic = info.lfMenuFont.lfItalic;
	lf.lfWeight = info.lfMenuFont.lfWeight;

	// Create "underline" font:
	lf.lfUnderline = TRUE;
	fontUnderline.CreateFontIndirect(&lf);
	lf.lfUnderline = FALSE;

	// Create bold font:
	lf.lfWeight = FW_BOLD;
	fontBold.CreateFontIndirect(&lf);

	// Create Marlett font:
	BYTE bCharSet = lf.lfCharSet;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfCharSet = SYMBOL_CHARSET;
	lf.lfWeight = 0;
	lf.lfHeight = ::GetSystemMetrics(SM_CYMENUCHECK) - 1;
	lstrcpy(lf.lfFaceName, strMarlettFontName);

	fontMarlett.CreateFontIndirect(&lf);
	lf.lfCharSet = bCharSet; // Restore charset

	// Create vertical font:
	CFont font;
	if (font.CreateStockObject(DEFAULT_GUI_FONT))
	{
		if (font.GetLogFont(&lf) != 0)
		{
			lf.lfOrientation = 900;
			lf.lfEscapement = 2700;

			lf.lfHeight = info.lfMenuFont.lfHeight;
			lf.lfWeight = info.lfMenuFont.lfWeight;
			lf.lfItalic = info.lfMenuFont.lfItalic;

			{
				lstrcpy(lf.lfFaceName, strVertFontName);
			}

			fontVert.CreateFontIndirect(&lf);

			lf.lfEscapement = 900;
			fontVertCaption.CreateFontIndirect(&lf);
		}
	}

	// Create dialog underline and bold fonts:
	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pDefaultGUIFont);
	pDefaultGUIFont->GetLogFont(&lf);

	lf.lfUnderline = TRUE;
	fontDefaultGUIUnderline.CreateFontIndirect(&lf);
	lf.lfUnderline = FALSE;

	lf.lfWeight = FW_BOLD;
	fontDefaultGUIBold.CreateFontIndirect(&lf);

	UpdateTextMetrics();

#if 0
	// Notify toolbars about font changing:
	for (POSITION posTlb = afxAllToolBars.GetHeadPosition(); posTlb != NULL;)
	{
		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(posTlb);
		ENSURE(pToolBar != NULL);

		if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);
			pToolBar->OnGlobalFontsChanged();
		}
	}
#endif
}

static BOOL CALLBACK InfoEnumProc( HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT /*lprcMonitor*/, LPARAM dwData)
{
	CRect* pRect = (CRect*) dwData;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	if (GetMonitorInfo(hMonitor, &mi))
	{
		CRect rectMon = mi.rcWork;

		pRect->left = min(pRect->left, rectMon.left);
		pRect->right = max(pRect->right, rectMon.right);
		pRect->top = min(pRect->top, rectMon.top);
		pRect->bottom = max(pRect->bottom, rectMon.bottom);
	}

	return TRUE;
}

void AFX_GLOBAL_DATA::OnSettingChange()
{
	m_bInSettingChange = TRUE;

	m_sizeSmallIcon.cx = ::GetSystemMetrics(SM_CXSMICON);
	m_sizeSmallIcon.cy = ::GetSystemMetrics(SM_CYSMICON);

	m_rectVirtual.SetRectEmpty();

	if (!EnumDisplayMonitors(NULL, NULL, InfoEnumProc, (LPARAM) &m_rectVirtual))
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &m_rectVirtual, 0);
	}

	// Get system menu animation type:
	m_bMenuAnimation = FALSE;
	m_bMenuFadeEffect = FALSE;

	if (!bIsRemoteSession)
	{
		::SystemParametersInfo(SPI_GETMENUANIMATION, 0, &m_bMenuAnimation, 0);

		if (m_bMenuAnimation)
		{
			::SystemParametersInfo(SPI_GETMENUFADE, 0, &m_bMenuFadeEffect, 0);
		}
	}

	m_nShellAutohideBars = 0;
	m_bRefreshAutohideBars = TRUE;

	::SystemParametersInfo(SPI_GETMENUUNDERLINES, 0, &m_bSysUnderlineKeyboardShortcuts, 0);
	m_bUnderlineKeyboardShortcuts = m_bSysUnderlineKeyboardShortcuts;

	m_bInSettingChange = FALSE;
}

void AFX_GLOBAL_DATA::UpdateSysColors()
{
	m_bIsBlackHighContrast = ::GetSysColor(COLOR_3DLIGHT) == RGB(255, 255, 255) && ::GetSysColor(COLOR_3DFACE) == RGB(0, 0, 0);
	m_bIsWhiteHighContrast = ::GetSysColor(COLOR_3DDKSHADOW) == RGB(0, 0, 0) && ::GetSysColor(COLOR_3DFACE) == RGB(255, 255, 255);

	CWindowDC dc(NULL);
	m_nBitsPerPixel = dc.GetDeviceCaps(BITSPIXEL);

	clrBarFace = clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
	clrBarShadow = clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrBarDkShadow = clrBtnDkShadow = ::GetSysColor(COLOR_3DDKSHADOW);
	clrBarLight = clrBtnLight = ::GetSysColor(COLOR_3DLIGHT);
	clrBarHilite = clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	clrBarText = clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
	clrGrayedText = ::GetSysColor(COLOR_GRAYTEXT);
	clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);

	clrHilite = ::GetSysColor(COLOR_HIGHLIGHT);
	clrTextHilite = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

	clrBarWindow = clrWindow = ::GetSysColor(COLOR_WINDOW);
	clrWindowText = ::GetSysColor(COLOR_WINDOWTEXT);

	clrCaptionText = ::GetSysColor(COLOR_CAPTIONTEXT);
	clrMenuText = ::GetSysColor(COLOR_MENUTEXT);

	clrActiveCaption = ::GetSysColor(COLOR_ACTIVECAPTION);
	clrInactiveCaption = ::GetSysColor(COLOR_INACTIVECAPTION);

	clrActiveCaptionGradient = ::GetSysColor(COLOR_GRADIENTACTIVECAPTION);
	clrInactiveCaptionGradient = ::GetSysColor(COLOR_GRADIENTINACTIVECAPTION);

	clrActiveBorder = ::GetSysColor(COLOR_ACTIVEBORDER);
	clrInactiveBorder = ::GetSysColor(COLOR_INACTIVEBORDER);

	clrInactiveCaptionText = ::GetSysColor(COLOR_INACTIVECAPTIONTEXT);

	if (m_bIsBlackHighContrast)
	{
		clrHotLinkNormalText = clrWindowText;
		clrHotLinkHoveredText = clrWindowText;
		clrHotLinkVisitedText = clrWindowText;
	}
	else
	{
		clrHotLinkNormalText = ::GetSysColor(COLOR_HOTLIGHT);
		clrHotLinkHoveredText = RGB(0, 0, 255);   // Bright blue
		clrHotLinkVisitedText = RGB(128, 0, 128); // Violet
	}

	hbrBtnShadow = ::GetSysColorBrush(COLOR_BTNSHADOW);
	ENSURE(hbrBtnShadow != NULL);

	hbrBtnHilite = ::GetSysColorBrush(COLOR_BTNHIGHLIGHT);
	ENSURE(hbrBtnHilite != NULL);

	hbrWindow = ::GetSysColorBrush(COLOR_WINDOW);
	ENSURE(hbrWindow != NULL);

	brBtnFace.DeleteObject();
	brBtnFace.CreateSolidBrush(clrBtnFace);

	brBarFace.DeleteObject();
	brBarFace.CreateSolidBrush(clrBarFace);

	brActiveCaption.DeleteObject();
	brActiveCaption.CreateSolidBrush(clrActiveCaption);

	brInactiveCaption.DeleteObject();
	brInactiveCaption.CreateSolidBrush(clrInactiveCaption);

	brHilite.DeleteObject();
	brHilite.CreateSolidBrush(clrHilite);

	brBlack.DeleteObject();
	brBlack.CreateSolidBrush(clrBtnDkShadow);

	brWindow.DeleteObject();
	brWindow.CreateSolidBrush(clrWindow);

	penHilite.DeleteObject();
	penHilite.CreatePen(PS_SOLID, 1, afxGlobalData.clrHilite);

	penBarFace.DeleteObject();
	penBarFace.CreatePen(PS_SOLID, 1, afxGlobalData.clrBarFace);

	penBarShadow.DeleteObject();
	penBarShadow.CreatePen(PS_SOLID, 1, afxGlobalData.clrBarShadow);

	if (brLight.GetSafeHandle())
	{
		brLight.DeleteObject();
	}

	if (m_nBitsPerPixel > 8)
	{
		COLORREF clrLight = RGB(GetRValue(clrBtnFace) +((GetRValue(clrBtnHilite) - GetRValue(clrBtnFace)) / 2 ),
			GetGValue(clrBtnFace) +((GetGValue(clrBtnHilite) - GetGValue(clrBtnFace)) / 2),
			GetBValue(clrBtnFace) +((GetBValue(clrBtnHilite) - GetBValue(clrBtnFace)) / 2));

		brLight.CreateSolidBrush(clrLight);
	}
	else
	{
		HBITMAP hbmGray = CreateDitherBitmap(dc.GetSafeHdc());
		ENSURE(hbmGray != NULL);

		CBitmap bmp;
		bmp.Attach(hbmGray);

		brLight.CreatePatternBrush(&bmp);
	}

//	CMenuImages::CleanUp();
//	CDockingManager::m_bSDParamsModified = TRUE;
}

BOOL AFX_GLOBAL_DATA::SetMenuFont(LPLOGFONT lpLogFont, BOOL bHorz)
{
	ENSURE(lpLogFont != NULL);

	if (bHorz)
	{
		// Create regular font:
		fontRegular.DeleteObject();
		if (!fontRegular.CreateFontIndirect(lpLogFont))
		{
			ASSERT(FALSE);
			return FALSE;
		}

		// Create underline font:
		lpLogFont->lfUnderline = TRUE;
		fontUnderline.DeleteObject();
		fontUnderline.CreateFontIndirect(lpLogFont);
		lpLogFont->lfUnderline = FALSE;

		// Create bold font(used in the default menu items):
		long lSavedWeight = lpLogFont->lfWeight;
		lpLogFont->lfWeight = 700;

		fontBold.DeleteObject();
		BOOL bResult = fontBold.CreateFontIndirect(lpLogFont);

		lpLogFont->lfWeight = lSavedWeight; // Restore weight

		if (!bResult)
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}
	else // Vertical font
	{
		fontVert.DeleteObject();
		if (!fontVert.CreateFontIndirect(lpLogFont))
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}

	UpdateTextMetrics();
	return TRUE;
}

void AFX_GLOBAL_DATA::UpdateTextMetrics()
{
	CWindowDC dc(NULL);

	CFont* pOldFont = dc.SelectObject(&fontRegular);
	ENSURE(pOldFont != NULL);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	int nExtra = tm.tmHeight < 15 ? 2 : 5;

	m_nTextHeightHorz = tm.tmHeight + nExtra;
	m_nTextWidthHorz = tm.tmMaxCharWidth + nExtra;

	dc.SelectObject(&fontVert);
	dc.GetTextMetrics(&tm);

	nExtra = tm.tmHeight < 15 ? 2 : 5;

	m_nTextHeightVert = tm.tmHeight + nExtra;
	m_nTextWidthVert = tm.tmMaxCharWidth + nExtra;

	dc.SelectObject(pOldFont);
}

HBITMAP AFX_GLOBAL_DATA::CreateDitherBitmap(HDC hDC)
{
	struct  // BITMAPINFO with 16 colors
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD      bmiColors[16];
	}
	bmi;
	memset(&bmi, 0, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 8;
	bmi.bmiHeader.biHeight = 8;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	COLORREF clr = afxGlobalData.clrBtnFace;

	bmi.bmiColors[0].rgbBlue = GetBValue(clr);
	bmi.bmiColors[0].rgbGreen = GetGValue(clr);
	bmi.bmiColors[0].rgbRed = GetRValue(clr);

	clr = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	bmi.bmiColors[1].rgbBlue = GetBValue(clr);
	bmi.bmiColors[1].rgbGreen = GetGValue(clr);
	bmi.bmiColors[1].rgbRed = GetRValue(clr);

	// initialize the brushes
	long patGray[8];
	for (int i = 0; i < 8; i++)
		patGray[i] = (i & 1) ? 0xAAAA5555L : 0x5555AAAAL;

	HBITMAP hbm = CreateDIBitmap(hDC, &bmi.bmiHeader, CBM_INIT, (LPBYTE)patGray, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	return hbm;
}

#if (WINVER >= 0x0601)
ITaskbarList* AFX_GLOBAL_DATA::GetITaskbarList()
{
	HRESULT hr = S_OK;

	if (!bIsWindows7 || !m_bTaskBarInterfacesAvailable)
	{
		return NULL;
	}

	if (m_pTaskbarList != NULL)
	{
		return m_pTaskbarList;
	}

	if (!m_bComInitialized)
	{
		hr = CoInitialize(NULL);
		if (SUCCEEDED(hr))
		{
			m_bComInitialized = TRUE;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTaskbarList));
	}

	ASSERT(SUCCEEDED(hr));
	return m_pTaskbarList;
}

ITaskbarList3* AFX_GLOBAL_DATA::GetITaskbarList3()
{
	HRESULT hr = S_OK;

	if (!bIsWindows7 || !m_bTaskBarInterfacesAvailable)
	{
		return NULL;
	}

	if (m_pTaskbarList3 != NULL)
	{
		return m_pTaskbarList3;
	}

	if (!m_bComInitialized)
	{
		hr = CoInitialize(NULL);
		if (SUCCEEDED(hr))
		{
			m_bComInitialized = TRUE;
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTaskbarList3));
	}

	ASSERT(SUCCEEDED(hr));
	return m_pTaskbarList3;
}

void AFX_GLOBAL_DATA::ReleaseTaskBarRefs()
{
	m_bTaskBarInterfacesAvailable = FALSE;

	if (m_pTaskbarList != NULL)
	{
		RELEASE(m_pTaskbarList);
		m_pTaskbarList = NULL;
	}

	if (m_pTaskbarList3 != NULL)
	{
		RELEASE(m_pTaskbarList3);
		m_pTaskbarList3 = NULL;
	}
}
#endif

void AFX_GLOBAL_DATA::CleanUp()
{
	if (brLight.GetSafeHandle())
	{
		brLight.DeleteObject();
	}

	// cleanup fonts:
	fontRegular.DeleteObject();
	fontBold.DeleteObject();
	fontUnderline.DeleteObject();
	fontVert.DeleteObject();
	fontVertCaption.DeleteObject();
	fontTooltip.DeleteObject();

	ReleaseTaskBarRefs();
	ReleaseD2DRefs();

	if (m_bBufferedPaintInited && m_pfBufferedPaintUnInit != NULL)
	{
		m_pfBufferedPaintUnInit();
		m_bBufferedPaintInited = FALSE;
	}

	if (m_hinstUXThemeDLL != NULL)
	{
		::FreeLibrary(m_hinstUXThemeDLL);
		m_hinstUXThemeDLL = NULL;
	}

	if (m_hinstDwmapiDLL != NULL)
	{
		::FreeLibrary(m_hinstDwmapiDLL);
		m_hinstDwmapiDLL = NULL;
	}

	m_bEnableAccessibility = FALSE;

	if (m_bComInitialized)
	{
		CoUninitialize();
		m_bComInitialized = FALSE;
	}
}

void ControlBarCleanUp()
{
	afxGlobalData.CleanUp();

//	afxMenuHash.CleanUp();

//	CMFCToolBar::CleanUpImages();
//	CMenuImages::CleanUp();

//	if (GetCmdMgr() != NULL)
//	{
//		GetCmdMgr()->CleanUp();
//	}

//	CKeyboardManager::CleanUp();

	// Destroy visualization manager:
//	CMFCVisualManager::DestroyInstance(TRUE /* bAutoDestroyOnly */);
//	CMFCVisualManagerOffice2007::CleanStyle();
}

#ifdef _AFXDLL
void AfxGlobalsAddRef()
{
	g_dwAfxGlobalDataRef++;
}

void AfxGlobalsRelease()
{
	g_dwAfxGlobalDataRef--;
	if (g_dwAfxGlobalDataRef == 0)
	{
		ControlBarCleanUp();
	}
}
#endif

BOOL AFX_GLOBAL_DATA::DrawParentBackground(CWnd* pWnd, CDC* pDC, LPRECT rectClip)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pWnd);

	BOOL bRes = FALSE;

	CRgn rgn;
	if (rectClip != NULL)
	{
		rgn.CreateRectRgnIndirect(rectClip);
		pDC->SelectClipRgn(&rgn);
	}

	CWnd* pParent = pWnd->GetParent();
	ASSERT_VALID(pParent);

	// In Windows XP, we need to call DrawThemeParentBackground function to implement
	// transparent controls
	if (m_pfDrawThemeBackground != NULL)
	{
		bRes = (*m_pfDrawThemeBackground)(pWnd->GetSafeHwnd(), pDC->GetSafeHdc(), rectClip) == S_OK;
	}

	if (!bRes)
	{
		CPoint pt(0, 0);
		pWnd->MapWindowPoints(pParent, &pt, 1);
		pt = pDC->OffsetWindowOrg(pt.x, pt.y);

		bRes = (BOOL) pParent->SendMessage(WM_ERASEBKGND, (WPARAM)pDC->m_hDC);

		pDC->SetWindowOrg(pt.x, pt.y);
	}

	pDC->SelectClipRgn(NULL);

	return bRes;
}

CFrameWnd* AFXGetParentFrame(const CWnd* pWnd)
{
	if (pWnd->GetSafeHwnd() == NULL)
	{
		return NULL;
	}
	ASSERT_VALID(pWnd);

	const CWnd* pParentWnd = pWnd;

	while (pParentWnd != NULL)
	{
//		if (pParentWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
////		{
//			CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pParentWnd);
//			pParentWnd = pMiniFrame->GetParent();
//		}
//		else
		{
			pParentWnd = pParentWnd->GetParent();
		}

		if (pParentWnd == NULL)
		{
			return NULL;
		}

		if (pParentWnd->IsFrameWnd())
		{
			return(CFrameWnd*)pParentWnd;
		}
	}

	return NULL;
}

COLORREF AFX_GLOBAL_DATA::GetColor(int nColor)
{
	switch(nColor)
	{
		case COLOR_BTNFACE:             return clrBtnFace;
		case COLOR_BTNSHADOW:           return clrBtnShadow;
		case COLOR_3DDKSHADOW:          return clrBtnDkShadow;
		case COLOR_3DLIGHT:             return clrBtnLight;
		case COLOR_BTNHIGHLIGHT:        return clrBtnHilite;
		case COLOR_BTNTEXT:             return clrBtnText;
		case COLOR_GRAYTEXT:            return clrGrayedText;
		case COLOR_WINDOWFRAME:         return clrWindowFrame;

		case COLOR_HIGHLIGHT:           return clrHilite;
		case COLOR_HIGHLIGHTTEXT:       return clrTextHilite;

		case COLOR_WINDOW:              return clrWindow;
		case COLOR_WINDOWTEXT:          return clrWindowText;

		case COLOR_CAPTIONTEXT:         return clrCaptionText;
		case COLOR_MENUTEXT:            return clrMenuText;

		case COLOR_ACTIVECAPTION:       return clrActiveCaption;
		case COLOR_INACTIVECAPTION:     return clrInactiveCaption;

		case COLOR_ACTIVEBORDER:        return clrActiveBorder;
		case COLOR_INACTIVEBORDER:      return clrInactiveBorder;

		case COLOR_INACTIVECAPTIONTEXT: return clrInactiveCaptionText;
	}

	return ::GetSysColor(nColor);
}

BOOL AFX_GLOBAL_DATA::SetLayeredAttrib(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	return(::SetLayeredWindowAttributes(hwnd, crKey, bAlpha, dwFlags));
}

void AFX_GLOBAL_DATA::EnableAccessibilitySupport(BOOL bEnable/* = TRUE*/)
{
	m_bEnableAccessibility = bEnable;
}

CString AFX_GLOBAL_DATA::RegisterWindowClass(LPCTSTR lpszClassNamePrefix)
{
	ENSURE(lpszClassNamePrefix != NULL);

	// Register a new window class:
	HINSTANCE hInst = AfxGetInstanceHandle();
	UINT uiClassStyle = CS_DBLCLKS;
	HCURSOR hCursor = ::LoadCursor(NULL, IDC_ARROW);
	HBRUSH hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

	CString strClassName;
	strClassName.Format(_T("%s:%x:%x:%x:%x"), lpszClassNamePrefix, (UINT_PTR)hInst, uiClassStyle, (UINT_PTR)hCursor, (UINT_PTR)hbrBackground);

	// See if the class already exists:
	WNDCLASS wndcls;
	if (::GetClassInfo(hInst, strClassName, &wndcls))
	{
		// Already registered, assert everything is good:
		ASSERT(wndcls.style == uiClassStyle);
	}
	else
	{
		// Otherwise we need to register a new class:
		wndcls.style = uiClassStyle;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = hCursor;
		wndcls.hbrBackground = hbrBackground;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = strClassName;

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
		}
	}

	return strClassName;
}

BOOL AFX_GLOBAL_DATA::ExcludeTag(CString& strBuffer, LPCTSTR lpszTag, CString& strTag, BOOL bIsCharsList /* = FALSE*/)
{
	const int iBufLen = strBuffer.GetLength();

	CString strTagStart = _T("<");
	strTagStart += lpszTag;
	strTagStart += _T(">");

	const int iTagStartLen = strTagStart.GetLength();

	int iStart = -1;

	int iIndexStart = strBuffer.Find(strTagStart);
	if (iIndexStart < 0)
	{
		return FALSE;
	}

	iStart = iIndexStart + iTagStartLen;

	CString strTagEnd = _T("</");
	strTagEnd += lpszTag;
	strTagEnd += _T('>');

	const int iTagEndLen = strTagEnd.GetLength();

	int iIndexEnd =  -1;
	int nBalanse = 1;
	for (int i = iStart; i < iBufLen - iTagEndLen + 1; i ++)
	{
		if (strBuffer [i] != '<')
		{
			continue;
		}

		if (i < iBufLen - iTagStartLen && _tcsncmp(strBuffer.Mid(i), strTagStart, iTagStartLen) == 0)
		{
			i += iTagStartLen - 1;
			nBalanse ++;
			continue;
		}

		if (_tcsncmp(strBuffer.Mid(i), strTagEnd, iTagEndLen) == 0)
		{
			nBalanse --;
			if (nBalanse == 0)
			{
				iIndexEnd = i;
				break;
			}

			i += iTagEndLen - 1;
		}
	}

	if (iIndexEnd == -1 || iStart > iIndexEnd)
	{
		return FALSE;
	}

	strTag = strBuffer.Mid(iStart, iIndexEnd - iStart);
	strTag.TrimLeft();
	strTag.TrimRight();

	strBuffer.Delete(iIndexStart, iIndexEnd + iTagEndLen - iIndexStart);

	if (bIsCharsList)
	{
		if (strTag.GetLength() > 1 && strTag [0] == _T('\"'))
		{
			strTag = strTag.Mid(1, strTag.GetLength() - 2);
		}

		strTag.Replace(_T("\\t"), _T("\t"));
		strTag.Replace(_T("\\n"), _T("\n"));
		strTag.Replace(_T("\\r"), _T("\r"));
		strTag.Replace(_T("\\b"), _T("\b"));
		strTag.Replace(_T("LT"), _T("<"));
		strTag.Replace(_T("GT"), _T(">"));
		strTag.Replace(_T("AMP"), _T("&"));
	}

	return TRUE;
}

BOOL AFX_GLOBAL_DATA::DwmExtendFrameIntoClientArea(HWND hWnd, AFX_MARGINS* pMargins)
{
	if (m_pfDwmExtendFrameIntoClientArea == NULL)
	{
		return FALSE;
	}

	HRESULT hres = (*m_pfDwmExtendFrameIntoClientArea)(hWnd, pMargins);
	return hres == S_OK;
}

LRESULT AFX_GLOBAL_DATA::DwmDefWindowProc(HWND hWnd, UINT message, WPARAM wp, LPARAM lp)
{
	if (m_pfDwmDefWindowProc == NULL)
	{
		return(LRESULT)-1;
	}

	LRESULT lres = 0;
	(*m_pfDwmDefWindowProc)(hWnd, message, wp, lp, &lres);

	return lres;
}

BOOL AFX_GLOBAL_DATA::DwmIsCompositionEnabled()
{
	if (m_pfDwmIsCompositionEnabled == NULL || bDisableAero)
	{
		return FALSE;
	}

	BOOL bEnabled = FALSE;

	(*m_pfDwmIsCompositionEnabled)(&bEnabled);
	return bEnabled;
}

BOOL AFX_GLOBAL_DATA::DrawTextOnGlass(HTHEME hTheme, CDC* pDC, int iPartId, int iStateId, CString strText, CRect rect, DWORD dwFlags, int nGlowSize, COLORREF clrText)
{
	//---- bits used in dwFlags of DTTOPTS ----
#define AFX_DTT_TEXTCOLOR    (1UL << 0)      // crText has been specified
#define AFX_DTT_BORDERCOLOR  (1UL << 1)      // crBorder has been specified
#define AFX_DTT_SHADOWCOLOR  (1UL << 2)      // crShadow has been specified
#define AFX_DTT_SHADOWTYPE   (1UL << 3)      // iTextShadowType has been specified
#define AFX_DTT_SHADOWOFFSET (1UL << 4)      // ptShadowOffset has been specified
#define AFX_DTT_BORDERSIZE   (1UL << 5)      // nBorderSize has been specified
#define AFX_DTT_FONTPROP     (1UL << 6)      // iFontPropId has been specified
#define AFX_DTT_COLORPROP    (1UL << 7)      // iColorPropId has been specified
#define AFX_DTT_STATEID      (1UL << 8)      // IStateId has been specified
#define AFX_DTT_CALCRECT     (1UL << 9)      // Use pRect as and in/out parameter
#define AFX_DTT_APPLYOVERLAY (1UL << 10)     // fApplyOverlay has been specified
#define AFX_DTT_GLOWSIZE     (1UL << 11)     // iGlowSize has been specified
#define AFX_DTT_CALLBACK     (1UL << 12)     // pfnDrawTextCallback has been specified
#define AFX_DTT_COMPOSITED   (1UL << 13)     // Draws text with antialiased alpha(needs a DIB section)

	if (hTheme == NULL || m_pfDrawThemeTextEx == NULL || !DwmIsCompositionEnabled())
	{
		pDC->DrawText(strText, rect, dwFlags);
		return FALSE;
	}

	CComBSTR bstmp = (LPCTSTR)strText;

	wchar_t* wbuf = new wchar_t[bstmp.Length() + 1];
	wcscpy_s(wbuf, bstmp.Length() + 1, bstmp);

	AFX_DTTOPTS dto;
	memset(&dto, 0, sizeof(AFX_DTTOPTS));
	dto.dwSize = sizeof(AFX_DTTOPTS);
	dto.dwFlags = AFX_DTT_COMPOSITED;

	if (nGlowSize > 0)
	{
		dto.dwFlags |= AFX_DTT_GLOWSIZE;
		dto.iGlowSize = nGlowSize;
	}

	if (clrText != (COLORREF)-1)
	{
		dto.dwFlags |= AFX_DTT_TEXTCOLOR;
		dto.crText = clrText;
	}

	(*m_pfDrawThemeTextEx)(hTheme, pDC->GetSafeHdc(), iPartId, iStateId, wbuf, -1, dwFlags, rect, &dto);

	delete [] wbuf;

	return TRUE;
}

HCURSOR AFX_GLOBAL_DATA::GetHandCursor()
{
	if (m_hcurHand == NULL)
	{
		m_hcurHand = ::LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_HAND));
	}

	return m_hcurHand;
}

BOOL AFX_GLOBAL_DATA::Resume()
{
	m_hinstUXThemeDLL = ::AfxCtxLoadLibraryW(L"UxTheme.dll");

	if (m_hinstUXThemeDLL != NULL)
	{
		m_pfDrawThemeBackground = (DRAWTHEMEPARENTBACKGROUND)::GetProcAddress (m_hinstUXThemeDLL, "DrawThemeParentBackground");
		m_pfDrawThemeTextEx = (DRAWTHEMETEXTEX)::GetProcAddress (m_hinstUXThemeDLL, "DrawThemeTextEx");
		m_pfBeginBufferedPaint = (BEGINBUFFEREDPAINT)::GetProcAddress (m_hinstUXThemeDLL, "BeginBufferedPaint");
		m_pfEndBufferedPaint = (ENDBUFFEREDPAINT)::GetProcAddress (m_hinstUXThemeDLL, "EndBufferedPaint");
	}
	else
	{
		m_pfDrawThemeBackground = NULL;
		m_pfDrawThemeTextEx = NULL;
		m_pfBeginBufferedPaint = NULL;
		m_pfEndBufferedPaint = NULL;
	}

	if (m_hinstDwmapiDLL != NULL)
	{
		m_hinstDwmapiDLL = AfxLoadSystemLibraryUsingFullPath(L"dwmapi.dll");
		ENSURE(m_hinstDwmapiDLL != NULL);

		m_pfDwmExtendFrameIntoClientArea = (DWMEXTENDFRAMEINTOCLIENTAREA)::GetProcAddress (m_hinstDwmapiDLL, "DwmExtendFrameIntoClientArea");
		m_pfDwmDefWindowProc = (DWMDEFWINDOWPROC) ::GetProcAddress (m_hinstDwmapiDLL, "DwmDefWindowProc");
		m_pfDwmIsCompositionEnabled = (DWMISCOMPOSITIONENABLED)::GetProcAddress (m_hinstDwmapiDLL, "DwmIsCompositionEnabled");
	}

	if (m_bEnableAccessibility)
	{
		EnableAccessibilitySupport();
	}

//	CMFCVisualManagerOffice2007::CleanStyle ();

//	if (CMFCVisualManager::m_pRTIDefault != NULL)
//	{
//		CMFCVisualManager::SetDefaultManager (CMFCVisualManager::m_pRTIDefault);
//	}

	return TRUE;
}

BOOL AFX_GLOBAL_DATA::GetNonClientMetrics (NONCLIENTMETRICS& info)
{
	struct AFX_OLDNONCLIENTMETRICS
	{
		UINT    cbSize;
		int     iBorderWidth;
		int     iScrollWidth;
		int     iScrollHeight;
		int     iCaptionWidth;
		int     iCaptionHeight;
		LOGFONT lfCaptionFont;
		int     iSmCaptionWidth;
		int     iSmCaptionHeight;
		LOGFONT lfSmCaptionFont;
		int     iMenuWidth;
		int     iMenuHeight;
		LOGFONT lfMenuFont;
		LOGFONT lfStatusFont;
		LOGFONT lfMessageFont;
	};

	if (_AfxGetComCtlVersion() < MAKELONG(1, 6))
	{
		info.cbSize = sizeof(AFX_OLDNONCLIENTMETRICS);
	}

	return ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, info.cbSize, &info, 0);
}


BOOL AFXAPI AfxIsExtendedFrameClass(CWnd* pWnd)
{
	ENSURE( pWnd );
#if 0
	if (pWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		return TRUE;
	}
	if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		return TRUE;
	}
	if (pWnd->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		return TRUE;
	}
	if (pWnd->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		return TRUE;
	}
	if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		return TRUE;
	}
#endif
	return FALSE;
}


BOOL AFXAPI AfxIsMFCToolBar(CWnd* pWnd)
{
	ENSURE( pWnd );
//	if (pWnd->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
//	{
//		return TRUE;
//	}
	return FALSE;
}

HRESULT AFX_GLOBAL_DATA::ShellCreateItemFromParsingName(PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv)
{
	static HMODULE hShellDll = AfxCtxLoadLibrary(_T("Shell32.dll"));
	ENSURE(hShellDll != NULL);

	typedef	HRESULT (__stdcall *PFNSHCREATEITEMFROMPARSINGNAME)(
		PCWSTR,
		IBindCtx*,
		REFIID,
		void**
		);

	PFNSHCREATEITEMFROMPARSINGNAME pSHCreateItemFromParsingName =
		(PFNSHCREATEITEMFROMPARSINGNAME)GetProcAddress(hShellDll, "SHCreateItemFromParsingName");
	if (pSHCreateItemFromParsingName == NULL)
	{
		return E_FAIL;
	}

	return (*pSHCreateItemFromParsingName)(pszPath, pbc, riid, ppv);
}

BOOL AFX_GLOBAL_DATA::InitD2D(D2D1_FACTORY_TYPE d2dFactoryType, DWRITE_FACTORY_TYPE writeFactoryType)
{
	if (m_bD2DInitialized)
	{
		return TRUE;
	}

	HRESULT hr = S_OK;

	if (!m_bComInitialized)
	{
		hr = CoInitialize(NULL);
		if (FAILED(hr))
		{
			return FALSE;
		}
	}

	m_bComInitialized = TRUE;

	if ((m_hinstD2DDLL = AfxLoadSystemLibraryUsingFullPath(L"D2D1.dll")) == NULL)
	{
		return FALSE;
	}

	typedef HRESULT (WINAPI * D2D1CREATEFACTORY)(D2D1_FACTORY_TYPE factoryType, REFIID riid, CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, void **ppIFactory);
	typedef HRESULT (WINAPI * DWRITECREATEFACTORY)(DWRITE_FACTORY_TYPE factoryType, REFIID riid, IUnknown **factory);

	D2D1CREATEFACTORY pfD2D1CreateFactory = (D2D1CREATEFACTORY)::GetProcAddress(m_hinstD2DDLL, "D2D1CreateFactory");
	if (pfD2D1CreateFactory != NULL)
	{
		hr = (*pfD2D1CreateFactory)(d2dFactoryType, __uuidof(ID2D1Factory),
			NULL, reinterpret_cast<void **>(&m_pDirect2dFactory));
		if (FAILED(hr))
		{
			m_pDirect2dFactory = NULL;
			return FALSE;
		}
	}

	m_pfD2D1MakeRotateMatrix = (D2D1MAKEROTATEMATRIX)::GetProcAddress(m_hinstD2DDLL, "D2D1MakeRotateMatrix");

	m_hinstDWriteDLL = AfxLoadSystemLibraryUsingFullPath(L"DWrite.dll");
	if (m_hinstDWriteDLL != NULL)
	{
		DWRITECREATEFACTORY pfD2D1CreateFactory = (DWRITECREATEFACTORY)::GetProcAddress(m_hinstDWriteDLL, "DWriteCreateFactory");
		if (pfD2D1CreateFactory != NULL)
		{
			hr = (*pfD2D1CreateFactory)(writeFactoryType, __uuidof(IDWriteFactory), (IUnknown**)&m_pWriteFactory);
		}
	}

	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&m_pWicFactory);

	m_bD2DInitialized = TRUE;
	return TRUE;
}

void AFX_GLOBAL_DATA::ReleaseD2DRefs()
{
	if (!m_bD2DInitialized)
	{
		return;
	}

	if (m_pDirect2dFactory != NULL)
	{
		m_pDirect2dFactory->Release();
		m_pDirect2dFactory = NULL;
	}

	if (m_pWriteFactory != NULL)
	{
		m_pWriteFactory->Release();
		m_pWriteFactory = NULL;
	}

	if (m_pWicFactory != NULL)
	{
		m_pWicFactory->Release();
		m_pWicFactory = NULL;
	}

	if (m_hinstD2DDLL != NULL)
	{
		::FreeLibrary(m_hinstD2DDLL);
	}

	if (m_hinstDWriteDLL != NULL)
	{
		::FreeLibrary(m_hinstDWriteDLL);
	}

	m_bD2DInitialized = FALSE;
}
