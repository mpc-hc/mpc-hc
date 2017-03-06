/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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
#include "DirectVobSub.h"
#include "VSFilter.h"

CDirectVobSub::CDirectVobSub()
    : m_iSelectedLanguage(0)
    , m_MediaFPS(25.0)
    , m_fForced(false)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_iSelectedLanguage = 0;
    m_fHideSubtitles = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_HIDE), FALSE);
    m_subPicQueueSettings.nSize = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SUBPICTTOBUFFER), 10);
    m_subPicQueueSettings.bDisableSubtitleAnimation = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_DISABLESUBANIM), FALSE);
    m_subPicQueueSettings.nRenderAtWhenAnimationIsDisabled = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RENDERATWITHOUTANIM), 50);
    m_subPicQueueSettings.nAnimationRate = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_ANIMATIONRATE), 100);
    m_subPicQueueSettings.bAllowDroppingSubpic = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_ALLOWDROPPINGSUBPIC), TRUE);
    m_fOverridePlacement = !!theApp.GetProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_OVERRIDEPLACEMENT), FALSE);
    m_PlacementXperc = theApp.GetProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_XPERC), 50);
    m_PlacementYperc = theApp.GetProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_YPERC), 90);
    m_fBufferVobSub = !!theApp.GetProfileInt(ResStr(IDS_R_VOBSUB), ResStr(IDS_RV_BUFFER), TRUE);
    m_fOnlyShowForcedVobSubs = !!theApp.GetProfileInt(ResStr(IDS_R_VOBSUB), ResStr(IDS_RV_ONLYSHOWFORCEDSUBS), FALSE);
    m_fPolygonize = !!theApp.GetProfileInt(ResStr(IDS_R_VOBSUB), ResStr(IDS_RV_POLYGONIZE), FALSE);
    m_defStyle <<= theApp.GetProfileString(ResStr(IDS_R_TEXT), ResStr(IDS_RT_STYLE));
    m_fFlipPicture = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_FLIPPICTURE), FALSE);
    m_fFlipSubtitles = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_FLIPSUBTITLES), FALSE);
    m_fOSD = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SHOWOSDSTATS), FALSE);
    m_fSaveFullPath = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SAVEFULLPATH), FALSE);
    m_nReloaderDisableCount = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_DISABLERELOADER), FALSE) ? 1 : 0;
    m_SubtitleDelay = theApp.GetProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_SUBTITLEDELAY), 0);
    m_SubtitleSpeedMul = theApp.GetProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_SUBTITLESPEEDMUL), 1000);
    m_SubtitleSpeedDiv = theApp.GetProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_SUBTITLESPEEDDIV), 1000);
    m_fMediaFPSEnabled = !!theApp.GetProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_MEDIAFPSENABLED), FALSE);
    m_ePARCompensationType = static_cast<CSimpleTextSubtitle::EPARCompensationType>(theApp.GetProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_AUTOPARCOMPENSATION), 0));

    int gcd = GCD(m_SubtitleSpeedMul, m_SubtitleSpeedDiv);
    m_SubtitleSpeedNormalizedMul = m_SubtitleSpeedMul / gcd;
    m_SubtitleSpeedNormalizedDiv = m_SubtitleSpeedDiv / gcd;

    BYTE* pData = nullptr;
    UINT nSize;
    if (theApp.GetProfileBinary(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_MEDIAFPS), &pData, &nSize) && pData) {
        if (nSize == sizeof(m_MediaFPS)) {
            memcpy(&m_MediaFPS, pData, sizeof(m_MediaFPS));
        }
        delete [] pData;
    }

    m_ZoomRect.left = m_ZoomRect.top = 0;
    m_ZoomRect.right = m_ZoomRect.bottom = 1;
}

CDirectVobSub::~CDirectVobSub()
{
    CAutoLock cAutoLock(&m_propsLock);
}

