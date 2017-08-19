/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016-2017 see Authors.txt
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
#include <algorithm>
#include <commdlg.h>
#include <afxdlgs.h>
#include "DirectVobSubFilter.h"
#include "DirectVobSubPropPage.h"
#include "VSFilter.h"
#include "StyleEditorDialog.h"

#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/MediaTypes.h"

#include "version.h"


BOOL WINAPI MyGetDialogSize(int iResourceID, DLGPROC pDlgProc, LPARAM lParam, SIZE* pResult)
{
    HWND hwnd = CreateDialogParam(AfxGetResourceHandle(),
                                  MAKEINTRESOURCE(iResourceID),
                                  GetDesktopWindow(),
                                  pDlgProc,
                                  lParam);

    if (hwnd == nullptr) {
        return FALSE;
    }

    RECT rc;
    GetWindowRect(hwnd, &rc);
    pResult->cx = rc.right - rc.left;
    pResult->cy = rc.bottom - rc.top;

    DestroyWindow(hwnd);

    return TRUE;
}

STDMETHODIMP CDVSBasePPage::GetPageInfo(LPPROPPAGEINFO pPageInfo)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString str;
    if (!str.LoadString(m_TitleId)) {
        return E_FAIL;
    }

    WCHAR wszTitle[STR_MAX_LENGTH];
    wcscpy_s(wszTitle, str);

    CheckPointer(pPageInfo, E_POINTER);

    // Allocate dynamic memory for the property page title

    LPOLESTR pszTitle;
    HRESULT hr = AMGetWideString(wszTitle, &pszTitle);
    if (FAILED(hr)) {
        TRACE(_T("No caption memory"));
        return hr;
    }

    pPageInfo->cb               = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle         = pszTitle;
    pPageInfo->pszDocString     = nullptr;
    pPageInfo->pszHelpFile      = nullptr;
    pPageInfo->dwHelpContext    = 0;
    // Set defaults in case GetDialogSize fails
    pPageInfo->size.cx          = 340;
    pPageInfo->size.cy          = 150;

    MyGetDialogSize(m_DialogId, DialogProc, 0L, &pPageInfo->size);

    return NOERROR;
}

STDMETHODIMP CDVSBasePPage::Activate(HWND hwndParent, LPCRECT pRect, BOOL fModal)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CheckPointer(pRect, E_POINTER);
    /*
        // Return failure if SetObject has not been called.
        if (m_bObjectSet == FALSE) {
            return E_UNEXPECTED;
        }
    */
    if (m_hwnd) {
        return E_UNEXPECTED;
    }

    m_hwnd = CreateDialogParam(AfxGetResourceHandle(), MAKEINTRESOURCE(m_DialogId), hwndParent, DialogProc, (LPARAM)this);
    if (m_hwnd == nullptr) {
        return E_OUTOFMEMORY;
    }

    OnActivate();
    Move(pRect);
    return Show(SW_SHOWNORMAL);
}

/* CDVSBasePPage */

CDVSBasePPage::CDVSBasePPage(LPCTSTR pName, LPUNKNOWN lpunk, int DialogId, int TitleId)
    : CBasePropertyPage(pName, lpunk, DialogId, TitleId)
    , m_fDisableInstantUpdate(false)
    , m_bIsInitialized(FALSE)
    , m_fAttached(false)
{
}

INT_PTR CDVSBasePPage::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            if (m_bIsInitialized) {
                m_bDirty = TRUE;
                if (m_pPageSite) {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }

                switch (HIWORD(wParam)) {
                    case BN_CLICKED:
                    case CBN_SELCHANGE:
                    case EN_CHANGE: {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());

                        if (!m_fDisableInstantUpdate
                                && !(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_INSTANTUPDATE)
                                && LOWORD(wParam) != IDC_EDIT1
                                && !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_INSTANTUPDATE), TRUE)) {
                            OnApplyChanges();
                        }
                    }
                }
            }
        }
        break;

        case WM_NCDESTROY:
            DetachControls();
            break;
    }

    return OnMessage(uMsg, wParam, lParam)
           ? 0
           : CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

HRESULT CDVSBasePPage::OnConnect(IUnknown* pUnknown)
{
    if (!(m_pDirectVobSub = pUnknown)) {
        return E_NOINTERFACE;
    }

    m_pDirectVobSub->LockSubtitleReloader(true); // *

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    UpdateObjectData(false);

    m_bIsInitialized = FALSE;

    return NOERROR;
}

HRESULT CDVSBasePPage::OnDisconnect()
{
    if (m_pDirectVobSub == nullptr) {
        return E_UNEXPECTED;
    }

    m_pDirectVobSub->LockSubtitleReloader(false); // *

    // for some reason OnDisconnect() will be called twice, that's why we
    // need to release m_pDirectVobSub manually on the first call to avoid
    // a second "m_pDirectVobSub->LockSubtitleReloader(false);"

    m_pDirectVobSub.Release();

    return NOERROR;
}

HRESULT CDVSBasePPage::OnActivate()
{
    ASSERT(m_pDirectVobSub);

    AttachControls();

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    UpdateControlData(false);

    m_bIsInitialized = TRUE;

    return NOERROR;
}

HRESULT CDVSBasePPage::OnDeactivate()
{
    ASSERT(m_pDirectVobSub);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    UpdateControlData(true);

    m_bIsInitialized = FALSE;

    return NOERROR;
}

HRESULT CDVSBasePPage::OnApplyChanges()
{
    ASSERT(m_pDirectVobSub);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_bIsInitialized) {
        OnDeactivate();
        UpdateObjectData(true);
        m_pDirectVobSub->UpdateRegistry(); // *
        OnActivate();
    }

    return NOERROR;
}

