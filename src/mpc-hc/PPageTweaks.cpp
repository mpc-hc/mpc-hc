/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include "mplayerc.h"
#include "PPageTweaks.h"
#include "MainFrm.h"
#include "../DSUtil/SysVersion.h"


// CPPageTweaks dialog

IMPLEMENT_DYNAMIC(CPPageTweaks, CPPageBase)
CPPageTweaks::CPPageTweaks()
    : CPPageBase(CPPageTweaks::IDD, CPPageTweaks::IDD)
    , m_nJumpDistS(0)
    , m_nJumpDistM(0)
    , m_nJumpDistL(0)
    , m_OSD_Size(0)
    , m_fNotifyMSN(TRUE)
    , m_fPreventMinimize(FALSE)
    , m_fUseWin7TaskBar(TRUE)
    , m_fUseSearchInFolder(FALSE)
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
    DDX_Check(pDX, IDC_CHECK4, m_fNotifyMSN);
    DDX_Check(pDX, IDC_CHECK6, m_fPreventMinimize);
    DDX_Check(pDX, IDC_CHECK_WIN7, m_fUseWin7TaskBar);
    DDX_Check(pDX, IDC_CHECK7, m_fUseSearchInFolder);
    DDX_Check(pDX, IDC_CHECK8, m_fUseTimeTooltip);
    DDX_Control(pDX, IDC_COMBO3, m_TimeTooltipPosition);
    DDX_Control(pDX, IDC_COMBO1, m_FontType);
    DDX_Control(pDX, IDC_COMBO2, m_FontSize);
    DDX_Check(pDX, IDC_CHECK1, m_fFastSeek);
}

int CALLBACK EnumFontProc(ENUMLOGFONT FAR* lf, NEWTEXTMETRIC FAR* tm, int FontType, LPARAM dwData)
{
    CAtlArray<CString>* fntl = (CAtlArray<CString>*)dwData;
    if (FontType == TRUETYPE_FONTTYPE) {
        fntl->Add(lf->elfFullName);
    }
    return true;
}

