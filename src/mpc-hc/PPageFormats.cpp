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
#include "PPageFormats.h"
#include "FileAssoc.h"
#include "SysVersion.h"
#include <psapi.h>
#include <string>


// CPPageFormats dialog


IMPLEMENT_DYNAMIC(CPPageFormats, CPPageBase)
CPPageFormats::CPPageFormats()
    : CPPageBase(CPPageFormats::IDD, CPPageFormats::IDD)
    , m_list(0)
    , m_exts(_T(""))
    , m_iRtspHandler(0)
    , m_fRtspFileExtFirst(FALSE)
    , m_bInsufficientPrivileges(false)
{
}

CPPageFormats::~CPPageFormats()
{
}

void CPPageFormats::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list);
    DDX_Text(pDX, IDC_EDIT1, m_exts);
    DDX_Control(pDX, IDC_STATIC1, m_autoplay);
    DDX_Control(pDX, IDC_CHECK1, m_apvideo);
    DDX_Control(pDX, IDC_CHECK2, m_apmusic);
    DDX_Control(pDX, IDC_CHECK3, m_apaudiocd);
    DDX_Control(pDX, IDC_CHECK4, m_apdvd);
    DDX_Radio(pDX, IDC_RADIO1, m_iRtspHandler);
    DDX_Check(pDX, IDC_CHECK5, m_fRtspFileExtFirst);
    DDX_Control(pDX, IDC_CHECK6, m_fContextDir);
    DDX_Control(pDX, IDC_CHECK7, m_fContextFiles);
    DDX_Control(pDX, IDC_CHECK8, m_fAssociatedWithIcons);
}

int CPPageFormats::GetChecked(int iItem)
{
    LVITEM lvi;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_IMAGE;
    m_list.GetItem(&lvi);
    return lvi.iImage;
}

void CPPageFormats::SetChecked(int iItem, int iChecked)
{
    LVITEM lvi;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_IMAGE;
    lvi.iImage = iChecked;
    m_list.SetItem(&lvi);
}

CString GetProgramDir()
{
    CString RtnVal;
    TCHAR    FileName[_MAX_PATH];
    ::GetModuleFileName(AfxGetInstanceHandle(), FileName, _MAX_PATH);
    RtnVal = FileName;
    RtnVal = RtnVal.Left(RtnVal.ReverseFind('\\'));
    return RtnVal;
}

int FileExists(const TCHAR* fileName)
{
    DWORD fileAttr;
    fileAttr = ::GetFileAttributes(fileName);
    if (0xFFFFFFFF == fileAttr) {
        return false;
    }
    return true;
}

void CPPageFormats::SetListItemState(int nItem)
{
    if (nItem < 0) {
        return;
    }

    CFileAssoc::reg_state_t state = CFileAssoc::IsRegistered(AfxGetAppSettings().m_Formats[(int)m_list.GetItemData(nItem)]);

    SetChecked(nItem, (state == CFileAssoc::ALL_REGISTERED) ? 1 : (state == CFileAssoc::NOT_REGISTERED) ? 0 : 2);
}

bool CPPageFormats::IsNeededIconsLib()
{
    bool needIconLib = false;
    int i = 0;

    while (!needIconLib && i < m_list.GetItemCount()) {
        if (GetChecked(i) == 1) {
            needIconLib = true;
        }
        i++;
    }

    return needIconLib;
}

BEGIN_MESSAGE_MAP(CPPageFormats, CPPageBase)
    ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickList1)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnLvnItemchangedList1)
    ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LIST1, OnBeginlabeleditList)
    ON_NOTIFY(LVN_DOLABELEDIT, IDC_LIST1, OnDolabeleditList)
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnEndlabeleditList)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton12)
    ON_BN_CLICKED(IDC_BUTTON_EXT_SET, OnBnClickedButton11)
    ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton14)
    ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton13)
    ON_BN_CLICKED(IDC_BUTTON5, OnBnVistaModify)
    ON_BN_CLICKED(IDC_CHECK7, OnFilesAssocModified)
    ON_BN_CLICKED(IDC_CHECK8, OnFilesAssocModified)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateButtonDefault)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON_EXT_SET, OnUpdateButtonSet)
END_MESSAGE_MAP()

// CPPageFormats message handlers