void CDVSBasePPage::AttachControls()
{
    DetachControls();

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    POSITION pos = m_controls.GetStartPosition();
    while (pos) {
        UINT id;
        CWnd* pControl;
        m_controls.GetNextAssoc(pos, id, pControl);
        if (pControl) {
            BOOL fRet = pControl->Attach(GetDlgItem(m_Dlg, id));
            ASSERT(fRet);
        }
    }

    m_fAttached = true;
}

void CDVSBasePPage::DetachControls()
{
    if (!m_fAttached) {
        return;
    }

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    POSITION pos = m_controls.GetStartPosition();
    while (pos) {
        UINT id;
        CWnd* pControl;
        m_controls.GetNextAssoc(pos, id, pControl);
        if (pControl) {
            pControl->Detach();
        }
    }

    m_fAttached = false;
}

void CDVSBasePPage::BindControl(UINT id, CWnd& control)
{
    m_controls[id] = &control;
}

/* CDVSMainPPage */

#pragma warning(push)
#pragma warning(disable: 4351) // new behavior: elements of array 'array' will be default initialized
CDVSMainPPage::CDVSMainPPage(LPUNKNOWN pUnk, HRESULT* phr)
    : CDVSBasePPage(NAME("VSFilter Property Page (main)"), pUnk, IDD_DVSMAINPAGE, IDD_DVSMAINPAGE)
    , m_fn()
    , m_iSelectedLanguage(0)
    , m_nLangs(0)
    , m_ppLangs(nullptr)
    , m_fOverridePlacement(false)
    , m_PlacementXperc(50)
    , m_PlacementYperc(90)
    , m_fOnlyShowForcedVobSubs(false)
    , m_ePARCompensationType(CSimpleTextSubtitle::EPCTDisabled)
{
    BindControl(IDC_FILENAME, m_fnedit);
    BindControl(IDC_LANGCOMBO, m_langs);
    BindControl(IDC_OVERRIDEPLACEMENT, m_oplacement);
    BindControl(IDC_SPIN1, m_subposx);
    BindControl(IDC_SPIN2, m_subposy);
    BindControl(IDC_FONT, m_font);
    BindControl(IDC_ONLYSHOWFORCEDSUBS, m_forcedsubs);
    BindControl(IDC_PARCOMBO, m_PARCombo);
}
#pragma warning(pop)

CDVSMainPPage::~CDVSMainPPage()
{
    FreeLangs();
}

void CDVSMainPPage::FreeLangs()
{
    if (m_nLangs > 0 && m_ppLangs) {
        for (ptrdiff_t i = 0; i < m_nLangs; i++) {
            CoTaskMemFree(m_ppLangs[i]);
        }
        CoTaskMemFree(m_ppLangs);
        m_nLangs = 0;
        m_ppLangs = nullptr;
    }
}

void CDVSMainPPage::AllocLangs(int nLangs)
{
    m_ppLangs = (WCHAR**)CoTaskMemRealloc(m_ppLangs, sizeof(WCHAR*)*nLangs);
    m_nLangs = nLangs;
}

bool CDVSMainPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case BN_CLICKED: {
                    if (LOWORD(wParam) == IDC_OPEN) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());

                        CFileDialog fd(TRUE, nullptr, nullptr,
                                       OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
                                       _T(".idx .smi .sub .srt .psb .ssa .ass .usf .xss .txt .rt .sup|*.idx;*.smi;*.sub;*.srt;*.psb;*.ssa;*.ass;*.usf;*.xss;*.txt;*.rt;*.sup|")
                                       _T("All files (*.*)|*.*||"),
                                       CDialog::FromHandle(m_Dlg), 0);

                        if (fd.DoModal() == IDOK) {
                            m_fnedit.SetWindowText(fd.GetPathName());
                        }

                        return true;
                    } else if (LOWORD(wParam) == IDC_FONT) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());

                        CStyleEditorDialog dlg(_T("Default"), &m_defStyle, CWnd::FromHandle(m_hwnd));

                        if (dlg.DoModal() == IDOK) {
                            dlg.GetStyle(m_defStyle);
                            CString str = m_defStyle.fontName;
                            if (str.GetLength() > 18) {
                                str = str.Left(16).TrimRight() + _T("...");
                            }
                            m_font.SetWindowText(str);
                        }

                        return true;
                    }
                }
                break;
            }
        }
        break;
    }

    return false;
}

void CDVSMainPPage::UpdateObjectData(bool fSave)
{
    if (fSave) {
        if (m_pDirectVobSub->put_FileName(m_fn) == S_OK) {
            int nLangs;
            m_pDirectVobSub->get_LanguageCount(&nLangs);
            AllocLangs(nLangs);
            for (int i = 0; i < m_nLangs; i++) {
                m_pDirectVobSub->get_LanguageName(i, &m_ppLangs[i]);
            }
            m_pDirectVobSub->get_SelectedLanguage(&m_iSelectedLanguage);
        }

        m_pDirectVobSub->put_SelectedLanguage(m_iSelectedLanguage);
        m_pDirectVobSub->put_Placement(m_fOverridePlacement, m_PlacementXperc, m_PlacementYperc);
        m_pDirectVobSub->put_VobSubSettings(true, m_fOnlyShowForcedVobSubs, false);
        m_pDirectVobSub->put_TextSettings(&m_defStyle);
        m_pDirectVobSub->put_AspectRatioSettings(&m_ePARCompensationType);
    } else {
        m_pDirectVobSub->get_FileName(m_fn);
        int nLangs;
        m_pDirectVobSub->get_LanguageCount(&nLangs);
        AllocLangs(nLangs);
        for (int i = 0; i < m_nLangs; i++) {
            m_pDirectVobSub->get_LanguageName(i, &m_ppLangs[i]);
        }
        m_pDirectVobSub->get_SelectedLanguage(&m_iSelectedLanguage);
        m_pDirectVobSub->get_Placement(&m_fOverridePlacement, &m_PlacementXperc, &m_PlacementYperc);
        m_pDirectVobSub->get_VobSubSettings(nullptr, &m_fOnlyShowForcedVobSubs, nullptr);
        m_pDirectVobSub->get_TextSettings(&m_defStyle);
        m_pDirectVobSub->get_AspectRatioSettings(&m_ePARCompensationType);
    }
}

