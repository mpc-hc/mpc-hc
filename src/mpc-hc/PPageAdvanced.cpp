/*
* (C) 2014-2017 see Authors.txt
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
#include "PPageAdvanced.h"
#include "mplayerc.h"
#include "MainFrm.h"
#include "EventDispatcher.h"
#include <strsafe.h>

CPPageAdvanced::CPPageAdvanced()
    : CMPCThemePPageBase(IDD, IDD)
{
    EventRouter::EventSelection fires;
    fires.insert(MpcEvent::DEFAULT_TOOLBAR_SIZE_CHANGED);
    GetEventd().Connect(m_eventc, fires);
}

void CPPageAdvanced::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list);
    DDX_Control(pDX, IDC_COMBO1, m_comboBox);
    DDX_Control(pDX, IDC_SPIN1, m_spinButtonCtrl);
}

BOOL CPPageAdvanced::OnInitDialog()
{
    __super::OnInitDialog();

    if (CFont* pFont = m_list.GetFont()) {
        LOGFONT logfont;
        pFont->GetLogFont(&logfont);
        logfont.lfWeight = FW_BOLD;
        m_fontBold.CreateFontIndirect(&logfont);
    }

    SetRedraw(FALSE);
    m_list.SetExtendedStyle(m_list.GetExtendedStyle() /* | LVS_EX_FULLROWSELECT */ | LVS_EX_AUTOSIZECOLUMNS /*| LVS_EX_DOUBLEBUFFER */| LVS_EX_INFOTIP);
    m_list.setAdditionalStyles(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
    m_list.InsertColumn(COL_NAME, ResStr(IDS_PPAGEADVANCED_COL_NAME), LVCFMT_LEFT);
    m_list.InsertColumn(COL_VALUE, ResStr(IDS_PPAGEADVANCED_COL_VALUE), LVCFMT_RIGHT);

    if (auto pToolTip = m_list.GetToolTips()) {
        // Set topmost for tooltip window. Workaround bug https://connect.microsoft.com/VisualStudio/feedback/details/272350
        pToolTip->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOREDRAW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOOWNERZORDER);
    }

    GetDlgItem(IDC_EDIT1)->GetWindowRect(editRect);
    ScreenToClient(editRect);

    m_spinButtonCtrl.SetBuddy(GetDlgItem(IDC_EDIT1));

    GetDlgItem(IDC_EDIT1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_COMBO1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_RADIO1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_RADIO2)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_SPIN1)->ShowWindow(SW_HIDE);

    GetDlgItemText(IDC_RADIO1, m_strTrue);
    GetDlgItemText(IDC_RADIO2, m_strFalse);

    InitSettings();

    m_list.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
    m_list.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);

    SetRedraw(TRUE);
    return TRUE;
}

