/*
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

#pragma once

#include "IDirectVobSub.h"
#include "IFilterVersion.h"

class CDirectVobSub : public IDirectVobSub2, public IFilterVersion
{
protected:
    CDirectVobSub();
    virtual ~CDirectVobSub();

protected:
    CCritSec m_propsLock;

    CString m_FileName;
    int m_iSelectedLanguage;
    bool m_fHideSubtitles;
    unsigned int m_uSubPictToBuffer;
    bool m_fAnimWhenBuffering;
    bool m_fOverridePlacement;
    int m_PlacementXperc, m_PlacementYperc;
    bool m_fBufferVobSub, m_fOnlyShowForcedVobSubs, m_fPolygonize;
    CSimpleTextSubtitle::EPARCompensationType m_ePARCompensationType;

    STSStyle m_defStyle;

    bool m_fAdvancedRenderer;
    bool m_fFlipPicture, m_fFlipSubtitles;
    bool m_fOSD;
    int m_nReloaderDisableCount;
    int m_SubtitleDelay, m_SubtitleSpeedMul, m_SubtitleSpeedDiv;
    bool m_fMediaFPSEnabled;
    double m_MediaFPS;
    bool m_fSaveFullPath;
	bool m_fSubtitlesOverlapping;
    NORMALIZEDRECT m_ZoomRect;

    CComPtr<ISubClock> m_pSubClock;
    bool m_fForced;

public:

    // IDirectVobSub

    STDMETHODIMP get_FileName(WCHAR* fn);
    STDMETHODIMP put_FileName(WCHAR* fn);
    STDMETHODIMP get_LanguageCount(int* nLangs);
    STDMETHODIMP get_LanguageName(int iLanguage, WCHAR** ppName);
    STDMETHODIMP get_SelectedLanguage(int* iSelected);
    STDMETHODIMP put_SelectedLanguage(int iSelected);
    STDMETHODIMP get_HideSubtitles(bool* fHideSubtitles);
    STDMETHODIMP put_HideSubtitles(bool fHideSubtitles);
    STDMETHODIMP get_PreBuffering(bool* fDoPreBuffering); // deprecated
    STDMETHODIMP put_PreBuffering(bool fDoPreBuffering);  // deprecated
    STDMETHODIMP get_SubPictToBuffer(unsigned int* uSubPictToBuffer);
    STDMETHODIMP put_SubPictToBuffer(unsigned int uSubPictToBuffer);
    STDMETHODIMP get_AnimWhenBuffering(bool* fAnimWhenBuffering);
    STDMETHODIMP put_AnimWhenBuffering(bool fAnimWhenBuffering);
    STDMETHODIMP get_Placement(bool* fOverridePlacement, int* xperc, int* yperc);
    STDMETHODIMP put_Placement(bool fOverridePlacement, int xperc, int yperc);
    STDMETHODIMP get_VobSubSettings(bool* fBuffer, bool* fOnlyShowForcedSubs, bool* fPolygonize);
    STDMETHODIMP put_VobSubSettings(bool fBuffer, bool fOnlyShowForcedSubs, bool fPolygonize);
    STDMETHODIMP get_TextSettings(void* lf, int lflen, COLORREF* color, bool* fShadow, bool* fOutline, bool* fAdvancedRenderer);
    STDMETHODIMP put_TextSettings(void* lf, int lflen, COLORREF color, bool fShadow, bool fOutline, bool fAdvancedRenderer);
    STDMETHODIMP get_Flip(bool* fPicture, bool* fSubtitles);
    STDMETHODIMP put_Flip(bool fPicture, bool fSubtitles);
    STDMETHODIMP get_OSD(bool* fShowOSD);
    STDMETHODIMP put_OSD(bool fShowOSD);
    STDMETHODIMP get_SaveFullPath(bool* fSaveFullPath);
    STDMETHODIMP put_SaveFullPath(bool fSaveFullPath);
	STDMETHODIMP get_SubtitlesOverlapping(bool* fSubtitlesOverlapping);
	STDMETHODIMP put_SubtitlesOverlapping(bool fSubtitlesOverlapping);
    STDMETHODIMP get_SubtitleTiming(int* delay, int* speedmul, int* speeddiv);
    STDMETHODIMP put_SubtitleTiming(int delay, int speedmul, int speeddiv);
    STDMETHODIMP get_MediaFPS(bool* fEnabled, double* fps);
    STDMETHODIMP put_MediaFPS(bool fEnabled, double fps);
    STDMETHODIMP get_ZoomRect(NORMALIZEDRECT* rect);
    STDMETHODIMP put_ZoomRect(NORMALIZEDRECT* rect);
    STDMETHODIMP get_ColorFormat(int* iPosition) { return E_NOTIMPL; }
    STDMETHODIMP put_ColorFormat(int iPosition) { return E_NOTIMPL; }

    STDMETHODIMP UpdateRegistry();

    STDMETHODIMP HasConfigDialog(int iSelected);
    STDMETHODIMP ShowConfigDialog(int iSelected, HWND hWndParent);

    // settings for the rest are stored in the registry

    STDMETHODIMP IsSubtitleReloaderLocked(bool* fLocked);
    STDMETHODIMP LockSubtitleReloader(bool fLock);
    STDMETHODIMP get_SubtitleReloader(bool* fDisabled);
    STDMETHODIMP put_SubtitleReloader(bool fDisable);

    // the followings need a partial or full reloading of the filter

    STDMETHODIMP get_ExtendPicture(int* horizontal, int* vertical, int* resx2, int* resx2minw, int* resx2minh);
    STDMETHODIMP put_ExtendPicture(int horizontal, int vertical, int resx2, int resx2minw, int resx2minh);
    STDMETHODIMP get_LoadSettings(int* level, bool* fExternalLoad, bool* fWebLoad, bool* fEmbeddedLoad);
    STDMETHODIMP put_LoadSettings(int level, bool fExternalLoad, bool fWebLoad, bool fEmbeddedLoad);

    // IDirectVobSub2

    STDMETHODIMP AdviseSubClock(ISubClock* pSubClock);
    STDMETHODIMP_(bool) get_Forced();
    STDMETHODIMP put_Forced(bool fForced);
    STDMETHODIMP get_TextSettings(STSStyle* pDefStyle);
    STDMETHODIMP put_TextSettings(STSStyle* pDefStyle);
    STDMETHODIMP get_AspectRatioSettings(CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType);
    STDMETHODIMP put_AspectRatioSettings(CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType);

    // IFilterVersion

    STDMETHODIMP_(DWORD) GetFilterVersion();
};