void CDVSMainPPage::UpdateControlData(bool fSave)
{
    if (fSave) {
        CString fn;
        m_fnedit.GetWindowText(fn);
        wcscpy_s(m_fn, fn);

        m_iSelectedLanguage = m_langs.GetCurSel();
        m_fOverridePlacement = !!m_oplacement.GetCheck();
        m_PlacementXperc = m_subposx.GetPos32();
        m_PlacementYperc = m_subposy.GetPos32();
        m_fOnlyShowForcedVobSubs = !!m_forcedsubs.GetCheck();
        if (m_PARCombo.GetCurSel() != CB_ERR) {
            m_ePARCompensationType = static_cast<CSimpleTextSubtitle::EPARCompensationType>(m_PARCombo.GetItemData(m_PARCombo.GetCurSel()));
        } else {
            m_ePARCompensationType = CSimpleTextSubtitle::EPCTDisabled;
        }
    } else {
        m_fnedit.SetWindowText(CString(m_fn));
        m_oplacement.SetCheck(m_fOverridePlacement);
        m_subposx.SetRange32(-20, 120);
        m_subposx.SetPos32(m_PlacementXperc);
        m_subposx.EnableWindow(m_fOverridePlacement);
        m_subposy.SetRange32(-20, 120);
        m_subposy.SetPos32(m_PlacementYperc);
        m_subposy.EnableWindow(m_fOverridePlacement);
        m_font.SetWindowText(m_defStyle.fontName);
        m_forcedsubs.SetCheck(m_fOnlyShowForcedVobSubs);
        m_langs.ResetContent();
        m_langs.EnableWindow(m_nLangs > 0);
        for (ptrdiff_t i = 0; i < m_nLangs; i++) {
            m_langs.AddString(CString(m_ppLangs[i]));
        }
        CorrectComboListWidth(m_langs);
        m_langs.SetCurSel(m_iSelectedLanguage);

        m_PARCombo.ResetContent();
        m_PARCombo.InsertString(0, ResStr(IDS_RT_PAR_DISABLED));
        m_PARCombo.SetItemData(0, CSimpleTextSubtitle::EPCTDisabled);
        if (m_ePARCompensationType == CSimpleTextSubtitle::EPCTDisabled) {
            m_PARCombo.SetCurSel(0);
        }

        m_PARCombo.InsertString(1, ResStr(IDS_RT_PAR_DOWNSCALE));
        m_PARCombo.SetItemData(1, CSimpleTextSubtitle::EPCTDownscale);
        if (m_ePARCompensationType == CSimpleTextSubtitle::EPCTDownscale) {
            m_PARCombo.SetCurSel(1);
        }

        m_PARCombo.InsertString(2, ResStr(IDS_RT_PAR_UPSCALE));
        m_PARCombo.SetItemData(2, CSimpleTextSubtitle::EPCTUpscale);
        if (m_ePARCompensationType == CSimpleTextSubtitle::EPCTUpscale) {
            m_PARCombo.SetCurSel(2);
        }

        m_PARCombo.InsertString(3, ResStr(IDS_RT_PAR_ACCURATE_SIZE));
        m_PARCombo.SetItemData(3, CSimpleTextSubtitle::EPCTAccurateSize);
        if (m_ePARCompensationType == CSimpleTextSubtitle::EPCTAccurateSize) {
            m_PARCombo.SetCurSel(3);
        }
    }
}

/* CDVSGeneralPPage */

CDVSGeneralPPage::CDVSGeneralPPage(LPUNKNOWN pUnk, HRESULT* phr)
    : CDVSBasePPage(NAME("VSFilter Property Page (global settings)"), pUnk, IDD_DVSGENERALPAGE, IDD_DVSGENERALPAGE)
    , m_HorExt(0)
    , m_VerExt(0)
    , m_ResX2(0)
    , m_ResX2minw(0)
    , m_ResX2minh(0)
    , m_LoadLevel(0)
    , m_fExternalLoad(true)
    , m_fWebLoad(true)
    , m_fEmbeddedLoad(true)
{
    BindControl(IDC_VEREXTCOMBO, m_verext);
    BindControl(IDC_MOD32FIX, m_mod32fix);
    BindControl(IDC_RESX2COMBO, m_resx2);
    BindControl(IDC_SPIN3, m_resx2w);
    BindControl(IDC_SPIN4, m_resx2h);
    BindControl(IDC_LOADCOMBO, m_load);
    BindControl(IDC_EXTLOAD, m_extload);
    BindControl(IDC_WEBLOAD, m_webload);
    BindControl(IDC_EMBLOAD, m_embload);
}

bool CDVSGeneralPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case CBN_SELCHANGE: {
                    AFX_MANAGE_STATE(AfxGetStaticModuleState());

                    if (LOWORD(wParam) == IDC_RESX2COMBO) {
                        m_resx2w.EnableWindow(m_resx2.GetCurSel() == 2);
                        m_resx2h.EnableWindow(m_resx2.GetCurSel() == 2);
                        return true;
                    } else if (LOWORD(wParam) == IDC_LOADCOMBO) {
                        m_extload.EnableWindow(m_load.GetCurSel() == 1);
                        m_webload.EnableWindow(m_load.GetCurSel() == 1);
                        m_embload.EnableWindow(m_load.GetCurSel() == 1);
                        return true;
                    }
                }
                break;
            }
        }
        break;
    }

    return false;
}