void CPPageAdvanced::InitSettings()
{
    auto& s = AfxGetAppSettings();

    auto addBoolItem = [this](int nItem, CString name, bool defaultValue, bool & settingReference, CString toolTipText) {
        auto pItem = std::make_shared<SettingsBool>(name, defaultValue, settingReference, toolTipText);
        auto eSetting = static_cast<ADVANCED_SETTINGS>(nItem);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        m_hiddenOptions[eSetting] = pItem;
        m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, (pItem->GetValue() ? m_strTrue : m_strFalse), !IsDefault(eSetting));
        m_list.SetItemData(iItem, nItem);
    };

    // The range parameter defines range (inclusive) that particular option can have.
    auto addIntItem = [this](int nItem, CString name, int defaultValue, int& settingReference, std::pair<int, int> range, CString toolTipText) {
        ASSERT(range.first <= defaultValue && defaultValue <= range.second); // Default value not in range?
        if (range.first > settingReference || settingReference > range.second) {
            // In case settings were corrupted, reset to default.
            ASSERT(FALSE);
            settingReference = defaultValue;
        }
        auto pItem = std::make_shared<SettingsInt>(name, defaultValue, settingReference, range, toolTipText);
        auto eSetting = static_cast<ADVANCED_SETTINGS>(nItem);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        m_hiddenOptions[eSetting] = pItem;

        CString str;
        str.Format(_T("%d"), pItem->GetValue());
        m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, str, !IsDefault(eSetting));
        m_list.SetItemData(iItem, nItem);
    };

    // The list parameter defines list of the strings that will be in the combobox.
    auto addComboItem = [this](int nItem, CString name, int defaultValue, int& settingReference, std::deque<CString> list, CString toolTipText) {
        auto pItem = std::make_shared<SettingsCombo>(name, defaultValue, settingReference, list, toolTipText);
        auto eSetting = static_cast<ADVANCED_SETTINGS>(nItem);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        m_hiddenOptions[eSetting] = pItem;
        m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, list.at(settingReference), !IsDefault(eSetting));
        m_list.SetItemData(iItem, nItem);
    };

    auto addCStringItem = [this](int nItem, CString name, CString defaultValue, CString & settingReference, CString toolTipText) {
        auto pItem = std::make_shared<SettingsCString>(name, defaultValue, settingReference, toolTipText);
        auto eSetting = static_cast<ADVANCED_SETTINGS>(nItem);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        m_hiddenOptions[eSetting] = pItem;
        m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, pItem->GetValue(), !IsDefault(eSetting));
        m_list.SetItemData(iItem, nItem);
    };

    addBoolItem(HIDE_WINDOWED, IDS_RS_HIDE_WINDOWED_CONTROLS, false, s.bHideWindowedControls, StrRes(IDS_PPAGEADVANCED_HIDE_WINDOWED));
    addBoolItem(BLOCK_VSFILTER, IDS_RS_BLOCKVSFILTER, true, s.fBlockVSFilter, StrRes(IDS_PPAGEADVANCED_BLOCK_VSFILTER));
    addIntItem(RECENT_FILES_NB, IDS_RS_RECENT_FILES_NUMBER, 20, s.iRecentFilesNumber, std::make_pair(0, 1000), StrRes(IDS_PPAGEADVANCED_RECENT_FILES_NUMBER));
    addIntItem(FILE_POS_LONGER, IDS_RS_FILEPOSLONGER, 0, s.iRememberPosForLongerThan, std::make_pair(0, INT_MAX), StrRes(IDS_PPAGEADVANCED_FILE_POS_LONGER));
    addBoolItem(FILE_POS_AUDIO, IDS_RS_FILEPOSAUDIO, true, s.bRememberPosForAudioFiles, StrRes(IDS_PPAGEADVANCED_FILE_POS_AUDIO));
    addIntItem(COVER_SIZE_LIMIT, IDS_RS_COVER_ART_SIZE_LIMIT, 600, s.nCoverArtSizeLimit, std::make_pair(0, INT_MAX), StrRes(IDS_PPAGEADVANCED_COVER_SIZE_LIMIT));
    addBoolItem(LOGGING, IDS_RS_LOGGING, false, s.bEnableLogging, StrRes(IDS_PPAGEADVANCED_LOGGER));
    addIntItem(AUTO_DOWNLOAD_SCORE_MOVIES, IDS_RS_AUTODOWNLOADSCOREMOVIES, 0x16, s.nAutoDownloadScoreMovies,
               std::make_pair(10, 30), StrRes(IDS_PPAGEADVANCED_SCORE));
    addIntItem(AUTO_DOWNLOAD_SCORE_SERIES, IDS_RS_AUTODOWNLOADSCORESERIES, 0x18, s.nAutoDownloadScoreSeries,
               std::make_pair(10, 30), StrRes(IDS_PPAGEADVANCED_SCORE));
    addIntItem(DEFAULT_TOOLBAR_SIZE, IDS_RS_DEFAULTTOOLBARSIZE, 24, s.nDefaultToolbarSize,
               std::make_pair(16, 128), StrRes(IDS_PPAGEADVANCED_DEFAULTTOOLBARSIZE));
    addBoolItem(USE_LEGACY_TOOLBAR, IDS_RS_USE_LEGACY_TOOLBAR, false, s.bUseLegacyToolbar, StrRes(IDS_PPAGEADVANCED_USE_LEGACY_TOOLBAR));
    addBoolItem(USE_YDL, IDS_RS_USE_YDL, true, s.bUseYDL, _T("Process HTTP(s) URLs with Youtube-DL (if available). There is an internal whitelist/blacklist for URLs that are always/never processed."));
    addIntItem(YDL_MAX_HEIGHT, IDS_RS_YDL_MAX_HEIGHT, 1440, s.iYDLMaxHeight, std::make_pair(0, INT_MAX), _T("Can be used to limit the video resolution that is chosen by Youtube-DL by the given height. Value 0 means it will choose the highest resolution available."));
    addIntItem(YDL_VIDEO_FORMAT, IDS_RS_YDL_VIDEO_FORMAT, 0, s.iYDLVideoFormat, std::make_pair(0, 8), _T("Preferred video format for stream that is selected from Youtube-DL results.\n0: Automatic\n1: H.264 30fps\n2: H.264 60fps\n3: VP9 30fps\n4: VP9 60fps\n5: VP9 Profile2 30fps\n6: VP9 Profile2 60fps\n7: AV1 30fps\n8: AV1 60fps"));
    addBoolItem(YDL_AUDIO_ONLY, IDS_RS_YDL_AUDIO_ONLY, false, s.bYDLAudioOnly, _T("Instructs Youtube-DL to prefer an audio-only stream (if available)"));
    addCStringItem(YDL_COMMAND_LINE, IDS_RS_YDL_COMMAND_LINE, _T(""), s.sYDLCommandLine, _T("Command line parameters for downloading (\"save a copy\") with youtube-dl.exe\nExamples:\n-f best[height<=1080]\n-f bestvideo+bestaudio\n-o \"C:\\downloads\\%(title)s.%(ext)s\""));
    addBoolItem(SAVEIMAGE_POSITION, IDS_RS_SAVEIMAGE_POSITION, true, s.bSaveImagePosition, _T("Include video position in filename"));
    addBoolItem(SAVEIMAGE_CURRENTTIME, IDS_RS_SAVEIMAGE_CURRENTTIME, false, s.bSaveImageCurrentTime, _T("Include current time in filename"));
    addBoolItem(INACCURATE_FASTSEEK, IDS_RS_ALLOW_INACCURATE_FASTSEEK, true, s.bAllowInaccurateFastseek, _T("When enabled, fast seek (to keyframe) has a maximum inaccuracy of 20 seconds. When disabled, a smaller inaccuracy is allowed when deciding between a fast and a normal seek. For example 30% of jump size."));
    addBoolItem(LOOP_FOLDER_NEXT_FILE, IDS_RS_LOOP_FOLDER_NEXT_FILE, false, s.bLoopFolderOnPlayNextFile, _T("Loop back to first file in folder after playing the last file."));
    addBoolItem(MPCTHEME_MODERNSEEKBAR, IDS_RS_MODERNSEEKBAR, true, s.bModernSeekbar, _T("Requires Dark Theme. When enabled, the time seekbar and volume indicator don't show a dragger at current position, but instead are filled from left to right."));
    addIntItem(MODERNSEEKBAR_HEIGHT, IDS_RS_MODERNSEEKBARHEIGHT, DEF_MODERN_SEEKBAR_HEIGHT, s.iModernSeekbarHeight, std::make_pair(MIN_MODERN_SEEKBAR_HEIGHT, MAX_MODERN_SEEKBAR_HEIGHT), _T("Modern seekbar height (at 96dpi, otherwise scaled)"));
}   