BOOL CPPageFormats::OnInitDialog()
{
    __super::OnInitDialog();

    m_bFileExtChanged = false;

    m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    m_list.InsertColumn(COL_CATEGORY, _T("Category"), LVCFMT_LEFT, 300);
    m_list.InsertColumn(COL_ENGINE, _T("Engine"), LVCFMT_RIGHT, 60);

    m_onoff.Create(IDB_ONOFF, 12, 3, 0xffffff);
    m_list.SetImageList(&m_onoff, LVSIL_SMALL);

    int fSetContextFiles = FALSE;

    CMediaFormats& mf = AfxGetAppSettings().m_Formats;
    mf.UpdateData(FALSE);
    for (int i = 0; i < (int)mf.GetCount(); i++) {
        CString label;
        label.Format(_T("%s (%s)"), mf[i].GetDescription(), mf[i].GetExts());

        int iItem = m_list.InsertItem(i, label);
        m_list.SetItemData(iItem, i);
        engine_t e = mf[i].GetEngineType();
        m_list.SetItemText(iItem, COL_ENGINE,
                           e == DirectShow ? _T("DirectShow") :
                           e == RealMedia ? _T("RealMedia") :
                           e == QuickTime ? _T("QuickTime") :
                           e == ShockWave ? _T("ShockWave") : _T("-"));

        CFileAssoc::reg_state_t state = CFileAssoc::IsRegistered(mf[i]);
        SetChecked(iItem, (state == CFileAssoc::SOME_REGISTERED) ? 2 : (state == CFileAssoc::ALL_REGISTERED));

        if (!fSetContextFiles && CFileAssoc::AreRegisteredFileContextMenuEntries(mf[i]) != CFileAssoc::NOT_REGISTERED) {
            fSetContextFiles = TRUE;
        }
    }

    //  m_list.SetColumnWidth(COL_CATEGORY, LVSCW_AUTOSIZE);
    m_list.SetColumnWidth(COL_ENGINE, LVSCW_AUTOSIZE_USEHEADER);

    m_list.SetSelectionMark(0);
    m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    m_exts = mf[(int)m_list.GetItemData(0)].GetExtsWithPeriod();

    AppSettings& s = AfxGetAppSettings();
    bool fRtspFileExtFirst;
    engine_t e = s.m_Formats.GetRtspHandler(fRtspFileExtFirst);
    m_iRtspHandler = (e == RealMedia ? 0 : e == QuickTime ? 1 : 2);
    m_fRtspFileExtFirst = fRtspFileExtFirst;

    m_fContextFiles.SetCheck(fSetContextFiles);

    m_apvideo.SetCheck(CFileAssoc::IsAutoPlayRegistered(CFileAssoc::AP_VIDEO));
    m_apmusic.SetCheck(CFileAssoc::IsAutoPlayRegistered(CFileAssoc::AP_MUSIC));
    m_apaudiocd.SetCheck(CFileAssoc::IsAutoPlayRegistered(CFileAssoc::AP_AUDIOCD));
    m_apdvd.SetCheck(CFileAssoc::IsAutoPlayRegistered(CFileAssoc::AP_DVDMOVIE));

    CreateToolTip();

    if (SysVersion::IsVistaOrLater() && !IsUserAnAdmin()) {
        GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_BUTTON3)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_BUTTON4)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK3)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK4)->EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK5)->EnableWindow(FALSE);

        GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO3)->EnableWindow(FALSE);

        GetDlgItem(IDC_BUTTON5)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_BUTTON5)->SendMessage(BCM_SETSHIELD, 0, 1);

        m_bInsufficientPrivileges = true;
    } else {
        GetDlgItem(IDC_BUTTON5)->ShowWindow(SW_HIDE);
    }

    m_fContextDir.SetCheck(CFileAssoc::AreRegisteredFolderContextMenuEntries());
    m_fAssociatedWithIcons.SetCheck(s.fAssociatedWithIcons);

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageFormats::OnApply()
{
    UpdateData();

    {
        int i = m_list.GetSelectionMark();
        if (i >= 0) {
            i = (int)m_list.GetItemData(i);
        }
        if (i >= 0) {
            CMediaFormats& mf = AfxGetAppSettings().m_Formats;
            mf[i].SetExts(m_exts);
            m_exts = mf[i].GetExtsWithPeriod();
            UpdateData(FALSE);
        }
    }

    CMediaFormats& mf = AfxGetAppSettings().m_Formats;

    CFileAssoc::RegisterApp();

    int fSetContextFiles = m_fContextFiles.GetCheck();
    int fSetAssociatedWithIcon = m_fAssociatedWithIcons.GetCheck();

    if (m_bFileExtChanged) {
        if (fSetAssociatedWithIcon && IsNeededIconsLib() && !FileExists(GetProgramDir() + _T("\\mpciconlib.dll"))) {
            AfxMessageBox(IDS_MISSING_ICONS_LIB, MB_ICONEXCLAMATION | MB_OK, 0);
        }

        for (int i = 0; i < m_list.GetItemCount(); i++) {
            int iChecked = GetChecked(i);
            if (iChecked == 2) {
                continue;
            }

            CFileAssoc::Register(mf[(int)m_list.GetItemData(i)], !!iChecked, !!fSetContextFiles, !!fSetAssociatedWithIcon);
        }

        m_bFileExtChanged = false;
    }

    CFileAssoc::RegisterFolderContextMenuEntries(!!m_fContextDir.GetCheck());

    SetListItemState(m_list.GetSelectionMark());

    CFileAssoc::RegisterAutoPlay(CFileAssoc::AP_VIDEO, !!m_apvideo.GetCheck());
    CFileAssoc::RegisterAutoPlay(CFileAssoc::AP_MUSIC, !!m_apmusic.GetCheck());
    CFileAssoc::RegisterAutoPlay(CFileAssoc::AP_AUDIOCD, !!m_apaudiocd.GetCheck());
    CFileAssoc::RegisterAutoPlay(CFileAssoc::AP_DVDMOVIE, !!m_apdvd.GetCheck());

    AppSettings& s = AfxGetAppSettings();
    s.m_Formats.SetRtspHandler(m_iRtspHandler == 0 ? RealMedia : m_iRtspHandler == 1 ? QuickTime : DirectShow, !!m_fRtspFileExtFirst);
    s.fAssociatedWithIcons = !!m_fAssociatedWithIcons.GetCheck();

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return __super::OnApply();
}