void CDVSGeneralPPage::UpdateObjectData(bool fSave)
{
    if (fSave) {
        m_pDirectVobSub->put_ExtendPicture(m_HorExt, m_VerExt, m_ResX2, m_ResX2minw, m_ResX2minh);
        m_pDirectVobSub->put_LoadSettings(m_LoadLevel, m_fExternalLoad, m_fWebLoad, m_fEmbeddedLoad);
    } else {
        m_pDirectVobSub->get_ExtendPicture(&m_HorExt, &m_VerExt, &m_ResX2, &m_ResX2minw, &m_ResX2minh);
        m_pDirectVobSub->get_LoadSettings(&m_LoadLevel, &m_fExternalLoad, &m_fWebLoad, &m_fEmbeddedLoad);
    }
}

void CDVSGeneralPPage::UpdateControlData(bool fSave)
{
    if (fSave) {
        if (m_verext.GetCurSel() >= 0) {
            m_VerExt = (int)m_verext.GetItemData(m_verext.GetCurSel());
        }
        m_HorExt = !!m_mod32fix.GetCheck();
        if (m_resx2.GetCurSel() >= 0) {
            m_ResX2 = (int)m_resx2.GetItemData(m_resx2.GetCurSel());
        }
        m_ResX2minw = m_resx2w.GetPos32();
        m_ResX2minh = m_resx2h.GetPos32();
        if (m_load.GetCurSel() >= 0) {
            m_LoadLevel = (int)m_load.GetItemData(m_load.GetCurSel());
        }
        m_fExternalLoad = !!m_extload.GetCheck();
        m_fWebLoad = !!m_webload.GetCheck();
        m_fEmbeddedLoad = !!m_embload.GetCheck();
    } else {
        m_verext.ResetContent();
        m_verext.AddString(ResStr(IDS_ORGHEIGHT));
        m_verext.SetItemData(0, 0);
        m_verext.AddString(ResStr(IDS_EXTTO169));
        m_verext.SetItemData(1, 1);
        m_verext.AddString(ResStr(IDS_EXTTO43));
        m_verext.SetItemData(2, 2);
        m_verext.AddString(ResStr(IDS_EXTTO480));
        m_verext.SetItemData(3, 3);
        m_verext.AddString(ResStr(IDS_EXTTO576));
        m_verext.SetItemData(4, 4);
        m_verext.AddString(ResStr(IDS_CROPTO169));
        m_verext.SetItemData(5, 0x81);
        m_verext.AddString(ResStr(IDS_CROPTO43));
        m_verext.SetItemData(6, 0x82);
        m_verext.SetCurSel((m_VerExt & 0x7f) + ((m_VerExt & 0x80) ? 4 : 0));
        m_mod32fix.SetCheck(m_HorExt & 1);
        m_resx2.ResetContent();
        m_resx2.AddString(ResStr(IDS_ORGRES));
        m_resx2.SetItemData(0, 0);
        m_resx2.AddString(ResStr(IDS_DBLRES));
        m_resx2.SetItemData(1, 1);
        m_resx2.AddString(ResStr(IDS_DBLRESIF));
        m_resx2.SetItemData(2, 2);
        m_resx2.SetCurSel(m_ResX2);
        m_resx2w.SetRange32(0, 2048);
        m_resx2w.SetPos32(m_ResX2minw);
        m_resx2w.EnableWindow(m_ResX2 == 2);
        m_resx2h.SetRange32(0, 2048);
        m_resx2h.SetPos32(m_ResX2minh);
        m_resx2h.EnableWindow(m_ResX2 == 2);
        m_load.ResetContent();
        m_load.AddString(ResStr(IDS_DONOTLOAD));
        m_load.SetItemData(0, 2);
        m_load.AddString(ResStr(IDS_LOADWHENNEEDED));
        m_load.SetItemData(1, 0);
        m_load.AddString(ResStr(IDS_ALWAYSLOAD));
        m_load.SetItemData(2, 1);
        m_load.SetCurSel(!m_LoadLevel ? 1 : m_LoadLevel == 1 ? 2 : 0);
        m_extload.SetCheck(m_fExternalLoad);
        m_webload.SetCheck(m_fWebLoad);
        m_embload.SetCheck(m_fEmbeddedLoad);
        m_extload.EnableWindow(m_load.GetCurSel() == 1);
        m_webload.EnableWindow(m_load.GetCurSel() == 1);
        m_embload.EnableWindow(m_load.GetCurSel() == 1);
    }
}

/* CDVSSubpicQueuePPage */

CDVSSubpicQueuePPage::CDVSSubpicQueuePPage(LPUNKNOWN pUnk, HRESULT* phr)
    : CDVSBasePPage(NAME("VSFilter Property Page (subpicture queue settings)"), pUnk, IDD_DVSSUBPICQUEUEPAGE, IDD_DVSSUBPICQUEUEPAGE)
    , m_nSubPictToBuffer(10)
    , m_bDisableSubtitleAnimation(false)
    , m_bAllowDroppingSubpic(true)
    , m_nRenderAtWhenAnimationIsDisabled(50)
    , m_nAnimationRate(100)
{
    m_fDisableInstantUpdate = true;

    BindControl(IDC_PREBUFFERING, m_subPictToBufferCtrl);
    BindControl(IDC_CHECK_NO_SUB_ANIM, m_disableSubtitleAnimationButton);
    BindControl(IDC_SPIN2, m_renderAtCtrl);
    BindControl(IDC_SPIN3, m_animationRateCtrl);
    BindControl(IDC_CHECK_ALLOW_DROPPING_SUBPIC, m_allowDroppingSubpicButton);
}

void CDVSSubpicQueuePPage::UpdateEditControls(bool bDisableSubtitleAnimation)
{
    ::EnableWindow(::GetDlgItem(m_Dlg, IDC_STATIC1), bDisableSubtitleAnimation);
    m_renderAtCtrl.EnableWindow(bDisableSubtitleAnimation);
    ::EnableWindow(::GetDlgItem(m_Dlg, IDC_STATIC2), bDisableSubtitleAnimation);
    ::EnableWindow(::GetDlgItem(m_Dlg, IDC_STATIC3), !bDisableSubtitleAnimation);
    m_animationRateCtrl.EnableWindow(!bDisableSubtitleAnimation);
    ::EnableWindow(::GetDlgItem(m_Dlg, IDC_STATIC4), !bDisableSubtitleAnimation);
}