STDMETHODIMP CDirectVobSub::get_FileName(WCHAR* fn)
{
    CAutoLock cAutoLock(&m_propsLock);

    CheckPointer(fn, E_POINTER);

    wcscpy_s(fn, MAX_PATH, m_FileName);

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_FileName(WCHAR* fn)
{
    CAutoLock cAutoLock(&m_propsLock);

    CheckPointer(fn, E_POINTER);

    CString tmp = fn;
    if (!m_FileName.Left(m_FileName.ReverseFind('.') + 1).CompareNoCase(tmp.Left(tmp.ReverseFind('.') + 1))) {
        return S_FALSE;
    }

    m_FileName = fn;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_LanguageCount(int* nLangs)
{
    CAutoLock cAutoLock(&m_propsLock);

    return nLangs ? *nLangs = 0, S_OK : E_POINTER;
}

STDMETHODIMP CDirectVobSub::get_LanguageName(int iLanguage, WCHAR** ppName)
{
    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_SelectedLanguage(int* iSelected)
{
    CAutoLock cAutoLock(&m_propsLock);

    return iSelected ? *iSelected = m_iSelectedLanguage, S_OK : E_POINTER;
}

STDMETHODIMP CDirectVobSub::put_SelectedLanguage(int iSelected)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_iSelectedLanguage == iSelected) {
        return S_FALSE;
    }

    int nCount;
    if (FAILED(get_LanguageCount(&nCount))
            || iSelected < 0
            || iSelected >= nCount) {
        return E_FAIL;
    }

    m_iSelectedLanguage = iSelected;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_HideSubtitles(bool* fHideSubtitles)
{
    CAutoLock cAutoLock(&m_propsLock);

    return fHideSubtitles ? *fHideSubtitles = m_fHideSubtitles, S_OK : E_POINTER;
}

STDMETHODIMP CDirectVobSub::put_HideSubtitles(bool fHideSubtitles)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_fHideSubtitles == fHideSubtitles) {
        return S_FALSE;
    }

    m_fHideSubtitles = fHideSubtitles;

    return S_OK;
}

// deprecated
STDMETHODIMP CDirectVobSub::get_PreBuffering(bool* fDoPreBuffering)
{
    CAutoLock cAutoLock(&m_propsLock);

    return fDoPreBuffering ? *fDoPreBuffering = (m_subPicQueueSettings.nSize > 0), S_OK : E_POINTER;
}

// deprecated
STDMETHODIMP CDirectVobSub::put_PreBuffering(bool fDoPreBuffering)
{
    CAutoLock cAutoLock(&m_propsLock);

    if ((m_subPicQueueSettings.nSize > 0) == fDoPreBuffering) {
        return S_FALSE;
    }

    m_subPicQueueSettings.nSize = fDoPreBuffering ? 10 : 0; // 10 is the default value for SubPictToBuffer

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_SubPictToBuffer(unsigned int* uSubPictToBuffer)
{
    CAutoLock cAutoLock(&m_propsLock);

    return uSubPictToBuffer ? *uSubPictToBuffer = (unsigned int)m_subPicQueueSettings.nSize, S_OK : E_POINTER;
}

STDMETHODIMP CDirectVobSub::put_SubPictToBuffer(unsigned int uSubPictToBuffer)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_subPicQueueSettings.nSize == (int)uSubPictToBuffer) {
        return S_FALSE;
    }

    m_subPicQueueSettings.nSize = (int)uSubPictToBuffer;

    return S_OK;
}

// deprecated
STDMETHODIMP CDirectVobSub::get_AnimWhenBuffering(bool* fAnimWhenBuffering)
{
    CAutoLock cAutoLock(&m_propsLock);

    return fAnimWhenBuffering ? *fAnimWhenBuffering = !m_subPicQueueSettings.bDisableSubtitleAnimation, S_OK : E_POINTER;
}