BOOL CPPageAdvanced::OnApply()
{
    auto& s = AfxGetAppSettings();

    int nOldDefaultToolbarSize = s.nDefaultToolbarSize;

    for (int i = 0; i < m_list.GetItemCount(); i++) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(i));
        m_hiddenOptions.at(eSetting)->Apply();
    }

    s.MRU.SetSize(s.iRecentFilesNumber);
    s.MRUDub.SetSize(s.iRecentFilesNumber);
    s.filePositions.SetMaxSize(s.iRecentFilesNumber);
    s.dvdPositions.SetMaxSize(s.iRecentFilesNumber);

    // There is no main frame when the option dialog is displayed stand-alone
    if (CMainFrame* pMainFrame = AfxGetMainFrame()) {
        pMainFrame->UpdateControlState(CMainFrame::UPDATE_CONTROLS_VISIBILITY);
    }

    if (nOldDefaultToolbarSize != s.nDefaultToolbarSize) {
        m_eventc.FireEvent(MpcEvent::DEFAULT_TOOLBAR_SIZE_CHANGED);
        if (CMainFrame* pMainFrame = AfxGetMainFrame()) {
            pMainFrame->RecalcLayout();
        }
    }

    return __super::OnApply();
}

bool CPPageAdvanced::IsDefault(ADVANCED_SETTINGS eSetting) const
{
    return m_hiddenOptions.at(eSetting)->IsDefault();
}

