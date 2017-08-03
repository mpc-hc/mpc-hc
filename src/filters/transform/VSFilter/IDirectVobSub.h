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

#include "../../../Subtitles/STS.h"

#ifdef __cplusplus
extern "C" {
#endif

interface __declspec(uuid("EBE1FB08-3957-47ca-AF13-5827E5442E56"))
    IDirectVobSub :
    public IUnknown
{
    STDMETHOD(get_FileName)(THIS_ WCHAR* fn) PURE;  // fn should point to a buffer allocated to at least the length of MAX_PATH (=260)

    STDMETHOD(put_FileName)(THIS_ WCHAR* fn) PURE;

    STDMETHOD(get_LanguageCount)(THIS_ int* nLangs) PURE;

    STDMETHOD(get_LanguageName)(THIS_ int iLanguage, WCHAR** ppName) PURE; // the returned *ppName is allocated with CoTaskMemAlloc

    STDMETHOD(get_SelectedLanguage)(THIS_ int* iSelected) PURE;

    STDMETHOD(put_SelectedLanguage)(THIS_ int iSelected) PURE;

    STDMETHOD(get_HideSubtitles)(THIS_ bool* fHideSubtitles) PURE;

    STDMETHOD(put_HideSubtitles)(THIS_ bool fHideSubtitles) PURE;

    // deprecated
    STDMETHOD(get_PreBuffering)(THIS_ bool* fDoPreBuffering) PURE;

    // deprecated
    STDMETHOD(put_PreBuffering)(THIS_ bool fDoPreBuffering) PURE;

    STDMETHOD(get_Placement)(THIS_ bool* fOverridePlacement, int* xperc, int* yperc) PURE;

    STDMETHOD(put_Placement)(THIS_ bool fOverridePlacement, int xperc, int yperc) PURE;

    STDMETHOD(get_VobSubSettings)(THIS_ bool* fBuffer, bool* fOnlyShowForcedSubs, bool* fPolygonize) PURE;

    STDMETHOD(put_VobSubSettings)(THIS_ bool fBuffer, bool fOnlyShowForcedSubs, bool fPolygonize) PURE;

    // depending on lflen, lf must point to LOGFONTA or LOGFONTW
    STDMETHOD(get_TextSettings)(THIS_ void* lf, int lflen, COLORREF* color, bool* fShadow, bool* fOutline, bool* fAdvancedRenderer) PURE;

    STDMETHOD(put_TextSettings)(THIS_ void* lf, int lflen, COLORREF color, bool fShadow, bool fOutline, bool fAdvancedRenderer) PURE;

    STDMETHOD(get_Flip)(THIS_ bool* fPicture, bool* fSubtitles) PURE;

    STDMETHOD(put_Flip)(THIS_ bool fPicture, bool fSubtitles) PURE;

    STDMETHOD(get_OSD)(THIS_ bool* fOSD) PURE;

    STDMETHOD(put_OSD)(THIS_ bool fOSD) PURE;

    STDMETHOD(get_SaveFullPath)(THIS_ bool* fSaveFullPath) PURE;

    STDMETHOD(put_SaveFullPath)(THIS_ bool fSaveFullPath) PURE;

    STDMETHOD(get_SubtitleTiming)(THIS_ int* delay, int* speedmul, int* speeddiv) PURE;

    STDMETHOD(put_SubtitleTiming)(THIS_ int delay, int speedmul, int speeddiv) PURE;

    STDMETHOD(get_MediaFPS)(THIS_ bool* fEnabled, double* fps) PURE;

    STDMETHOD(put_MediaFPS)(THIS_ bool fEnabled, double fps) PURE;

    // no longer supported

    STDMETHOD(get_ColorFormat)(THIS_ int* iPosition) PURE;

    STDMETHOD(put_ColorFormat)(THIS_ int iPosition) PURE;


    STDMETHOD(get_ZoomRect)(THIS_ NORMALIZEDRECT* rect) PURE;

    STDMETHOD(put_ZoomRect)(THIS_ NORMALIZEDRECT* rect) PURE;


    STDMETHOD(UpdateRegistry)(THIS_) PURE;


    STDMETHOD(HasConfigDialog)(THIS_ int iSelected) PURE;

    STDMETHOD(ShowConfigDialog)(THIS_ int iSelected, HWND hWndParent) PURE; // if available, this will popup a child dialog allowing the user to edit the style options


    STDMETHOD(IsSubtitleReloaderLocked)(THIS_ bool* fLocked) PURE;

    STDMETHOD(LockSubtitleReloader)(THIS_ bool fLock) PURE;

    STDMETHOD(get_SubtitleReloader)(THIS_ bool* fDisabled) PURE;

    STDMETHOD(put_SubtitleReloader)(THIS_ bool fDisable) PURE;

    // horizontal: 0 - disabled, 1 - mod32 extension (width = (width + 31) & ~31)
    // vertical:   0 - disabled, 1 - 16:9, 2 - 4:3, 0x80 - crop (use crop together with 16:9 or 4:3, eg 0x81 will crop to 16:9 if the picture was taller)
    // resx2:      0 - disabled, 1 - enabled, 2 - depends on the original resolution
    // resx2minw:  resolution doubler will be used if width * height <= resx2minw * resx2minh (resx2minw * resx2minh equals to 384 * 288 by default)
    STDMETHOD(get_ExtendPicture)(THIS_ int* horizontal, int* vertical, int* resx2, int* resx2minw, int* resx2minh) PURE;

    STDMETHOD(put_ExtendPicture)(THIS_ int horizontal, int vertical, int resx2, int resx2minw, int resx2minh) PURE;

    // level: 0 - when needed, 1 - always, 2 - disabled
    STDMETHOD(get_LoadSettings)(THIS_ int* level, bool* fExternalLoad, bool* fWebLoad, bool* fEmbeddedLoad) PURE;

    STDMETHOD(put_LoadSettings)(THIS_ int level, bool fExternalLoad, bool fWebLoad, bool fEmbeddedLoad) PURE;

    STDMETHOD(get_SubPictToBuffer)(THIS_ unsigned int* uSubPictToBuffer) PURE;

    STDMETHOD(put_SubPictToBuffer)(THIS_ unsigned int uSubPictToBuffer) PURE;

    STDMETHOD(get_AnimWhenBuffering)(THIS_ bool* fAnimWhenBuffering) PURE;

    STDMETHOD(put_AnimWhenBuffering)(THIS_ bool fAnimWhenBuffering) PURE;
};

interface __declspec(uuid("FE6EC6A0-21CA-4970-9EF0-B296F7F38AF0"))
    ISubClock :
    public IUnknown
{
    STDMETHOD(SetTime)(REFERENCE_TIME rt) PURE;
    STDMETHOD_(REFERENCE_TIME, GetTime)() PURE;
};

interface __declspec(uuid("0665B760-FBC1-46C3-A35F-E471527C96A4"))
    ISubClock2 :
    public ISubClock
{
    STDMETHOD(SetAvgTimePerFrame)(REFERENCE_TIME rt) PURE;
    STDMETHOD(GetAvgTimePerFrame)(REFERENCE_TIME* prt) PURE;  // return S_OK only if *prt was set and is valid
};

interface __declspec(uuid("AB52FC9C-2415-4dca-BC1C-8DCC2EAE8150"))
    IDirectVobSub2 :
    public IDirectVobSub
{
    STDMETHOD(AdviseSubClock)(THIS_ ISubClock* pSubClock) PURE;

    STDMETHOD_(bool, get_Forced)(THIS_) PURE;

    STDMETHOD(put_Forced)(THIS_ bool fForced) PURE;

    STDMETHOD(get_TextSettings)(THIS_ STSStyle* pDefStyle) PURE;

    STDMETHOD(put_TextSettings)(THIS_ STSStyle* pDefStyle) PURE;

    STDMETHOD(get_AspectRatioSettings)(THIS_ CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType) PURE;

    STDMETHOD(put_AspectRatioSettings)(THIS_ CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType) PURE;
};

interface __declspec(uuid("4484A031-713F-4893-8C24-4505C56E8915"))
    IDirectVobSub3 :
    public IDirectVobSub2
{
    STDMETHOD_(bool, get_DisableSubtitleAnimation)(THIS_) PURE;
    STDMETHOD(put_DisableSubtitleAnimation)(THIS_ bool bDisableSubtitleAnimation) PURE;

    STDMETHOD_(int, get_RenderAtWhenAnimationIsDisabled)(THIS_) PURE;
    STDMETHOD(put_RenderAtWhenAnimationIsDisabled)(THIS_ int nRenderAtWhenAnimationIsDisabled) PURE;

    STDMETHOD_(int, get_AnimationRate)(THIS_) PURE;
    STDMETHOD(put_AnimationRate)(THIS_ int nAnimationRate) PURE;

    STDMETHOD_(bool, get_AllowDroppingSubpic)(THIS_) PURE;
    STDMETHOD(put_AllowDroppingSubpic)(THIS_ bool bAllowDroppingSubpic) PURE;
};


#ifdef __cplusplus
}
#endif