// deprecated
STDMETHODIMP CDirectVobSub::put_AnimWhenBuffering(bool fAnimWhenBuffering)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_subPicQueueSettings.bDisableSubtitleAnimation == !fAnimWhenBuffering) {
        return S_FALSE;
    }

    m_subPicQueueSettings.bDisableSubtitleAnimation = !fAnimWhenBuffering;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_Placement(bool* fOverridePlacement, int* xperc, int* yperc)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (fOverridePlacement) {
        *fOverridePlacement = m_fOverridePlacement;
    }
    if (xperc) {
        *xperc = m_PlacementXperc;
    }
    if (yperc) {
        *yperc = m_PlacementYperc;
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_Placement(bool fOverridePlacement, int xperc, int yperc)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_fOverridePlacement == fOverridePlacement && m_PlacementXperc == xperc && m_PlacementYperc == yperc) {
        return S_FALSE;
    }

    m_fOverridePlacement = fOverridePlacement;
    m_PlacementXperc = xperc;
    m_PlacementYperc = yperc;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_VobSubSettings(bool* fBuffer, bool* fOnlyShowForcedSubs, bool* fPolygonize)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (fBuffer) {
        *fBuffer = m_fBufferVobSub;
    }
    if (fOnlyShowForcedSubs) {
        *fOnlyShowForcedSubs = m_fOnlyShowForcedVobSubs;
    }
    if (fPolygonize) {
        *fPolygonize = m_fPolygonize;
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_VobSubSettings(bool fBuffer, bool fOnlyShowForcedSubs, bool fPolygonize)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_fBufferVobSub == fBuffer && m_fOnlyShowForcedVobSubs == fOnlyShowForcedSubs && m_fPolygonize == fPolygonize) {
        return S_FALSE;
    }

    m_fBufferVobSub = fBuffer;
    m_fOnlyShowForcedVobSubs = fOnlyShowForcedSubs;
    m_fPolygonize = fPolygonize;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_TextSettings(void* lf, int lflen, COLORREF* color, bool* fShadow, bool* fOutline, bool* fAdvancedRenderer)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (lf) {
        if (lflen == sizeof(LOGFONTA)) {
            strncpy_s(((LOGFONTA*)lf)->lfFaceName, LF_FACESIZE, CStringA(m_defStyle.fontName), _TRUNCATE);
        } else if (lflen == sizeof(LOGFONTW)) {
            wcsncpy_s(((LOGFONTW*)lf)->lfFaceName, LF_FACESIZE, CStringW(m_defStyle.fontName), _TRUNCATE);
        } else {
            return E_INVALIDARG;
        }

        ((LOGFONT*)lf)->lfCharSet = (BYTE)m_defStyle.charSet;
        ((LOGFONT*)lf)->lfItalic = (BYTE)m_defStyle.fItalic;
        ((LOGFONT*)lf)->lfHeight = (LONG)m_defStyle.fontSize;
        ((LOGFONT*)lf)->lfWeight = m_defStyle.fontWeight;
        ((LOGFONT*)lf)->lfStrikeOut = (BYTE)m_defStyle.fStrikeOut;
        ((LOGFONT*)lf)->lfUnderline = (BYTE)m_defStyle.fUnderline;
    }

    if (color) {
        *color = m_defStyle.colors[0];
    }
    if (fShadow) {
        *fShadow = (m_defStyle.shadowDepthX != 0) || (m_defStyle.shadowDepthY != 0);
    }
    if (fOutline) {
        *fOutline = (m_defStyle.outlineWidthX + m_defStyle.outlineWidthY) > 0;
    }
    if (fAdvancedRenderer) {
        *fAdvancedRenderer = false; // Remove me
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_TextSettings(void* lf, int lflen, COLORREF color, bool fShadow, bool fOutline, bool fAdvancedRenderer)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (lf) {
        if (lflen == sizeof(LOGFONTA)) {
            m_defStyle.fontName = ((LOGFONTA*)lf)->lfFaceName;
        } else if (lflen == sizeof(LOGFONTW)) {
            m_defStyle.fontName = ((LOGFONTW*)lf)->lfFaceName;
        } else {
            return E_INVALIDARG;
        }

        m_defStyle.charSet = ((LOGFONT*)lf)->lfCharSet;
        m_defStyle.fItalic = !!((LOGFONT*)lf)->lfItalic;
        m_defStyle.fontSize = ((LOGFONT*)lf)->lfHeight;
        m_defStyle.fontWeight = ((LOGFONT*)lf)->lfWeight;
        m_defStyle.fStrikeOut = !!((LOGFONT*)lf)->lfStrikeOut;
        m_defStyle.fUnderline = !!((LOGFONT*)lf)->lfUnderline;

        if (m_defStyle.fontSize < 0) {
            HDC hdc = ::GetDC(nullptr);
            m_defStyle.fontSize = -m_defStyle.fontSize * 72 / GetDeviceCaps(hdc, LOGPIXELSY);
            ::ReleaseDC(nullptr, hdc);
        }

    }

    m_defStyle.colors[0] = color;
    m_defStyle.shadowDepthX = m_defStyle.shadowDepthY = fShadow ? 2 : 0;
    m_defStyle.outlineWidthX = m_defStyle.outlineWidthY = fOutline ? 2 : 0;

    return S_OK;

}

