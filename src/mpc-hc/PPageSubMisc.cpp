/*
 * (C) 2006-2014 see Authors.txt
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
#include "AuthDlg.h"
#include "PPageSubMisc.h"
#include "SubtitlesProviders.h"

// CPPageSubMisc dialog

IMPLEMENT_DYNAMIC(CPPageSubMisc, CPPageBase)

CPPageSubMisc::CPPageSubMisc()
    : CPPageBase(CPPageSubMisc::IDD, CPPageSubMisc::IDD)
    , m_fPreferDefaultForcedSubtitles(TRUE)
    , m_fPrioritizeExternalSubtitles(TRUE)
    , m_fDisableInternalSubtitles(FALSE)
    , m_bAutoDownloadSubtitles(FALSE)
    , m_strAutoDownloadSubtitlesExclude()
    , m_bAutoUploadSubtitles(FALSE)
    , m_bPreferHearingImpairedSubtitles(FALSE)
    , m_strSubtitlesProviders()
    , m_strSubtitlesLanguageOrder()
    , m_strAutoloadPaths()
    , m_pSubtitlesProviders(SubtitlesProviders::Instance())
{
}

CPPageSubMisc::~CPPageSubMisc()
{
}

void CPPageSubMisc::DoDataExchange(CDataExchange* pDX)
{
    CPPageBase::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK1, m_fPreferDefaultForcedSubtitles);
    DDX_Check(pDX, IDC_CHECK2, m_fPrioritizeExternalSubtitles);
    DDX_Check(pDX, IDC_CHECK3, m_fDisableInternalSubtitles);
    DDX_Check(pDX, IDC_CHECK4, m_bAutoDownloadSubtitles);
    DDX_Check(pDX, IDC_CHECK5, m_bPreferHearingImpairedSubtitles);
    DDX_Check(pDX, IDC_CHECK6, m_bAutoUploadSubtitles);
    DDX_Text(pDX, IDC_EDIT1, m_strAutoloadPaths);
    DDX_Text(pDX, IDC_EDIT2, m_strAutoDownloadSubtitlesExclude);
    DDX_Text(pDX, IDC_EDIT3, m_strSubtitlesLanguageOrder);
    DDX_Control(pDX, IDC_LIST1, m_list);
}

BOOL CPPageSubMisc::OnInitDialog()
{
    __super::OnInitDialog();

    const auto& s = AfxGetAppSettings();

    m_fPreferDefaultForcedSubtitles = s.bPreferDefaultForcedSubtitles;
    m_fPrioritizeExternalSubtitles = s.fPrioritizeExternalSubtitles;
    m_fDisableInternalSubtitles = s.fDisableInternalSubtitles;
    m_strAutoloadPaths = s.strSubtitlePaths;
    m_bAutoDownloadSubtitles = s.bAutoDownloadSubtitles;
    m_strAutoDownloadSubtitlesExclude = s.strAutoDownloadSubtitlesExclude;
    m_bAutoUploadSubtitles = s.bAutoUploadSubtitles;
    m_bPreferHearingImpairedSubtitles = s.bPreferHearingImpairedSubtitles;
    m_strSubtitlesLanguageOrder = s.strSubtitlesLanguageOrder;
    m_strSubtitlesProviders = s.strSubtitlesProviders;

    GetDlgItem(IDC_CHECK5)->EnableWindow(m_bAutoDownloadSubtitles);
    GetDlgItem(IDC_STATIC1)->EnableWindow(m_bAutoDownloadSubtitles);
    GetDlgItem(IDC_EDIT2)->EnableWindow(m_bAutoDownloadSubtitles);

    m_list.SetExtendedStyle(m_list.GetExtendedStyle()
                            | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT
                            | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP);

    m_list.SetImageList(&m_pSubtitlesProviders.GetImageList(), LVSIL_SMALL);

    CArray<int> columnWidth;
    if (columnWidth.GetCount() != COL_TOTAL_COLUMNS) {
        // default sizes
        columnWidth.RemoveAll();
        columnWidth.Add(120);
        columnWidth.Add(75);
        columnWidth.Add(300);
    }

    m_list.InsertColumn(COL_PROVIDER, _T("Provider")/*ResStr(IDS_SUBDL_DLG_PROVIDER_COL)*/, LVCFMT_LEFT, columnWidth[COL_PROVIDER]);
    m_list.InsertColumn(COL_USERNAME, _T("Username")/*ResStr(IDS_SUBDL_DLG_FILENAME_COL)*/, LVCFMT_LEFT, columnWidth[COL_USERNAME]);
    m_list.InsertColumn(COL_LANGUAGES, _T("Languages")/*ResStr(IDS_SUBDL_DLG_LANGUAGE_COL)*/, LVCFMT_LEFT, columnWidth[COL_LANGUAGES]);

    m_list.SetRedraw(FALSE);
    m_list.DeleteAllItems();

    int i = 0;
    for (const auto& iter : m_pSubtitlesProviders.Providers()) {
        int iItem = m_list.InsertItem((int)i++, CString(iter->Name().c_str()), iter->GetIconIndex());
        m_list.SetItemText(iItem, COL_USERNAME, UTF8To16(iter->UserName().c_str()));
        CString languages(UTF8To16(iter->Languages().c_str()));
        m_list.SetItemText(iItem, COL_LANGUAGES, languages.GetLength() ? languages : _T("ERROR: Internet connection could not be established."));
        m_list.SetCheck(iItem, iter->Enabled(SPF_SEARCH));
        m_list.SetItemData(iItem, (DWORD_PTR)(iter));
    }

    m_list.SetRedraw(TRUE);
    m_list.Invalidate();
    m_list.UpdateWindow();

    UpdateData(FALSE);

    return TRUE;
}

