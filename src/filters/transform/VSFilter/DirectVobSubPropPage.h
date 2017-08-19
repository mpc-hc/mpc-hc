/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2017 see Authors.txt
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

#pragma once

#include <afxcmn.h>
#include "IDirectVobSub.h"

class CDVSBasePPage : public CBasePropertyPage
{
public:
    // we have to override these to use external, resource-only dlls
    STDMETHODIMP GetPageInfo(LPPROPPAGEINFO pPageInfo);
    STDMETHODIMP Activate(HWND hwndParent, LPCRECT pRect, BOOL fModal);

protected:
    CComQIPtr<IDirectVobSub3> m_pDirectVobSub;

    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return false; }
    virtual void UpdateObjectData(bool fSave) {}
    virtual void UpdateControlData(bool fSave) {}

protected:
    CDVSBasePPage(LPCTSTR pName, LPUNKNOWN lpunk, int DialogId, int TitleId);

    bool m_fDisableInstantUpdate;

private:
    BOOL m_bIsInitialized;

    HRESULT OnConnect(IUnknown* pUnknown), OnDisconnect(), OnActivate(), OnDeactivate(), OnApplyChanges();
    INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    bool m_fAttached;
    void AttachControls(), DetachControls();

    CMap<UINT, UINT&, CWnd*, CWnd*> m_controls;

protected:
    void BindControl(UINT id, CWnd& control);
};

class __declspec(uuid("60765CF5-01C2-4ee7-A44B-C791CF25FEA0"))
    CDVSMainPPage : public CDVSBasePPage
{
    void FreeLangs(), AllocLangs(int nLangs);

    WCHAR m_fn[MAX_PATH];
    int m_iSelectedLanguage, m_nLangs;
    WCHAR** m_ppLangs;
    bool m_fOverridePlacement;
    int m_PlacementXperc, m_PlacementYperc;
    STSStyle m_defStyle;
    bool m_fOnlyShowForcedVobSubs;
    CSimpleTextSubtitle::EPARCompensationType m_ePARCompensationType;

    CEdit m_fnedit;
    CComboBox m_langs;
    CButton m_oplacement;
    CSpinButtonCtrl m_subposx, m_subposy;
    CButton m_font, m_forcedsubs;
    CButton m_AutoPARCompensation;
    CComboBox m_PARCombo;

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool fSave);
    virtual void UpdateObjectData(bool fSave);

public:
    CDVSMainPPage(LPUNKNOWN lpunk, HRESULT* phr);
    virtual ~CDVSMainPPage();
};

class __declspec(uuid("0180E49C-13BF-46db-9AFD-9F52292E1C22"))
    CDVSGeneralPPage : public CDVSBasePPage
{
    int m_HorExt, m_VerExt, m_ResX2, m_ResX2minw, m_ResX2minh;
    int m_LoadLevel;
    bool m_fExternalLoad, m_fWebLoad, m_fEmbeddedLoad;

    CComboBox m_verext;
    CButton m_mod32fix;
    CComboBox m_resx2;
    CSpinButtonCtrl m_resx2w, m_resx2h;
    CComboBox m_load;
    CButton m_extload, m_webload, m_embload;

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool fSave);
    virtual void UpdateObjectData(bool fSave);

public:
    CDVSGeneralPPage(LPUNKNOWN lpunk, HRESULT* phr);
};

class __declspec(uuid("485CACED-6741-457A-84A2-41FD70C28E3E"))
    CDVSSubpicQueuePPage : public CDVSBasePPage
{
    unsigned int m_nSubPictToBuffer;
    bool m_bDisableSubtitleAnimation, m_bAllowDroppingSubpic;
    int m_nRenderAtWhenAnimationIsDisabled, m_nAnimationRate;

    CButton m_disableSubtitleAnimationButton, m_allowDroppingSubpicButton;
    CSpinButtonCtrl m_subPictToBufferCtrl, m_renderAtCtrl, m_animationRateCtrl;

    void UpdateEditControls(bool bDisableSubtitleAnimation);
    void UpdateAllowDroppingSubpicControl(bool bBufferingEnabled);

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool bSave);
    virtual void UpdateObjectData(bool bSave);

public:
    CDVSSubpicQueuePPage(LPUNKNOWN lpunk, HRESULT* phr);
};

class __declspec(uuid("A8B25C0E-0894-4531-B668-AB1599FAF7F6"))
    CDVSMiscPPage : public CDVSBasePPage
{
    bool m_fFlipPicture, m_fFlipSubtitles, m_fHideSubtitles, m_fOSD, m_fReloaderDisabled, m_fSaveFullPath;

    CButton m_flippic, m_flipsub, m_hidesub, m_showosd, m_autoreload, m_savefullpath, m_instupd;

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool fSave);
    virtual void UpdateObjectData(bool fSave);

public:
    CDVSMiscPPage(LPUNKNOWN lpunk, HRESULT* phr);
};

class __declspec(uuid("ACE4747B-35BD-4e97-9DD7-1D4245B0695C"))
    CDVSTimingPPage : public CDVSBasePPage
{
    int m_SubtitleSpeedMul, m_SubtitleSpeedDiv, m_SubtitleDelay;
    bool m_fMediaFPSEnabled;
    double m_MediaFPS;

    CButton m_modfps;
    CEdit m_fps;
    CSpinButtonCtrl m_subdelay, m_subspeedmul, m_subspeeddiv;

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool fSave);
    virtual void UpdateObjectData(bool fSave);

public:
    CDVSTimingPPage(LPUNKNOWN lpunk, HRESULT* phr);
};

class __declspec(uuid("F544E0F5-CA3C-47ea-A64D-35FCF1602396"))
    CDVSAboutPPage : public CDVSBasePPage
{
protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    CDVSAboutPPage(LPUNKNOWN lpunk, HRESULT* phr);
};

class __declspec(uuid("525F116F-04AD-40a2-AE2F-A0C4E1AFEF98"))
    CDVSZoomPPage : public CDVSBasePPage
{
    NORMALIZEDRECT m_rect;

    CSpinButtonCtrl m_posx, m_posy, m_scalex, m_scaley;

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool fSave);
    virtual void UpdateObjectData(bool fSave);

public:
    CDVSZoomPPage(LPUNKNOWN lpunk, HRESULT* phr);
};

class __declspec(uuid("C2D6D98F-09CA-4524-AF64-1049B5665C9C"))
    CDVSColorPPage : public CDVSBasePPage
{
    CListBox m_preflist, m_dynchglist;
    CButton m_forcergb;

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool fSave);
    virtual void UpdateObjectData(bool fSave);

public:
    CDVSColorPPage(LPUNKNOWN lpunk, HRESULT* phr);
};

class __declspec(uuid("CE77C59C-CFD2-429f-868C-8B04D23F94CA"))
    CDVSPathsPPage : public CDVSBasePPage
{
    CStringArray m_paths;

    CListBox m_pathlist;
    CEdit m_path;
    CButton m_browse, m_remove, m_add;

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void UpdateControlData(bool fSave);
    virtual void UpdateObjectData(bool fSave);

public:
    CDVSPathsPPage(LPUNKNOWN lpunk, HRESULT* phr);
};