STDMETHODIMP CDirectVobSub::get_Flip(bool* fPicture, bool* fSubtitles)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (fPicture) {
        *fPicture = m_fFlipPicture;
    }
    if (fSubtitles) {
        *fSubtitles = m_fFlipSubtitles;
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_Flip(bool fPicture, bool fSubtitles)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_fFlipPicture == fPicture && m_fFlipSubtitles == fSubtitles) {
        return S_FALSE;
    }

    m_fFlipPicture = fPicture;
    m_fFlipSubtitles = fSubtitles;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_OSD(bool* fOSD)
{
    CAutoLock cAutoLock(&m_propsLock);

    return fOSD ? *fOSD = m_fOSD, S_OK : E_POINTER;
}

STDMETHODIMP CDirectVobSub::put_OSD(bool fOSD)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_fOSD == fOSD) {
        return S_FALSE;
    }

    m_fOSD = fOSD;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_SaveFullPath(bool* fSaveFullPath)
{
    CAutoLock cAutoLock(&m_propsLock);

    return fSaveFullPath ? *fSaveFullPath = m_fSaveFullPath, S_OK : E_POINTER;
}

STDMETHODIMP CDirectVobSub::put_SaveFullPath(bool fSaveFullPath)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_fSaveFullPath == fSaveFullPath) {
        return S_FALSE;
    }

    m_fSaveFullPath = fSaveFullPath;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_SubtitleTiming(int* delay, int* speedmul, int* speeddiv)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (delay) {
        *delay = m_SubtitleDelay;
    }
    if (speedmul) {
        *speedmul = m_SubtitleSpeedMul;
    }
    if (speeddiv) {
        *speeddiv = m_SubtitleSpeedDiv;
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_SubtitleTiming(int delay, int speedmul, int speeddiv)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_SubtitleDelay == delay && m_SubtitleSpeedMul == speedmul && m_SubtitleSpeedDiv == speeddiv) {
        return S_FALSE;
    }

    m_SubtitleDelay = delay;
    m_SubtitleSpeedMul = speedmul;
    if (speeddiv > 0) {
        m_SubtitleSpeedDiv = speeddiv;
    }

    int gcd = GCD(m_SubtitleSpeedMul, m_SubtitleSpeedDiv);
    m_SubtitleSpeedNormalizedMul = m_SubtitleSpeedMul / gcd;
    m_SubtitleSpeedNormalizedDiv = m_SubtitleSpeedDiv / gcd;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_MediaFPS(bool* fEnabled, double* fps)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (fEnabled) {
        *fEnabled = m_fMediaFPSEnabled;
    }
    if (fps) {
        *fps = m_MediaFPS;
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_MediaFPS(bool fEnabled, double fps)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_fMediaFPSEnabled == fEnabled && m_MediaFPS == fps) {
        return S_FALSE;
    }

    m_fMediaFPSEnabled = fEnabled;
    if (fps > 0) {
        m_MediaFPS = fps;
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_ZoomRect(NORMALIZEDRECT* rect)
{
    CAutoLock cAutoLock(&m_propsLock);

    CheckPointer(rect, E_POINTER);

    *rect = m_ZoomRect;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_ZoomRect(NORMALIZEDRECT* rect)
{
    CAutoLock cAutoLock(&m_propsLock);

    CheckPointer(rect, E_POINTER);

    if (!memcmp(&m_ZoomRect, rect, sizeof(m_ZoomRect))) {
        return S_FALSE;
    }

    m_ZoomRect = *rect;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::UpdateRegistry()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(&m_propsLock);

    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_HIDE), m_fHideSubtitles);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SUBPICTTOBUFFER), m_subPicQueueSettings.nSize);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_DISABLESUBANIM), m_subPicQueueSettings.bDisableSubtitleAnimation);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RENDERATWITHOUTANIM), m_subPicQueueSettings.nRenderAtWhenAnimationIsDisabled);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_ANIMATIONRATE), m_subPicQueueSettings.nAnimationRate);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_ALLOWDROPPINGSUBPIC), m_subPicQueueSettings.bAllowDroppingSubpic);
    theApp.WriteProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_OVERRIDEPLACEMENT), m_fOverridePlacement);
    theApp.WriteProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_XPERC), m_PlacementXperc);
    theApp.WriteProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_YPERC), m_PlacementYperc);
    theApp.WriteProfileInt(ResStr(IDS_R_VOBSUB), ResStr(IDS_RV_BUFFER), m_fBufferVobSub);
    theApp.WriteProfileInt(ResStr(IDS_R_VOBSUB), ResStr(IDS_RV_ONLYSHOWFORCEDSUBS), m_fOnlyShowForcedVobSubs);
    theApp.WriteProfileInt(ResStr(IDS_R_VOBSUB), ResStr(IDS_RV_POLYGONIZE), m_fPolygonize);
    CString style;
    theApp.WriteProfileString(ResStr(IDS_R_TEXT), ResStr(IDS_RT_STYLE), style <<= m_defStyle);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_FLIPPICTURE), m_fFlipPicture);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_FLIPSUBTITLES), m_fFlipSubtitles);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SHOWOSDSTATS), m_fOSD);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_SAVEFULLPATH), m_fSaveFullPath);
    theApp.WriteProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_SUBTITLEDELAY), m_SubtitleDelay);
    theApp.WriteProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_SUBTITLESPEEDMUL), m_SubtitleSpeedMul);
    theApp.WriteProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_SUBTITLESPEEDDIV), m_SubtitleSpeedDiv);
    theApp.WriteProfileInt(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_MEDIAFPSENABLED), m_fMediaFPSEnabled);
    theApp.WriteProfileBinary(ResStr(IDS_R_TIMING), ResStr(IDS_RTM_MEDIAFPS), (BYTE*)&m_MediaFPS, sizeof(m_MediaFPS));
    theApp.WriteProfileInt(ResStr(IDS_R_TEXT), ResStr(IDS_RT_AUTOPARCOMPENSATION), m_ePARCompensationType);

    return S_OK;
}

