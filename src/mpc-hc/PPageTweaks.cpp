/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015, 2017 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <VersionHelpersInternal.h>
#include "mplayerc.h"
#include "PPageTweaks.h"
#include "MainFrm.h"


// CPPageTweaks dialog

IMPLEMENT_DYNAMIC(CPPageTweaks, CMPCThemePPageBase)
CPPageTweaks::CPPageTweaks()
    : CMPCThemePPageBase(CPPageTweaks::IDD, CPPageTweaks::IDD)
    , m_nJumpDistS(0)
    , m_nJumpDistM(0)
    , m_nJumpDistL(0)
    , m_fNotifySkype(TRUE)
    , m_fPreventMinimize(FALSE)
    , m_bUseEnhancedTaskBar(TRUE)
    , m_fUseSearchInFolder(FALSE)
    , m_fUseTimeTooltip(TRUE)
    , m_bHideWindowedMousePointer(TRUE)
    , m_nOSDSize(0)
    , m_fFastSeek(FALSE)
    , m_fShowChapters(TRUE)
    , m_fLCDSupport(FALSE)
{
}

CPPageTweaks::~CPPageTweaks()
{
}

void CPPageTweaks::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT1, m_nJumpDistS);
    DDX_Text(pDX, IDC_EDIT2, m_nJumpDistM);
    DDX_Text(pDX, IDC_EDIT3, m_nJumpDistL);
    DDX_Check(pDX, IDC_CHECK4, m_fNotifySkype);
    DDX_Check(pDX, IDC_CHECK6, m_fPreventMinimize);
    DDX_Check(pDX, IDC_CHECK_ENHANCED_TASKBAR, m_bUseEnhancedTaskBar);
    DDX_Check(pDX, IDC_CHECK7, m_fUseSearchInFolder);
    DDX_Check(pDX, IDC_CHECK8, m_fUseTimeTooltip);
    DDX_Control(pDX, IDC_COMBO3, m_TimeTooltipPosition);
    DDX_Control(pDX, IDC_COMBO1, m_FontType);
    DDX_Control(pDX, IDC_COMBO2, m_FontSize);
    DDX_Control(pDX, IDC_COMBO4, m_FastSeekMethod);
    DDX_Check(pDX, IDC_FASTSEEK_CHECK, m_fFastSeek);
    DDX_Check(pDX, IDC_CHECK2, m_fShowChapters);
    DDX_Check(pDX, IDC_CHECK_LCD, m_fLCDSupport);
    DDX_Check(pDX, IDC_CHECK3, m_bHideWindowedMousePointer);
}

int CALLBACK EnumFontProc(ENUMLOGFONT FAR* lf, NEWTEXTMETRIC FAR* tm, int FontType, LPARAM dwData)
{
    CAtlArray<CString>* fntl = (CAtlArray<CString>*)dwData;
    if (FontType == TRUETYPE_FONTTYPE) {
        fntl->Add(lf->elfFullName);
    }
    return 1; /* Continue the enumeration */
}