void CPPageFormats::OnNMClickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

    if (lpnmlv->iItem >= 0 && lpnmlv->iSubItem == COL_CATEGORY) {
        CRect r;
        m_list.GetItemRect(lpnmlv->iItem, r, LVIR_ICON);
        if (r.PtInRect(lpnmlv->ptAction)) {
            if (m_bInsufficientPrivileges) {
                MessageBox(ResStr(IDS_CANNOT_CHANGE_FORMAT));
            } else {
                SetChecked(lpnmlv->iItem, (GetChecked(lpnmlv->iItem) & 1) == 0 ? 1 : 0);
                m_bFileExtChanged = true;
                SetModified();
            }
        }
    }

    *pResult = 0;
}

void CPPageFormats::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    if (pNMLV->iItem >= 0 && pNMLV->iSubItem == COL_CATEGORY
            && (pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVIS_SELECTED)) {
        m_exts = AfxGetAppSettings().m_Formats[(int)m_list.GetItemData(pNMLV->iItem)].GetExtsWithPeriod();
        UpdateData(FALSE);
    }

    *pResult = 0;
}

void CPPageFormats::OnBeginlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM* pItem = &pDispInfo->item;

    *pResult = FALSE;

    if (pItem->iItem < 0) {
        return;
    }

    if (pItem->iSubItem == COL_ENGINE) {
        *pResult = TRUE;
    }
}

void CPPageFormats::OnDolabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM* pItem = &pDispInfo->item;

    *pResult = FALSE;

    if (pItem->iItem < 0) {
        return;
    }

    CMediaFormatCategory& mfc = AfxGetAppSettings().m_Formats[m_list.GetItemData(pItem->iItem)];

    CAtlList<CString> sl;

    if (pItem->iSubItem == COL_ENGINE) {
        sl.AddTail(_T("DirectShow"));
        sl.AddTail(_T("RealMedia"));
        sl.AddTail(_T("QuickTime"));
        sl.AddTail(_T("ShockWave"));

        int nSel = (int)mfc.GetEngineType();

        m_list.ShowInPlaceComboBox(pItem->iItem, pItem->iSubItem, sl, nSel);

        *pResult = TRUE;
    }
}

void CPPageFormats::OnEndlabeleditList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM* pItem = &pDispInfo->item;

    *pResult = FALSE;

    if (!m_list.m_fInPlaceDirty) {
        return;
    }

    if (pItem->iItem < 0) {
        return;
    }

    CMediaFormatCategory& mfc = AfxGetAppSettings().m_Formats[m_list.GetItemData(pItem->iItem)];

    if (pItem->iSubItem == COL_ENGINE && pItem->lParam >= 0) {
        mfc.SetEngineType((engine_t)pItem->lParam);
        m_list.SetItemText(pItem->iItem, pItem->iSubItem, pItem->pszText);
        *pResult = TRUE;
    }

    if (*pResult) {
        SetModified();
    }
}

