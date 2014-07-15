/*
* (C) 2014 see Authors.txt
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
#include <strsafe.h>
#include "PPageAdvanced.h"
#include "mplayerc.h"

CPPageAdvanced::CPPageAdvanced()
    : CPPageBase(IDD, IDD)
{
}

void CPPageAdvanced::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list);
    DDX_Control(pDX, IDC_COMBO1, m_comboBox);
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
    m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
    m_list.InsertColumn(COL_NAME, ResStr(IDS_PPAGEADVANCED_COL_NAME), LVCFMT_LEFT);
    m_list.InsertColumn(COL_VALUE, ResStr(IDS_PPAGEADVANCED_COL_VALUE), LVCFMT_RIGHT);

    GetDlgItem(IDC_EDIT1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_COMBO1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_RADIO1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_RADIO2)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_HIDE);

    InitSettings();

    for (int i = 0; i < m_list.GetHeaderCtrl()->GetItemCount(); ++i) {
        m_list.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
    }
    SetRedraw(TRUE);
    return TRUE;
}

void CPPageAdvanced::InitSettings()
{
    auto& s = AfxGetAppSettings();

    auto addBoolItem = [this](int nItem, CString name, bool defaultValue, bool & settingReference, CString toolTipText) {
        auto pItem = std::make_shared<SettingsBool>(name, defaultValue, settingReference, toolTipText);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        m_list.SetItemText(iItem, COL_VALUE, (pItem->GetValue() ? _T("true") : _T("false")));
        m_list.SetItemData(iItem, nItem);
        m_hiddenOptions[static_cast<ADVANCED_SETTINGS>(nItem)] = pItem;
    };

    // The range parameter defines range (inclusive) that particular option can have.
    auto addIntItem = [this](int nItem, CString name, int defaultValue, int& settingReference, std::pair<int, int> range, CString toolTipText) {
        auto pItem = std::make_shared<SettingsInt>(name, defaultValue, settingReference, range, toolTipText);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        CString str;
        str.Format(_T("%d"), pItem->GetValue());
        m_list.SetItemText(iItem, COL_VALUE, str);
        m_list.SetItemData(iItem, nItem);
        m_hiddenOptions[static_cast<ADVANCED_SETTINGS>(nItem)] = pItem;
    };

    // The list parameter defines list of the strings that will be in the combobox.
    auto addComboItem = [this](int nItem, CString name, int defaultValue, int& settingReference, std::deque<CString> list, CString toolTipText) {
        auto pItem = std::make_shared<SettingsCombo>(name, defaultValue, settingReference, list, toolTipText);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        m_list.SetItemText(iItem, COL_VALUE, list.at(settingReference));
        m_list.SetItemData(iItem, nItem);
        m_hiddenOptions[static_cast<ADVANCED_SETTINGS>(nItem)] = pItem;
    };

    auto addCStringItem = [this](int nItem, CString name, CString defaultValue, CString & settingReference, CString toolTipText) {
        auto pItem = std::make_shared<SettingsCString>(name, defaultValue, settingReference, toolTipText);
        int iItem = m_list.InsertItem(nItem, pItem->GetName());
        m_list.SetItemText(iItem, COL_VALUE, pItem->GetValue());
        m_list.SetItemData(iItem, nItem);
        m_hiddenOptions[static_cast<ADVANCED_SETTINGS>(nItem)] = pItem;
    };

    addBoolItem(HIDE_WINDOWED, _T("HideWindowedControls"), false, s.bHideWindowedControls, ResStr(IDS_PPAGEADVANCED_HIDE_WINDOWED));
    addBoolItem(BLOCK_VSFILTER, _T("BlockVSFilter"), true, s.fBlockVSFilter, ResStr(IDS_PPAGEADVANCED_BLOCK_VSFILTER));
    addIntItem(RECENT_FILES_NB, _T("RecentFilesNumber"), 20, s.iRecentFilesNumber, std::make_pair(0, 1000), ResStr(IDS_PPAGEADVANCED_RECENT_FILES_NUMBER));
}

BOOL CPPageAdvanced::OnApply()
{
    for (int i = 0; i < m_list.GetItemCount(); i++) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(i));
        m_hiddenOptions.at(eSetting)->Apply();
    }

    auto& s = AfxGetAppSettings();

    s.MRU.SetSize(s.iRecentFilesNumber);
    s.MRUDub.SetSize(s.iRecentFilesNumber);
    s.filePositions.SetMaxSize(s.iRecentFilesNumber);
    s.dvdPositions.SetMaxSize(s.iRecentFilesNumber);

    return __super::OnApply();
}

bool CPPageAdvanced::IsDefault(ADVANCED_SETTINGS eSetting) const
{
    return m_hiddenOptions.at(eSetting)->IsDefault();
}

BEGIN_MESSAGE_MAP(CPPageAdvanced, CPPageBase)
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

    int iItem = m_list.GetSelectionMark();
    if (iItem >= 0) {
        SetRedraw(FALSE);
        CString str;
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        auto pItem = m_hiddenOptions.at(eSetting);
        pItem->ResetDefault();

        if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
            str = pItemBool->GetValue() ? _T("true") : _T("false");
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
            ASSERT(FALSE);
            return;
        }

        m_list.SetItemText(iItem, COL_VALUE, str);
        UpdateData(FALSE);
        SetRedraw(TRUE);
        Invalidate();
        SetModified();
    }
}

void CPPageAdvanced::OnUpdateDefaultButton(CCmdUI* pCmdUI)
{
    int iItem = m_list.GetSelectionMark();
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
            SetRedraw(FALSE);
            pItemBool->Toggle();
            CString str = pItemBool->GetValue() ? _T("true") : _T("false");
            m_list.SetItemText(pNMItemActivate->iItem, COL_VALUE, str);
            if (pItemBool->GetValue()) {
                CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
            } else {
                CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
            }
            UpdateData(FALSE);
            SetRedraw(TRUE);
            Invalidate();
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
        SetRedraw(FALSE);
        GetDlgItem(IDC_EDIT1)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_COMBO1)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO1)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO2)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_HIDE);
        if (pNMLV->iItem >= 0) {
            auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(pNMLV->iItem));
            auto pItem = m_hiddenOptions.at(eSetting);
            GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_SHOW);

            if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
                if (pItemBool->GetValue()) {
                    CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
                } else {
                    CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
                }
                GetDlgItem(IDC_RADIO1)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_RADIO2)->ShowWindow(SW_SHOW);
            } else if (auto pItemCombo = std::dynamic_pointer_cast<SettingsCombo>(pItem)) {
                m_comboBox.ResetContent();
                for (const auto& str : pItemCombo->GetList()) {
                    m_comboBox.AddString(str);
                }
                m_comboBox.SetCurSel(pItemCombo->GetValue());
                m_comboBox.ShowWindow(SW_SHOW);
            } else if (auto pItemInt = std::dynamic_pointer_cast<SettingsInt>(pItem)) {
                GetDlgItem(IDC_EDIT1)->ModifyStyle(0, ES_NUMBER, 0);
                SetDlgItemInt(IDC_EDIT1, pItemInt->GetValue());
                GetDlgItem(IDC_EDIT1)->ShowWindow(SW_SHOW);
            } else if (auto pItemCString = std::dynamic_pointer_cast<SettingsCString>(pItem)) {
                GetDlgItem(IDC_EDIT1)->ModifyStyle(ES_NUMBER, 0, 0);
                SetDlgItemText(IDC_EDIT1, pItemCString->GetValue());
                GetDlgItem(IDC_EDIT1)->ShowWindow(SW_SHOW);
            }
        }
        SetRedraw(TRUE);
        Invalidate();
    }

    *pResult = 0;
}

void CPPageAdvanced::OnBnClickedRadio1()
{
    int iItem = m_list.GetSelectionMark();
    if (iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        auto pItem = m_hiddenOptions.at(eSetting);

        if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
            SetRedraw(FALSE);
            pItemBool->SetValue(true);
            m_list.SetItemText(iItem, COL_VALUE, _T("true"));
            UpdateData(FALSE);
            SetRedraw(TRUE);
            Invalidate();
            SetModified();
        }
    }
}

void CPPageAdvanced::OnBnClickedRadio2()
{
    int iItem = m_list.GetSelectionMark();
    if (iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(iItem));
        auto pItem = m_hiddenOptions.at(eSetting);

        if (auto pItemBool = std::dynamic_pointer_cast<SettingsBool>(pItem)) {
            SetRedraw(FALSE);
            pItemBool->SetValue(false);
            m_list.SetItemText(iItem, COL_VALUE, _T("false"));
            UpdateData(FALSE);
            SetRedraw(TRUE);
            Invalidate();
            SetModified();
        }
    }
}

void CPPageAdvanced::OnCbnSelchangeCombobox()
{
    int iItem = m_comboBox.GetCurSel();
    int nItem = m_list.GetSelectionMark();
    if (iItem >= 0) {
        auto eSetting = static_cast<ADVANCED_SETTINGS>(m_list.GetItemData(nItem));
        auto pItem = m_hiddenOptions.at(eSetting);

        if (auto pItemCombo = std::dynamic_pointer_cast<SettingsCombo>(pItem)) {
            auto list = pItemCombo->GetList();
            pItemCombo->SetValue(iItem);
            m_list.SetItemText(nItem, COL_VALUE, list.at(iItem));
            UpdateData(FALSE);
            Invalidate();
            if (m_comboBox.IsWindowVisible()) {
                SetModified();
            }
        }
    }
}

void CPPageAdvanced::OnEnChangeEdit()
{
    UpdateData();

    int iItem = m_list.GetSelectionMark();
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
            m_list.SetItemText(iItem, COL_VALUE, str);
            UpdateData(FALSE);
            Invalidate();
            SetModified();
        }
    }
}