BOOL CPPageSubMisc::OnApply()
{
    UpdateData();

    auto& s = AfxGetAppSettings();

    s.bPreferDefaultForcedSubtitles = !!m_fPreferDefaultForcedSubtitles;
    s.fPrioritizeExternalSubtitles = !!m_fPrioritizeExternalSubtitles;
    s.fDisableInternalSubtitles = !!m_fDisableInternalSubtitles;
    s.strSubtitlePaths = m_strAutoloadPaths;
    s.bAutoDownloadSubtitles = !!m_bAutoDownloadSubtitles;
    s.strAutoDownloadSubtitlesExclude = m_strAutoDownloadSubtitlesExclude;
    s.bAutoUploadSubtitles = !!m_bAutoUploadSubtitles;
    s.bPreferHearingImpairedSubtitles = !!m_bPreferHearingImpairedSubtitles;
    s.strSubtitlesLanguageOrder = m_strSubtitlesLanguageOrder;

    for (int i = 0; i < m_list.GetItemCount(); ++i) {
        SubtitlesProvider* provider = (SubtitlesProvider*)(m_list.GetItemData(i));
        provider->Enabled(SPF_SEARCH, m_list.GetCheck(i));
    }

    s.strSubtitlesProviders = CString(m_pSubtitlesProviders.WriteSettings().c_str());

    return __super::OnApply();
}


BEGIN_MESSAGE_MAP(CPPageSubMisc, CPPageBase)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedResetSubsPath)
    ON_BN_CLICKED(IDC_CHECK4, OnAutoDownloadSubtitlesClicked)
    ON_NOTIFY(NM_RCLICK, IDC_LIST1, OnRightClick)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemChanged)
END_MESSAGE_MAP()