BOOL CPPageTweaks::OnInitDialog()
{
    __super::OnInitDialog();

    SetHandCursor(m_hWnd, IDC_COMBO1);

    const CAppSettings& s = AfxGetAppSettings();

    m_nJumpDistS = s.nJumpDistS;
    m_nJumpDistM = s.nJumpDistM;
    m_nJumpDistL = s.nJumpDistL;
    m_fNotifySkype = s.bNotifySkype;

    m_fPreventMinimize = s.fPreventMinimize;

    m_bUseEnhancedTaskBar = s.bUseEnhancedTaskBar;
    if (!IsWindows7OrGreater()) {
        GetDlgItem(IDC_CHECK_ENHANCED_TASKBAR)->EnableWindow(FALSE);
    }

    m_fUseSearchInFolder = s.fUseSearchInFolder;

    m_fUseTimeTooltip = s.fUseTimeTooltip;
    m_TimeTooltipPosition.AddString(ResStr(IDS_TIME_TOOLTIP_ABOVE));
    m_TimeTooltipPosition.AddString(ResStr(IDS_TIME_TOOLTIP_BELOW));
    m_TimeTooltipPosition.SetCurSel(s.nTimeTooltipPosition);
    m_TimeTooltipPosition.EnableWindow(m_fUseTimeTooltip);

    m_nOSDSize = s.nOSDSize;
    m_strOSDFont = s.strOSDFont;

    m_fFastSeek = s.bFastSeek;
    m_FastSeekMethod.AddString(ResStr(IDS_FASTSEEK_LATEST));
    m_FastSeekMethod.AddString(ResStr(IDS_FASTSEEK_NEAREST));
    m_FastSeekMethod.SetCurSel(s.eFastSeekMethod);

    m_fShowChapters = s.fShowChapters;

    m_bHideWindowedMousePointer = s.bHideWindowedMousePointer;

    m_fLCDSupport = s.fLCDSupport;

    m_FontType.Clear();
    m_FontSize.Clear();
    HDC dc = CreateDC(_T("DISPLAY"), nullptr, nullptr, nullptr);
    CAtlArray<CString> fntl;
    EnumFontFamilies(dc, nullptr, (FONTENUMPROC)EnumFontProc, (LPARAM)&fntl);
    DeleteDC(dc);
    for (size_t i = 0; i < fntl.GetCount(); ++i) {
        if (i > 0 && fntl[i - 1] == fntl[i]) {
            continue;
        }
        m_FontType.AddString(fntl[i]);
    }
    CorrectComboListWidth(m_FontType);
    int iSel = m_FontType.FindStringExact(0, m_strOSDFont);
    if (iSel == CB_ERR) {
        iSel = m_FontType.FindString(0, m_strOSDFont);
        if (iSel == CB_ERR) {
            iSel = 0;
        }
    }
    m_FontType.SetCurSel(iSel);

    CString str;
    for (int i = 10; i < 26; ++i) {
        str.Format(_T("%d"), i);
        m_FontSize.AddString(str);
        if (m_nOSDSize == i) {
            iSel = i;
        }
    }
    m_FontSize.SetCurSel(iSel - 10);

    CreateToolTip();
    EnableToolTips(TRUE);

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageTweaks::OnApply()
{
    UpdateData();

    CAppSettings& s = AfxGetAppSettings();

    s.nJumpDistS = m_nJumpDistS;
    s.nJumpDistM = m_nJumpDistM;
    s.nJumpDistL = m_nJumpDistL;
    s.bNotifySkype = !!m_fNotifySkype;

    s.fPreventMinimize = !!m_fPreventMinimize;
    s.bUseEnhancedTaskBar = !!m_bUseEnhancedTaskBar;
    s.fUseSearchInFolder = !!m_fUseSearchInFolder;
    s.fUseTimeTooltip = !!m_fUseTimeTooltip;
    s.nTimeTooltipPosition = m_TimeTooltipPosition.GetCurSel();
    s.nOSDSize = m_nOSDSize;
    m_FontType.GetLBText(m_FontType.GetCurSel(), s.strOSDFont);

    s.bFastSeek = !!m_fFastSeek;
    s.eFastSeekMethod = static_cast<decltype(s.eFastSeekMethod)>(m_FastSeekMethod.GetCurSel());

    s.bHideWindowedMousePointer = !!m_bHideWindowedMousePointer;

    s.fShowChapters = !!m_fShowChapters;

    s.fLCDSupport = !!m_fLCDSupport;

    CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
    if (m_bUseEnhancedTaskBar) {
        pFrame->CreateThumbnailToolbar();
    }
    pFrame->UpdateThumbarButton();

    // There is no main frame when the option dialog is displayed stand-alone
    if (CMainFrame* pMainFrame = AfxGetMainFrame()) {
        pMainFrame->UpdateControlState(CMainFrame::UPDATE_SKYPE);
        pMainFrame->UpdateControlState(CMainFrame::UPDATE_SEEKBAR_CHAPTERS);
    }

    return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageTweaks, CMPCThemePPageBase)
    ON_UPDATE_COMMAND_UI(IDC_COMBO4, OnUpdateFastSeek)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_BN_CLICKED(IDC_CHECK8, OnUseTimeTooltipClicked)
    ON_CBN_SELCHANGE(IDC_COMBO1, OnChngOSDCombo)
    ON_CBN_SELCHANGE(IDC_COMBO2, OnChngOSDCombo)
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
END_MESSAGE_MAP()


// CPPageTweaks message handlers

void CPPageTweaks::OnUpdateFastSeek(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsDlgButtonChecked(IDC_FASTSEEK_CHECK));
}

void CPPageTweaks::OnBnClickedButton1()
{
    m_nJumpDistS = DEFAULT_JUMPDISTANCE_1;
    m_nJumpDistM = DEFAULT_JUMPDISTANCE_2;
    m_nJumpDistL = DEFAULT_JUMPDISTANCE_3;

    UpdateData(FALSE);
    SetModified();
}

void CPPageTweaks::OnChngOSDCombo()
{
    CString str;
    m_nOSDSize = m_FontSize.GetCurSel() + 10;
    m_FontType.GetLBText(m_FontType.GetCurSel(), str);
    if (CMainFrame* pMainFrame = AfxGetMainFrame()) {
        pMainFrame->m_OSD.DisplayMessage(OSD_TOPLEFT, _T("Test"), 2000, m_nOSDSize, str);
    }
    SetModified();
}

void CPPageTweaks::OnUseTimeTooltipClicked()
{
    m_TimeTooltipPosition.EnableWindow(IsDlgButtonChecked(IDC_CHECK8));

    SetModified();
}

BOOL CPPageTweaks::OnToolTipNotify(UINT id, NMHDR* pNMH, LRESULT* pResult)
{
    LPTOOLTIPTEXT pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMH);

    UINT_PTR nID = pNMH->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND) {
        nID = ::GetDlgCtrlID((HWND)nID);
    }

    BOOL bRet = FALSE;

    switch (nID) {
        case IDC_COMBO1:
            bRet = FillComboToolTip(m_FontType, pTTT);
            break;
        case IDC_COMBO3:
            bRet = FillComboToolTip(m_TimeTooltipPosition, pTTT);
            break;
        case IDC_COMBO4:
            bRet = FillComboToolTip(m_FastSeekMethod, pTTT);
            break;
    }

    return bRet;
}