STDMETHODIMP CDirectVobSub::HasConfigDialog(int iSelected)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDirectVobSub::ShowConfigDialog(int iSelected, HWND hWndParent)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDirectVobSub::IsSubtitleReloaderLocked(bool* fLocked)
{
    CAutoLock cAutoLock(&m_propsLock);

    CheckPointer(fLocked, E_POINTER);

    bool fDisabled;
    get_SubtitleReloader(&fDisabled);

    *fLocked = fDisabled || m_nReloaderDisableCount > 0;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::LockSubtitleReloader(bool fLock)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (fLock) {
        m_nReloaderDisableCount++;
    } else {
        m_nReloaderDisableCount--;
    }

    ASSERT(m_nReloaderDisableCount >= 0);
    if (m_nReloaderDisableCount < 0) {
        m_nReloaderDisableCount = 0;
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_SubtitleReloader(bool* fDisabled)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(&m_propsLock);

    if (fDisabled) {
        *fDisabled = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_DISABLERELOADER), FALSE);
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_SubtitleReloader(bool fDisable)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(&m_propsLock);

    bool b;
    get_SubtitleReloader(&b);
    if (b == fDisable) {
        return S_FALSE;
    }

    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_DISABLERELOADER), fDisable);

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_ExtendPicture(int* horizontal, int* vertical, int* resx2, int* resx2minw, int* resx2minh)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(&m_propsLock);

    if (horizontal) {
        *horizontal = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_MOD32FIX), 0) & 1;
    }
    if (vertical) {
        *vertical = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_EXTPIC), 0);
    }
    if (resx2) {
        *resx2 = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RESX2), 2) & 3;
    }
    if (resx2minw) {
        *resx2minw = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RESX2MINW), 384);
    }
    if (resx2minh) {
        *resx2minh = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RESX2MINH), 288);
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_ExtendPicture(int horizontal, int vertical, int resx2, int resx2minw, int resx2minh)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(&m_propsLock);

    int i[5];
    get_ExtendPicture(i, i + 1, i + 2, i + 3, i + 4);
    if (i[0] == horizontal && i[1] == vertical && i[2] == resx2 && i[3] == resx2minw && i[4] == resx2minh) {
        return S_FALSE;
    }

    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_MOD32FIX), horizontal & 1);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_EXTPIC), vertical);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RESX2), resx2 & 3);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RESX2MINW), resx2minw);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_RESX2MINH), resx2minh);

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_LoadSettings(int* level, bool* fExternalLoad, bool* fWebLoad, bool* fEmbeddedLoad)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(&m_propsLock);

    if (level) {
        *level = theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_LOADLEVEL), 0) & 3;
    }
    if (fExternalLoad) {
        *fExternalLoad = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_EXTERNALLOAD), TRUE);
    }
    if (fWebLoad) {
        *fWebLoad = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_WEBLOAD), FALSE);
    }
    if (fEmbeddedLoad) {
        *fEmbeddedLoad = !!theApp.GetProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_EMBEDDEDLOAD), TRUE);
    }

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_LoadSettings(int level, bool fExternalLoad, bool fWebLoad, bool fEmbeddedLoad)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CAutoLock cAutoLock(&m_propsLock);

    int i;
    bool b[3];
    get_LoadSettings(&i, b, b + 1, b + 2);
    if (i == level && b[0] == fExternalLoad && b[1] == fWebLoad && b[2] == fEmbeddedLoad) {
        return S_FALSE;
    }

    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_LOADLEVEL), level & 3);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_EXTERNALLOAD), fExternalLoad);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_WEBLOAD), fWebLoad);
    theApp.WriteProfileInt(ResStr(IDS_R_GENERAL), ResStr(IDS_RG_EMBEDDEDLOAD), fEmbeddedLoad);

    return S_OK;
}