void CDVSSubpicQueuePPage::UpdateAllowDroppingSubpicControl(bool bBufferingEnabled)
{
    m_allowDroppingSubpicButton.EnableWindow(bBufferingEnabled);
}

bool CDVSSubpicQueuePPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case BN_CLICKED: {
                    if (LOWORD(wParam) == IDC_CHECK_NO_SUB_ANIM) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        UpdateEditControls(!!m_disableSubtitleAnimationButton.GetCheck());
                        return true;
                    }
                }
                break;
            }
        }
        break;
        case WM_VSCROLL: {
            if (LOWORD(wParam) == SB_THUMBPOSITION) {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());
                UpdateAllowDroppingSubpicControl(HIWORD(wParam) > 0);
                return true;
            }
        }
        break;
    }

    return false;
}

void CDVSSubpicQueuePPage::UpdateObjectData(bool bSave)
{
    if (bSave) {
        m_pDirectVobSub->put_SubPictToBuffer(m_nSubPictToBuffer);
        m_pDirectVobSub->put_DisableSubtitleAnimation(m_bDisableSubtitleAnimation);
        m_pDirectVobSub->put_RenderAtWhenAnimationIsDisabled(m_nRenderAtWhenAnimationIsDisabled);
        m_pDirectVobSub->put_AnimationRate(m_nAnimationRate);
        m_pDirectVobSub->put_AllowDroppingSubpic(m_bAllowDroppingSubpic);
    } else {
        m_pDirectVobSub->get_SubPictToBuffer(&m_nSubPictToBuffer);
        m_bDisableSubtitleAnimation = m_pDirectVobSub->get_DisableSubtitleAnimation();
        m_nRenderAtWhenAnimationIsDisabled = m_pDirectVobSub->get_RenderAtWhenAnimationIsDisabled();
        m_nAnimationRate = m_pDirectVobSub->get_AnimationRate();
        m_bAllowDroppingSubpic = m_pDirectVobSub->get_AllowDroppingSubpic();
    }
}

void CDVSSubpicQueuePPage::UpdateControlData(bool bSave)
{
    if (bSave) {
        m_nSubPictToBuffer = m_subPictToBufferCtrl.GetPos32();
        m_bDisableSubtitleAnimation = !!m_disableSubtitleAnimationButton.GetCheck();
        m_nRenderAtWhenAnimationIsDisabled = m_renderAtCtrl.GetPos32();
        m_nAnimationRate = m_animationRateCtrl.GetPos32();
        m_bAllowDroppingSubpic = !!m_allowDroppingSubpicButton.GetCheck();
    } else {
        m_subPictToBufferCtrl.SetPos32(m_nSubPictToBuffer);
        m_subPictToBufferCtrl.SetRange32(0, 120);
        m_disableSubtitleAnimationButton.SetCheck(m_bDisableSubtitleAnimation);
        m_renderAtCtrl.SetPos32(m_nRenderAtWhenAnimationIsDisabled);
        m_renderAtCtrl.SetRange32(0, 100);
        m_animationRateCtrl.SetPos32(m_nAnimationRate);
        m_animationRateCtrl.SetRange32(10, 100);
        UDACCEL accels[] = { { 0, 5 }, { 2, 10 }, { 5, 20 } };
        m_animationRateCtrl.SetAccel(_countof(accels), accels);
        m_allowDroppingSubpicButton.SetCheck(m_bAllowDroppingSubpic);

        UpdateEditControls(m_bDisableSubtitleAnimation);
        UpdateAllowDroppingSubpicControl(m_nSubPictToBuffer > 0);
    }
}

/* CDVSMiscPPage */

CDVSMiscPPage::CDVSMiscPPage(LPUNKNOWN pUnk, HRESULT* phr)
    : CDVSBasePPage(NAME("VSFilter Property Page (misc settings)"), pUnk, IDD_DVSMISCPAGE, IDD_DVSMISCPAGE)
    , m_fFlipPicture(false)
    , m_fFlipSubtitles(false)
    , m_fHideSubtitles(false)
    , m_fOSD(false)
    , m_fReloaderDisabled(false)
    , m_fSaveFullPath(false)
{
    BindControl(IDC_FLIP, m_flippic);
    BindControl(IDC_FLIPSUB, m_flipsub);
    BindControl(IDC_HIDE, m_hidesub);
    BindControl(IDC_SHOWOSDSTATS, m_showosd);
    BindControl(IDC_AUTORELOAD, m_autoreload);
    BindControl(IDC_SAVEFULLPATH, m_savefullpath);
    BindControl(IDC_INSTANTUPDATE, m_instupd);
}

bool CDVSMiscPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case BN_CLICKED: {
                    if (LOWORD(wParam) == IDC_INSTANTUPDATE) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_INSTANTUPDATE), !!m_instupd.GetCheck());
                        return true;
                    }
                }
                break;
            }
        }
        break;
    }

    return false;
}

void CDVSMiscPPage::UpdateObjectData(bool fSave)
{
    if (fSave) {
        m_pDirectVobSub->put_Flip(m_fFlipPicture, m_fFlipSubtitles);
        m_pDirectVobSub->put_HideSubtitles(m_fHideSubtitles);
        m_pDirectVobSub->put_OSD(m_fOSD);
        m_pDirectVobSub->put_SubtitleReloader(m_fReloaderDisabled);
        m_pDirectVobSub->put_SaveFullPath(m_fSaveFullPath);
    } else {
        m_pDirectVobSub->get_Flip(&m_fFlipPicture, &m_fFlipSubtitles);
        m_pDirectVobSub->get_HideSubtitles(&m_fHideSubtitles);
        m_pDirectVobSub->get_OSD(&m_fOSD);
        m_pDirectVobSub->get_SubtitleReloader(&m_fReloaderDisabled);
        m_pDirectVobSub->get_SaveFullPath(&m_fSaveFullPath);
    }
}