BOOL CPPageTweaks::OnInitDialog()
{
    __super::OnInitDialog();

    SetHandCursor(m_hWnd, IDC_COMBO1);

    AppSettings& s = AfxGetAppSettings();

    m_nJumpDistS = s.nJumpDistS;
    m_nJumpDistM = s.nJumpDistM;
    m_nJumpDistL = s.nJumpDistL;
    m_fNotifyMSN = s.fNotifyMSN;

    m_fPreventMinimize = s.fPreventMinimize;

    m_fUseWin7TaskBar = s.fUseWin7TaskBar;
    if (!SysVersion::Is7OrLater()) {
        GetDlgItem(IDC_CHECK_WIN7)->EnableWindow(FALSE);
    }

    m_fUseSearchInFolder = s.fUseSearchInFolder;

    m_fUseTimeTooltip = s.fUseTimeTooltip;
    m_TimeTooltipPosition.AddString(ResStr(IDS_TIME_TOOLTIP_ABOVE));
    m_TimeTooltipPosition.AddString(ResStr(IDS_TIME_TOOLTIP_BELOW));
    m_TimeTooltipPosition.SetCurSel(s.nTimeTooltipPosition);
    m_TimeTooltipPosition.EnableWindow(m_fUseTimeTooltip);

    m_OSD_Size = s.nOSDSize;
    m_OSD_Font = s.strOSDFont;

    m_fFastSeek = s.fFastSeek;

    m_FontType.Clear();
    m_FontSize.Clear();
    HDC dc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
    CAtlArray<CString> fntl;
    EnumFontFamilies(dc, NULL, (FONTENUMPROC)EnumFontProc, (LPARAM)&fntl);
    DeleteDC(dc);
    for (size_t i = 0; i < fntl.GetCount(); ++i) {
        if (i > 0 && fntl[i - 1] == fntl[i]) {
            continue;
        }
        m_FontType.AddString(fntl[i]);
    }
    CorrectComboListWidth(m_FontType);
    int iSel = m_FontType.FindStringExact(0, m_OSD_Font);
    if (iSel == CB_ERR) { iSel = 0; }
    m_FontType.SetCurSel(iSel);

    CString str;
    for (int i = 10; i < 26; ++i) {
        str.Format(_T("%d"), i);
        m_FontSize.AddString(str);
        if (m_OSD_Size == i) {
            iSel = i;
        }
    }
    m_FontSize.SetCurSel(iSel - 10);

    EnableToolTips(TRUE);

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageTweaks::OnApply()
{
    UpdateData();

    AppSettings& s = AfxGetAppSettings();

    s.nJumpDistS = m_nJumpDistS;
    s.nJumpDistM = m_nJumpDistM;
    s.nJumpDistL = m_nJumpDistL;
    s.fNotifyMSN = !!m_fNotifyMSN;

    s.fPreventMinimize = !!m_fPreventMinimize;
    s.fUseWin7TaskBar = !!m_fUseWin7TaskBar;
    s.fUseSearchInFolder = !!m_fUseSearchInFolder;
    s.fUseTimeTooltip = !!m_fUseTimeTooltip;
    s.nTimeTooltipPosition = m_TimeTooltipPosition.GetCurSel();
    s.nOSDSize = m_OSD_Size;
    m_FontType.GetLBText(m_FontType.GetCurSel(), s.strOSDFont);

    s.fFastSeek = !!m_fFastSeek;

    CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
    if (m_fUseWin7TaskBar) {
        pFrame->CreateThumbnailToolbar();
    }
    pFrame->UpdateThumbarButton();

    return __super::OnApply();
}

BEGIN_MESSAGE_MAP(CPPageTweaks, CPPageBase)
    ON_UPDATE_COMMAND_UI(IDC_CHECK3, OnUpdateCheck3)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_BN_CLICKED(IDC_CHECK8, OnUseTimeTooltipClicked)
    ON_CBN_SELCHANGE(IDC_COMBO1, OnChngOSDCombo)
    ON_CBN_SELCHANGE(IDC_COMBO2, OnChngOSDCombo)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


// CPPageTweaks message handlers

void CPPageTweaks::OnUpdateCheck3(CCmdUI* pCmdUI)
{
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
    m_OSD_Size = m_FontSize.GetCurSel() + 10;
    m_FontType.GetLBText(m_FontType.GetCurSel(), str);
    ((CMainFrame*)AfxGetMainWnd())->m_OSD.DisplayMessage(OSD_TOPLEFT, _T("Test"), 2000, m_OSD_Size, str);
    SetModified();
}

void CPPageTweaks::OnUseTimeTooltipClicked()
{
    m_TimeTooltipPosition.EnableWindow(IsDlgButtonChecked(IDC_CHECK8));

    SetModified();
}

BOOL CPPageTweaks::OnToolTipNotify(UINT id, NMHDR* pNMH, LRESULT* pResult)
{
    TOOLTIPTEXT* pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMH);
    int cid = ::GetDlgCtrlID((HWND)pNMH->idFrom);
    if (cid == IDC_COMBO1) {
        CDC* pDC = m_FontType.GetDC();
        CFont* pFont = m_FontType.GetFont();
        CFont* pOldFont = pDC->SelectObject(pFont);
        TEXTMETRIC tm;
        pDC->GetTextMetrics(&tm);
        CRect rc;
        m_FontType.GetWindowRect(rc);
        rc.right -= GetSystemMetrics(SM_CXVSCROLL) * GetSystemMetrics(SM_CXEDGE);
        int i = m_FontType.GetCurSel();
        CString str;
        m_FontType.GetLBText(i, str);
        CSize sz;
        sz = pDC->GetTextExtent(str);
        pDC->SelectObject(pOldFont);
        m_FontType.ReleaseDC(pDC);
        sz.cx += tm.tmAveCharWidth;
        str = str.Left(_countof(pTTT->szText));
        if (sz.cx > rc.Width()) {
            _tcscpy_s(pTTT->szText, str);
            pTTT->hinst = NULL;
        }

        return TRUE;
    }

    return FALSE;
}