void CPPageFormats::OnBnClickedButton1()
{
    for (int i = 0, j = m_list.GetItemCount(); i < j; i++) {
        SetChecked(i, 1);
    }
    m_bFileExtChanged = true;

    m_apvideo.SetCheck(1);
    m_apmusic.SetCheck(1);
    m_apaudiocd.SetCheck(1);
    m_apdvd.SetCheck(1);

    SetModified();
}

void CPPageFormats::OnBnClickedButton14()
{
    CMediaFormats& mf = AfxGetAppSettings().m_Formats;

    for (int i = 0, j = m_list.GetItemCount(); i < j; i++) {
        if (!mf[m_list.GetItemData(i)].GetLabel().CompareNoCase(_T("pls"))) {
            SetChecked(i, 0);
            continue;
        }
        SetChecked(i, mf[(int)m_list.GetItemData(i)].IsAudioOnly() ? 0 : 1);
    }
    m_bFileExtChanged = true;

    m_apvideo.SetCheck(1);
    m_apmusic.SetCheck(0);
    m_apaudiocd.SetCheck(0);
    m_apdvd.SetCheck(1);

    SetModified();
}

void CPPageFormats::OnBnClickedButton13()
{
    CMediaFormats& mf = AfxGetAppSettings().m_Formats;

    for (int i = 0, j = m_list.GetItemCount(); i < j; i++) {
        SetChecked(i, mf[(int)m_list.GetItemData(i)].IsAudioOnly() ? 1 : 0);
    }
    m_bFileExtChanged = true;

    m_apvideo.SetCheck(0);
    m_apmusic.SetCheck(1);
    m_apaudiocd.SetCheck(1);
    m_apdvd.SetCheck(0);

    SetModified();
}

void CPPageFormats::OnBnVistaModify()
{
    CString strCmd;
    TCHAR   strApp[_MAX_PATH];

    strCmd.Format(_T("/adminoption %d"), IDD);
    GetModuleFileNameEx(GetCurrentProcess(), AfxGetMyApp()->m_hInstance, strApp, _MAX_PATH);

    AfxGetMyApp()->RunAsAdministrator(strApp, strCmd, true);

    for (int i = 0; i < m_list.GetItemCount(); i++) {
        SetListItemState(i);
    }
}

void CPPageFormats::OnBnClickedButton12()
{
    int i = m_list.GetSelectionMark();
    if (i < 0) {
        return;
    }
    i = (int)m_list.GetItemData(i);
    CMediaFormats& mf = AfxGetAppSettings().m_Formats;
    mf[i].RestoreDefaultExts();
    m_exts = mf[i].GetExtsWithPeriod();
    SetListItemState(m_list.GetSelectionMark());
    UpdateData(FALSE);

    SetModified();
}

void CPPageFormats::OnBnClickedButton11()
{
    UpdateData();
    int i = m_list.GetSelectionMark();
    if (i < 0) {
        return;
    }
    i = (int)m_list.GetItemData(i);
    CMediaFormats& mf = AfxGetAppSettings().m_Formats;
    mf[i].SetExts(m_exts);
    m_exts = mf[i].GetExtsWithPeriod();
    SetListItemState(m_list.GetSelectionMark());
    UpdateData(FALSE);

    SetModified();
}

void CPPageFormats::OnFilesAssocModified()
{
    m_bFileExtChanged = true;
    SetModified();
}

void CPPageFormats::OnUpdateButtonDefault(CCmdUI* pCmdUI)
{
    int i = m_list.GetSelectionMark();
    if (i < 0) {
        pCmdUI->Enable(FALSE);
        return;
    }
    i = (int)m_list.GetItemData(i);

    CString orgexts, newexts;
    GetDlgItem(IDC_EDIT1)->GetWindowText(newexts);
    newexts.Trim();
    orgexts = AfxGetAppSettings().m_Formats[i].GetBackupExtsWithPeriod();

    pCmdUI->Enable(!!newexts.CompareNoCase(orgexts));
}

void CPPageFormats::OnUpdateButtonSet(CCmdUI* pCmdUI)
{
    int i = m_list.GetSelectionMark();
    if (i < 0) {
        pCmdUI->Enable(FALSE);
        return;
    }
    i = (int)m_list.GetItemData(i);

    CString orgexts, newexts;
    GetDlgItem(IDC_EDIT1)->GetWindowText(newexts);
    newexts.Trim();
    orgexts = AfxGetAppSettings().m_Formats[i].GetExtsWithPeriod();

    pCmdUI->Enable(!!newexts.CompareNoCase(orgexts));
}