void CDVSMiscPPage::UpdateControlData(bool fSave)
{
    if (fSave) {
        m_fFlipPicture = !!m_flippic.GetCheck();
        m_fFlipSubtitles = !!m_flipsub.GetCheck();
        m_fHideSubtitles = !!m_hidesub.GetCheck();
        m_fSaveFullPath = !!m_savefullpath.GetCheck();
        m_fOSD = !!m_showosd.GetCheck();
        m_fReloaderDisabled = !m_autoreload.GetCheck();
    } else {
        m_flippic.SetCheck(m_fFlipPicture);
        m_flipsub.SetCheck(m_fFlipSubtitles);
        m_hidesub.SetCheck(m_fHideSubtitles);
        m_savefullpath.SetCheck(m_fSaveFullPath);
        m_showosd.SetCheck(m_fOSD);
        m_autoreload.SetCheck(!m_fReloaderDisabled);
        m_instupd.SetCheck(!!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_INSTANTUPDATE), TRUE));
    }
}

/* CDVSTimingPPage */

CDVSTimingPPage::CDVSTimingPPage(LPUNKNOWN pUnk, HRESULT* phr)
    : CDVSBasePPage(NAME("VSFilter Timing Property Page"), pUnk, IDD_DVSTIMINGPAGE, IDD_DVSTIMINGPAGE)
    , m_SubtitleSpeedMul(1000)
    , m_SubtitleSpeedDiv(1000)
    , m_SubtitleDelay(0)
    , m_fMediaFPSEnabled(false)
    , m_MediaFPS(25.0)
{
    BindControl(IDC_MODFPS, m_modfps);
    BindControl(IDC_FPS, m_fps);
    BindControl(IDC_SPIN5, m_subdelay);
    BindControl(IDC_SPIN6, m_subspeedmul);
    BindControl(IDC_SPIN9, m_subspeeddiv);
}

bool CDVSTimingPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case BN_CLICKED: {
                    if (LOWORD(wParam) == IDC_MODFPS) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        m_fps.EnableWindow(!!m_modfps.GetCheck());
                        return true;
                    }
                }
                break;
            }
        }
        break;
    }

    return false;
}

void CDVSTimingPPage::UpdateObjectData(bool fSave)
{
    if (fSave) {
        m_pDirectVobSub->put_SubtitleTiming(m_SubtitleDelay, m_SubtitleSpeedMul, m_SubtitleSpeedDiv);
        m_pDirectVobSub->put_MediaFPS(m_fMediaFPSEnabled, m_MediaFPS);
    } else {
        m_pDirectVobSub->get_SubtitleTiming(&m_SubtitleDelay, &m_SubtitleSpeedMul, &m_SubtitleSpeedDiv);
        m_pDirectVobSub->get_MediaFPS(&m_fMediaFPSEnabled, &m_MediaFPS);
    }
}

void CDVSTimingPPage::UpdateControlData(bool fSave)
{
    if (fSave) {
        m_fMediaFPSEnabled = !!m_modfps.GetCheck();
        CString fpsstr;
        m_fps.GetWindowText(fpsstr);
        float fps;
        if (_stscanf_s(fpsstr, _T("%f"), &fps) == 1) {
            m_MediaFPS = fps;
        }
        m_SubtitleDelay = m_subdelay.GetPos32();
        m_SubtitleSpeedMul = m_subspeedmul.GetPos32();
        m_SubtitleSpeedDiv = m_subspeeddiv.GetPos32();
    } else {
        m_modfps.SetCheck(m_fMediaFPSEnabled);
        CString fpsstr;
        fpsstr.Format(_T("%.4f"), m_MediaFPS);
        m_fps.SetWindowText(fpsstr);
        m_fps.EnableWindow(m_fMediaFPSEnabled);
        m_subdelay.SetRange32(-180 * 60 * 1000, 180 * 60 * 1000);
        m_subspeedmul.SetRange32(0, 1000000);
        m_subspeeddiv.SetRange32(1, 1000000);
        m_subdelay.SetPos32(m_SubtitleDelay);
        m_subspeedmul.SetPos32(m_SubtitleSpeedMul);
        m_subspeeddiv.SetPos32(m_SubtitleSpeedDiv);
    }
}

/* CDVSAboutPPage */

CDVSAboutPPage::CDVSAboutPPage(LPUNKNOWN lpunk, HRESULT* phr) :
    CDVSBasePPage(NAME("About Property Page"), lpunk, IDD_DVSABOUTPAGE, IDD_DVSABOUTPAGE)
{
}

bool CDVSAboutPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG: {
            SetDlgItemText(m_Dlg, IDC_VERSION, _T("VSFilter 2.41.") MAKE_STR(MPC_VERSION_REV) _T(" ") MPC_VERSION_ARCH _T("\nCopyright 2001-2013 MPC-HC Team"));
        }
        break;
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case BN_CLICKED: {
                    if (LOWORD(wParam) == IDC_HOMEPAGEBTN) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        ShellExecute(m_Dlg, _T("open"), ResStr(IDS_URL_HOMEPAGE), nullptr, nullptr, SW_SHOWNORMAL);
                        return true;
                    } else if (LOWORD(wParam) == IDC_BUGREPORTBTN) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        ShellExecute(m_Dlg, _T("open"), ResStr(IDS_URL_EMAIL), nullptr, nullptr, SW_SHOWNORMAL);
                        return true;
                    }
                }
                break;
            }
        }
        break;
    }

    return false;
}