BEGIN_MESSAGE_MAP(CPPageAdvanced, CMPCThemePPageBase)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedDefaultButton)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateDefaultButton)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnNMDblclk)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, OnNMCustomdraw)
    ON_NOTIFY(LVN_GETINFOTIP, IDC_LIST1, OnLvnGetInfoTipList)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnLvnItemchangedList)
    ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
    ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio2)
    ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombobox)
    ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit)
END_MESSAGE_MAP()

void CPPageAdvanced::OnBnClickedDefaultButton()
{
    UpdateData();

    const int iItem = GetListSelectionMark();
    if (iItem >= 0) {
        CString str;
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        auto pItem = m_hiddenOptions.at(eSetting);
        pItem->ResetDefault();

        if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
            str = pItemBool->GetValue() ? m_strTrue : m_strFalse;
            SetDlgItemText(IDC_EDIT1, str);
            if (pItemBool->GetValue()) {
                CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
            } else {
                CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
            }
        } else if (auto pItemCombo = std::dynamic_pointer_cast<SettingsCombo>(pItem)) {
            const auto& list = pItemCombo->GetList();
            str = list.at(pItemCombo->GetValue());
            m_comboBox.SetCurSel(pItemCombo->GetValue());
        } else if (auto pItemInt = std::dynamic_pointer_cast<SettingsInt>(pItem)) {
            SetDlgItemInt(IDC_EDIT1, pItemInt->GetValue());
            str.Format(_T("%d"), pItemInt->GetValue());
        } else if (auto pItemCString = std::dynamic_pointer_cast<SettingsCString>(pItem)) {
            str = pItemCString->GetValue();
            SetDlgItemText(IDC_EDIT1, pItemCString->GetValue());
        } else {
            UNREACHABLE_CODE();
        }

        m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, str, !IsDefault(eSetting));
        UpdateData(FALSE);
        m_list.Update(iItem);
        SetModified();
    }
}

void CPPageAdvanced::OnUpdateDefaultButton(CCmdUI* pCmdUI)
{
    const int iItem = GetListSelectionMark();
    bool bEnable = false;
    if (iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        bEnable = !IsDefault(eSetting);
    }
    pCmdUI->Enable(bEnable);
}