void CPPageSubMisc::OnRightClick(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)pNMHDR;

    if (lpnmlv->iItem >= 0 && lpnmlv->iSubItem >= 0) {
        SubtitlesProvider& provider = *(SubtitlesProvider*)(m_list.GetItemData(lpnmlv->iItem));

        enum {
            SET_CREDENTIALS = 0x1000,
            RESET_CREDENTIALS,
            MOVE_UP,
            MOVE_DOWN,
            OPEN_URL,
        };

        CMenu m;
        m.CreatePopupMenu();
        m.AppendMenu(MF_STRING | (provider.Flags(SPF_LOGIN) ? MF_ENABLED : MF_DISABLED), SET_CREDENTIALS, L"Setup"/*ResStr(IDS_DISABLE_ALL_FILTERS)*/);
        m.AppendMenu(MF_STRING | (provider.Flags(SPF_LOGIN) && provider.UserName().length() ? MF_ENABLED : MF_DISABLED), RESET_CREDENTIALS, L"Reset"/*ResStr(IDS_DISABLE_ALL_FILTERS)*/);
        m.AppendMenu(MF_SEPARATOR);
        m.AppendMenu(MF_STRING | (lpnmlv->iItem > 0 ? MF_ENABLED : MF_DISABLED), MOVE_UP, L"Move Up"/*ResStr(IDS_DISABLE_ALL_FILTERS)*/);
        m.AppendMenu(MF_STRING | (lpnmlv->iItem < m_list.GetItemCount() - 1  ? MF_ENABLED : MF_DISABLED), MOVE_DOWN, L"Move Down"/*ResStr(IDS_DISABLE_ALL_FILTERS)*/);
        m.AppendMenu(MF_SEPARATOR);
        m.AppendMenu(MF_STRING | MF_ENABLED, OPEN_URL, L"Open Url" /*ResStr(IDS_ENABLE_ALL_FILTERS)*/);

        CPoint pt = lpnmlv->ptAction;
        ::MapWindowPoints(lpnmlv->hdr.hwndFrom, HWND_DESKTOP, &pt, 1);

        switch (m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, pt.x, pt.y, this)) {
            case OPEN_URL:
                provider.OpenUrl();
                break;
            case SET_CREDENTIALS: {
                CString szUser(UTF8To16(provider.UserName().c_str()));
                CString szPass(UTF8To16(provider.Password().c_str()));
                CString szDomain(provider.Name().c_str());
                if (ERROR_SUCCESS == PromptForCredentials(GetSafeHwnd(),
                                                          ResStr(IDS_CREDENTIALS_SERVER), L"Enter your credentials to connect to: " + CString(provider.Url().c_str()),
                                                          szDomain, szUser, szPass, /*&bSave*/nullptr)) {
                    provider.UserName((const char*)UTF16To8(szUser));
                    provider.Password((const char*)UTF16To8(szPass));
                    m_list.SetItemText(lpnmlv->iItem, 1, szUser);
                    SetModified();
                }
                break;
            }
            case RESET_CREDENTIALS:
                provider.UserName("");
                provider.Password("");
                m_list.SetItemText(lpnmlv->iItem, 1, _T(""));
                SetModified();
                break;
            case MOVE_UP: {
                m_pSubtitlesProviders.MoveUp(lpnmlv->iItem);
                ListView_SortItemsEx(m_list.GetSafeHwnd(), SortCompare, m_list.GetSafeHwnd());
                SetModified();
                break;
            }
            case MOVE_DOWN: {
                m_pSubtitlesProviders.MoveDown(lpnmlv->iItem);
                ListView_SortItemsEx(m_list.GetSafeHwnd(), SortCompare, m_list.GetSafeHwnd());
                SetModified();
                break;
            }
            default:
                break;
        }
    }
}

void CPPageSubMisc::OnAutoDownloadSubtitlesClicked()
{
    m_bAutoDownloadSubtitles = IsDlgButtonChecked(IDC_CHECK4);
    GetDlgItem(IDC_CHECK5)->EnableWindow(m_bAutoDownloadSubtitles);
    GetDlgItem(IDC_STATIC1)->EnableWindow(m_bAutoDownloadSubtitles);
    GetDlgItem(IDC_EDIT2)->EnableWindow(m_bAutoDownloadSubtitles);
    UpdateWindow();

    SetModified();
}

void CPPageSubMisc::OnBnClickedResetSubsPath()
{
    m_strAutoloadPaths = DEFAULT_SUBTITLE_PATHS;

    UpdateData(FALSE);
    SetModified();
}

void CPPageSubMisc::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = (LPNMLISTVIEW)pNMHDR;

    if (pNMLV->uOldState + pNMLV->uNewState == 0x3000) {
        SetModified();
    }
}

int CALLBACK CPPageSubMisc::SortCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CListCtrl& list = *(CListCtrl*)CListCtrl::FromHandle((HWND)(lParamSort));
    size_t left = ((SubtitlesProvider*)list.GetItemData((int)lParam1))->Index();
    size_t right = ((SubtitlesProvider*)list.GetItemData((int)lParam2))->Index();
    return left == right ? 0 : (int)(left - right);
}