/* CDVSZoomPPage */

CDVSZoomPPage::CDVSZoomPPage(LPUNKNOWN pUnk, HRESULT* phr) :
    CDVSBasePPage(NAME("VSFilter Zoom Property Page"), pUnk, IDD_DVSZOOMPAGE, IDD_DVSZOOMPAGE)
{
    ZeroMemory(&m_rect, sizeof(m_rect));
    BindControl(IDC_SPIN1, m_posx);
    BindControl(IDC_SPIN2, m_posy);
    BindControl(IDC_SPIN7, m_scalex);
    BindControl(IDC_SPIN8, m_scaley);
}

bool CDVSZoomPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case EN_CHANGE: {
                    if (LOWORD(wParam) == IDC_EDIT1 || LOWORD(wParam) == IDC_EDIT2
                            || LOWORD(wParam) == IDC_EDIT7 || LOWORD(wParam) == IDC_EDIT8) {
                        AFX_MANAGE_STATE(AfxGetStaticModuleState());
                        UpdateControlData(true);
                        UpdateObjectData(true);
                        return true;
                    }
                }

                break;
            }
        }
        break;
    }

    return false;
}

void CDVSZoomPPage::UpdateControlData(bool fSave)
{
    if (fSave) {
        m_rect.left = 1.0f * m_posx.GetPos32() / 100;
        m_rect.top = 1.0f * m_posy.GetPos32() / 100;
        m_rect.right = m_rect.left + 1.0f * m_scalex.GetPos32() / 100;
        m_rect.bottom = m_rect.top + 1.0f * m_scaley.GetPos32() / 100;
    } else {
        m_posx.SetRange32(-100, 100);
        m_posx.SetPos32((int)(m_rect.left * 100));
        m_posy.SetRange32(-100, 100);
        m_posy.SetPos32((int)(m_rect.top * 100));
        m_scalex.SetRange32(-300, 300);
        m_scalex.SetPos32((int)((m_rect.right - m_rect.left) * 100));
        m_scaley.SetRange32(-300, 300);
        m_scaley.SetPos32((int)((m_rect.bottom - m_rect.top) * 100));
    }
}

void CDVSZoomPPage::UpdateObjectData(bool fSave)
{
    if (fSave) {
        m_pDirectVobSub->put_ZoomRect(&m_rect);
    } else {
        m_pDirectVobSub->get_ZoomRect(&m_rect);
    }
}

// TODO: Make CDVSColorPPage and CDVSPathsPPage use an interface on DirectVobSub instead of the registry to communicate

/* CDVSColorPPage */

CDVSColorPPage::CDVSColorPPage(LPUNKNOWN pUnk, HRESULT* phr) :
    CDVSBasePPage(NAME("VSFilter Color Property Page"), pUnk, IDD_DVSCOLORPAGE, IDD_DVSCOLORPAGE)
{
    BindControl(IDC_PREFLIST, m_preflist);
    BindControl(IDC_DYNCHGLIST, m_dynchglist);
    BindControl(IDC_FORCERGBCHK, m_forcergb);

    m_fDisableInstantUpdate = true;
}

bool CDVSColorPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case LBN_DBLCLK:
                    if ((HWND)lParam == m_dynchglist.m_hWnd) {
                        int old = -1;
                        m_pDirectVobSub->get_ColorFormat(&old);
                        if (FAILED(m_pDirectVobSub->put_ColorFormat(m_dynchglist.GetCurSel()))) {
                            m_dynchglist.SetCurSel(old);
                        }

                        return true;
                    }
                    break;

                case BN_CLICKED: {
                    switch (LOWORD(wParam)) {
                        case IDC_COLORCHANGE: {
                            int old = -1;
                            m_pDirectVobSub->get_ColorFormat(&old);
                            if (FAILED(m_pDirectVobSub->put_ColorFormat(m_dynchglist.GetCurSel()))) {
                                m_dynchglist.SetCurSel(old);
                            }

                            return true;
                        }
                        case IDC_COLORUP: {
                            int sel = m_preflist.GetCurSel();
                            if (sel > 0) {
                                CString str;
                                m_preflist.GetText(sel, str);
                                int iPos = (int)m_preflist.GetItemData(sel);
                                m_preflist.DeleteString(sel);
                                sel--;
                                m_preflist.InsertString(sel, str);
                                m_preflist.SetItemData(sel, iPos);
                                m_preflist.SetCurSel(sel);
                            }

                            return true;
                        }
                        case IDC_COLORDOWN: {
                            int sel = m_preflist.GetCurSel();
                            if (sel >= 0 && sel < m_preflist.GetCount() - 1) {
                                CString str;
                                m_preflist.GetText(sel, str);
                                int iPos = (int)m_preflist.GetItemData(sel);
                                m_preflist.DeleteString(sel);
                                sel++;
                                m_preflist.InsertString(sel, str);
                                m_preflist.SetItemData(sel, iPos);
                                m_preflist.SetCurSel(sel);
                            }

                            return true;
                        }
                    }
                }
                break;
            }
        }
        break;
    }

    return false;
}

void CDVSColorPPage::UpdateObjectData(bool fSave)
{
    if (fSave) {
    } else {
    }
}