// IDirectVobSub2

STDMETHODIMP CDirectVobSub::AdviseSubClock(ISubClock* pSubClock)
{
    m_pSubClock = pSubClock;
    return S_OK;
}

STDMETHODIMP_(bool) CDirectVobSub::get_Forced()
{
    return m_fForced;
}

STDMETHODIMP CDirectVobSub::put_Forced(bool fForced)
{
    m_fForced = fForced;
    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_TextSettings(STSStyle* pDefStyle)
{
    CheckPointer(pDefStyle, E_POINTER);

    CAutoLock cAutoLock(&m_propsLock);

    *pDefStyle = m_defStyle;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_TextSettings(STSStyle* pDefStyle)
{
    CheckPointer(pDefStyle, E_POINTER);

    CAutoLock cAutoLock(&m_propsLock);

    if (m_defStyle == *pDefStyle) {
        return S_FALSE;
    }

    m_defStyle = *pDefStyle;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::get_AspectRatioSettings(CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType)
{
    CheckPointer(ePARCompensationType, E_POINTER);

    CAutoLock cAutoLock(&m_propsLock);

    *ePARCompensationType = m_ePARCompensationType;

    return S_OK;
}

STDMETHODIMP CDirectVobSub::put_AspectRatioSettings(CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType)
{
    CheckPointer(ePARCompensationType, E_POINTER);

    CAutoLock cAutoLock(&m_propsLock);

    if (m_ePARCompensationType == *ePARCompensationType) {
        return S_FALSE;
    }

    m_ePARCompensationType = *ePARCompensationType;

    return S_OK;
}

STDMETHODIMP_(bool) CDirectVobSub::get_DisableSubtitleAnimation()
{
    CAutoLock cAutoLock(&m_propsLock);

    return m_subPicQueueSettings.bDisableSubtitleAnimation;
}

STDMETHODIMP CDirectVobSub::put_DisableSubtitleAnimation(bool bDisableSubtitleAnimation)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_subPicQueueSettings.bDisableSubtitleAnimation == bDisableSubtitleAnimation) {
        return S_FALSE;
    }

    m_subPicQueueSettings.bDisableSubtitleAnimation = bDisableSubtitleAnimation;

    return S_OK;
}

STDMETHODIMP_(int) CDirectVobSub::get_RenderAtWhenAnimationIsDisabled()
{
    CAutoLock cAutoLock(&m_propsLock);

    return m_subPicQueueSettings.nRenderAtWhenAnimationIsDisabled;
}

STDMETHODIMP CDirectVobSub::put_RenderAtWhenAnimationIsDisabled(int nRenderAtWhenAnimationIsDisabled)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_subPicQueueSettings.nRenderAtWhenAnimationIsDisabled == nRenderAtWhenAnimationIsDisabled) {
        return S_FALSE;
    }

    m_subPicQueueSettings.nRenderAtWhenAnimationIsDisabled = nRenderAtWhenAnimationIsDisabled;

    return S_OK;
}