void CPPageAdvanced::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    if (pNMItemActivate->iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(pNMItemActivate->iItem));
        auto pItem = m_hiddenOptions.at(eSetting);
        if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
            pItemBool->Toggle();
            m_list.setItemTextWithDefaultFlag(pNMItemActivate->iItem, COL_VALUE,
                               pItemBool->GetValue() ? m_strTrue : m_strFalse, !IsDefault(eSetting));
            if (pItemBool->GetValue()) {
                CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
            } else {
                CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
            }
            UpdateData(FALSE);
            m_list.Update(pNMItemActivate->iItem);
            SetModified();
        }
    }
    *pResult = 0;
}

void CPPageAdvanced::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

    switch (pNMCD->dwDrawStage) {
        case CDDS_PREPAINT:
            *pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
            break;
        case CDDS_ITEMPREPAINT: {
            auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData((int)pNMCD->dwItemSpec));
            if (!IsDefault(eSetting)) {
                ::SelectObject(pNMCD->hdc, m_fontBold.GetSafeHandle());
                *pResult |= CDRF_NEWFONT;
            } else {
                *pResult = CDRF_DODEFAULT;
            }
        }
        break;
        default:
            *pResult = CDRF_DODEFAULT;
    }
}

void CPPageAdvanced::OnLvnGetInfoTipList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);

    auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(pGetInfoTip->iItem));
    auto pItem = m_hiddenOptions.at(eSetting);
    StringCchCopy(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, pItem->GetToolTipText());

    *pResult = 0;
}

void CPPageAdvanced::OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVNI_SELECTED)) {
        auto setDialogItemsVisibility = [this](std::initializer_list<int>&& ids, int nCmdShow) {
            for (const auto& nID : ids) {
                GetDlgItem(nID)->ShowWindow(nCmdShow);
            }
        };

        if (pNMLV->iItem >= 0) {
            m_lastSelectedItem = pNMLV->iItem;
            auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(pNMLV->iItem));
            auto pItem = m_hiddenOptions.at(eSetting);
            GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_SHOW);

            if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
                setDialogItemsVisibility({ IDC_EDIT1, IDC_COMBO1, IDC_SPIN1 }, SW_HIDE);
                if (pItemBool->GetValue()) {
                    CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
                } else {
                    CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
                }
                setDialogItemsVisibility({ IDC_RADIO1, IDC_RADIO2 }, SW_SHOW);
            } else if (auto pItemCombo = std::dynamic_pointer_cast<SettingsCombo>(pItem)) {
                setDialogItemsVisibility({ IDC_EDIT1, IDC_RADIO1, IDC_RADIO2, IDC_SPIN1 }, SW_HIDE);
                m_comboBox.ResetContent();
                for (const auto& str : pItemCombo->GetList()) {
                    m_comboBox.AddString(str);
                }
                m_comboBox.SetCurSel(pItemCombo->GetValue());
                m_comboBox.ShowWindow(SW_SHOW);
            } else if (auto pItemInt = std::dynamic_pointer_cast<SettingsInt>(pItem)) {
                setDialogItemsVisibility({ IDC_COMBO1, IDC_RADIO1, IDC_RADIO2 }, SW_HIDE);
                GetDlgItem(IDC_EDIT1)->ModifyStyle(0, ES_NUMBER, 0);
                const auto& range = pItemInt->GetRange();
                if (!m_spinButtonCtrl.GetBuddy()) {
                    GetDlgItem(IDC_EDIT1)->MoveWindow(editRect, TRUE);
                    m_spinButtonCtrl.SetBuddy(GetDlgItem(IDC_EDIT1));
                }
                m_spinButtonCtrl.SetRange32(range.first, range.second);
                m_spinButtonCtrl.SetPos32(pItemInt->GetValue());
                m_spinButtonCtrl.ShowWindow(SW_SHOW);
                GetDlgItem(IDC_EDIT1)->ShowWindow(SW_SHOW);
            } else if (auto pItemCString = std::dynamic_pointer_cast<SettingsCString>(pItem)) {
                setDialogItemsVisibility({ IDC_COMBO1, IDC_RADIO1, IDC_RADIO2, IDC_BUTTON1, IDC_SPIN1 }, SW_HIDE);
                GetDlgItem(IDC_EDIT1)->ModifyStyle(ES_NUMBER, 0, 0);
                SetDlgItemText(IDC_EDIT1, pItemCString->GetValue());
                m_spinButtonCtrl.SetBuddy(NULL);
                GetDlgItem(IDC_EDIT1)->ShowWindow(SW_SHOW);
            } else {
                UNREACHABLE_CODE();
            }
        } else {
            setDialogItemsVisibility({ IDC_EDIT1, IDC_COMBO1, IDC_RADIO1, IDC_RADIO2, IDC_BUTTON1, IDC_SPIN1 }, SW_HIDE);
        }
    }

    *pResult = 0;
}