void CDVSColorPPage::UpdateControlData(bool fSave)
{
    if (fSave) {
        if ((UINT)m_preflist.GetCount() == VIHSIZE) {
            BYTE* pData = DEBUG_NEW BYTE[VIHSIZE];

            for (int i = 0; i < m_preflist.GetCount(); i++) {
                pData[i] = (BYTE)m_preflist.GetItemData(i);
            }

            theApp.WriteProfileBinary(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_COLORFORMATS), pData, VIHSIZE);

            delete [] pData;
        } else {
            ASSERT(0);
        }

        theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_FORCERGB), !!m_forcergb.GetCheck());
    } else {
        m_preflist.ResetContent();
        m_dynchglist.ResetContent();

        BYTE* pData = nullptr;
        UINT nSize;

        if (!theApp.GetProfileBinary(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_COLORFORMATS), &pData, &nSize)
                || !pData || nSize != VIHSIZE) {
            SAFE_DELETE_ARRAY(pData);

            nSize = VIHSIZE;
            pData = DEBUG_NEW BYTE[VIHSIZE];
            for (size_t i = 0; i < VIHSIZE; i++) {
                pData[i] = (BYTE)i;
            }
        }

        if (pData) {
            for (int i = 0; i < (int)nSize; i++) {
                m_dynchglist.AddString(VIH2String(pData[i]));
                m_dynchglist.SetItemData(i, pData[i]);
                m_preflist.AddString(VIH2String(pData[i]));
                m_preflist.SetItemData(i, pData[i]);
            }

            int iPosition = -1;
            m_pDirectVobSub->get_ColorFormat(&iPosition);
            m_dynchglist.SetCurSel(iPosition);

            delete [] pData;
        }

        m_forcergb.SetCheck(theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_FORCERGB), 0) ? BST_CHECKED : BST_UNCHECKED);
    }
}

/* CDVSPathsPPage */

CDVSPathsPPage::CDVSPathsPPage(LPUNKNOWN pUnk, HRESULT* phr) :
    CDVSBasePPage(NAME("VSFilter Paths Property Page"), pUnk, IDD_DVSPATHSPAGE, IDD_DVSPATHSPAGE)
{
    BindControl(IDC_PATHLIST, m_pathlist);
    BindControl(IDC_PATHEDIT, m_path);
    BindControl(IDC_BROWSE, m_browse);
    BindControl(IDC_REMOVE, m_remove);
    BindControl(IDC_ADD, m_add);

    m_fDisableInstantUpdate = true;
}

bool CDVSPathsPPage::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case LBN_SELCHANGE:
                    if ((HWND)lParam == m_pathlist.m_hWnd) {
                        int i = m_pathlist.GetCurSel();
                        m_remove.EnableWindow(i >= 3 ? TRUE : FALSE);
                        if (i >= 0) {
                            CString path;
                            m_pathlist.GetText(i, path);
                            m_path.SetWindowText(path);
                        }
                        return true;
                    }
                    break;

                case LBN_SELCANCEL:
                    if ((HWND)lParam == m_pathlist.m_hWnd) {
                        m_remove.EnableWindow(FALSE);
                        return true;
                    }
                    break;

                case BN_CLICKED: {
                    switch (LOWORD(wParam)) {
                        case IDC_BROWSE: {
                            TCHAR pathbuff[MAX_PATH];

                            BROWSEINFO bi;
                            bi.hwndOwner = m_Dlg;
                            bi.pidlRoot = nullptr;
                            bi.pszDisplayName = pathbuff;
                            bi.lpszTitle = _T("");
                            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE | BIF_USENEWUI;
                            bi.lpfn = nullptr;
                            bi.lParam = 0;
                            bi.iImage = 0;

                            PIDLIST_ABSOLUTE iil = SHBrowseForFolder(&bi);
                            if (iil) {
                                SHGetPathFromIDList(iil, pathbuff);
                                m_path.SetWindowText(pathbuff);
                            }

                            return true;
                        }
                        break;

                        case IDC_REMOVE: {
                            int i = m_pathlist.GetCurSel();
                            if (i >= 0) {
                                m_pathlist.DeleteString(i);
                                i = std::min(i, m_pathlist.GetCount() - 1);
                                if (i >= 0 && m_pathlist.GetCount() > 0) {
                                    m_pathlist.SetCurSel(i);
                                    m_remove.EnableWindow(i >= 3 ? TRUE : FALSE);
                                }
                            }

                            return true;
                        }
                        break;

                        case IDC_ADD: {
                            CString path;
                            m_path.GetWindowText(path);
                            if (!path.IsEmpty() && m_pathlist.FindString(-1, path) < 0) {
                                m_pathlist.AddString(path);
                            }

                            return true;
                        }
                        break;
                    }
                }
                break;
            }
        }
        break;
    }

    return false;
}

void CDVSPathsPPage::UpdateObjectData(bool fSave)
{
    if (fSave) {
        CString chk(_T("123456789")), path, tmp;
        int i = 0;
        do {
            tmp.Format(IDS_RP_PATH, i++);
            path = theApp.GetProfileString(ResStr(IDS_R_DEFTEXTPATHES), tmp, chk);
            if (path != chk) {
                theApp.WriteProfileString(ResStr(IDS_R_DEFTEXTPATHES), tmp, _T(""));
            }
        } while (path != chk);

        for (i = 0; i < m_paths.GetSize(); i++) {
            tmp.Format(IDS_RP_PATH, i);
            theApp.WriteProfileString(ResStr(IDS_R_DEFTEXTPATHES), tmp, m_paths[i]);
        }
    } else {
        CString chk(_T("123456789")), path, tmp;
        int i = 0;
        do {
            if (!path.IsEmpty()) {
                m_paths.Add(path);
            }
            tmp.Format(IDS_RP_PATH, i++);
            path = theApp.GetProfileString(ResStr(IDS_R_DEFTEXTPATHES), tmp, chk);
        } while (path != chk);
    }
}

void CDVSPathsPPage::UpdateControlData(bool fSave)
{
    if (fSave) {
        m_paths.RemoveAll();
        for (int i = 0; i < m_pathlist.GetCount(); i++) {
            CString path;
            m_pathlist.GetText(i, path);
            m_paths.Add(path);
        }
    } else {
        m_pathlist.ResetContent();
        for (ptrdiff_t i = 0; i < m_paths.GetSize(); i++) {
            m_pathlist.AddString(m_paths[i]);
        }

        m_remove.EnableWindow(FALSE);
        m_add.EnableWindow(TRUE);
    }
}