STDMETHODIMP_(int) CDirectVobSub::get_AnimationRate()
{
    CAutoLock cAutoLock(&m_propsLock);

    return m_subPicQueueSettings.nAnimationRate;
}

STDMETHODIMP CDirectVobSub::put_AnimationRate(int nAnimationRate)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_subPicQueueSettings.nAnimationRate == nAnimationRate) {
        return S_FALSE;
    }

    m_subPicQueueSettings.nAnimationRate = nAnimationRate;

    return S_OK;
}

STDMETHODIMP_(bool) CDirectVobSub::get_AllowDroppingSubpic()
{
    CAutoLock cAutoLock(&m_propsLock);

    return m_subPicQueueSettings.bAllowDroppingSubpic;
}

STDMETHODIMP CDirectVobSub::put_AllowDroppingSubpic(bool bAllowDroppingSubpic)
{
    CAutoLock cAutoLock(&m_propsLock);

    if (m_subPicQueueSettings.bAllowDroppingSubpic == bAllowDroppingSubpic) {
        return S_FALSE;
    }

    m_subPicQueueSettings.bAllowDroppingSubpic = bAllowDroppingSubpic;

    return S_OK;
}

// IFilterVersion

STDMETHODIMP_(DWORD) CDirectVobSub::GetFilterVersion()
{
    return 0x0234;
}