void CPPageAdvanced::OnBnClickedRadio1()
{
    const int iItem = GetListSelectionMark();
    if (iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        auto pItem = m_hiddenOptions.at(eSetting);

        if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
            pItemBool->SetValue(true);
            m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, m_strTrue, !IsDefault(eSetting));
            UpdateData(FALSE);
            m_list.Update(iItem);
            SetModified();
        }
    }
}

void CPPageAdvanced::OnBnClickedRadio2()
{
    const int iItem = GetListSelectionMark();
    if (iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        auto pItem = m_hiddenOptions.at(eSetting);

        if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
            pItemBool->SetValue(false);
            m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, m_strFalse, !IsDefault(eSetting));
            UpdateData(FALSE);
            m_list.Update(iItem);
            SetModified();
        }
    }
}

void CPPageAdvanced::OnCbnSelchangeCombobox()
{
    int iItem = m_comboBox.GetCurSel();
    const int nItem = GetListSelectionMark();
    if (iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(nItem));
        auto pItem = m_hiddenOptions.at(eSetting);

        if (auto pItemCombo = std::dynamic_pointer_cast<SettingsCombo>(pItem)) {
            auto list = pItemCombo->GetList();
            pItemCombo->SetValue(iItem);
            m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, list.at(iItem), !IsDefault(eSetting));
            UpdateData(FALSE);
            m_list.Update(nItem);
            if (m_comboBox.IsWindowVisible()) {
                SetModified();
            }
        }
    }
}

void CPPageAdvanced::OnEnChangeEdit()
{
    UpdateData();

    const int iItem = GetListSelectionMark();
    if (iItem >= 0) {
        CString str;
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        auto pItem = m_hiddenOptions.at(eSetting);
        bool bChanged = false;

        if (auto pItemCombo = std::dynamic_pointer_cast<SettingsCombo>(pItem)) {
            ASSERT(FALSE);
        } else if (auto pItemInt = std::dynamic_pointer_cast<SettingsInt>(pItem)) {
            BOOL lpTrans;
            const auto& range = pItemInt->GetRange();
            int newValue = GetDlgItemInt(IDC_EDIT1, &lpTrans);
            if (!!lpTrans && range.first <= newValue && newValue <= range.second) {
                bChanged = newValue != pItemInt->GetValue();
                if (bChanged) {
                    pItemInt->SetValue(newValue);
                    str.Format(_T("%d"), pItemInt->GetValue());
                }
            } else {
                SetDlgItemInt(IDC_EDIT1, pItemInt->GetValue());
            }
        } else if (auto pItemCString = std::dynamic_pointer_cast<SettingsCString>(pItem)) {
            GetDlgItemText(IDC_EDIT1, str);
            bChanged = str != pItemCString->GetValue();
            if (bChanged) {
                pItemCString->SetValue(str);
            }
        }

        if (bChanged) {
            m_list.setItemTextWithDefaultFlag(iItem, COL_VALUE, str, !IsDefault(eSetting));
            m_list.Update(iItem);
            UpdateData(FALSE);
            SetModified();
        }
    }
}
