/*
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include "../../../subtitles/STS.h"

#ifdef __cplusplus
extern "C" {
#endif

    [uuid("EBE1FB08-3957-47ca-AF13-5827E5442E56")]
interface IDirectVobSub :
    public IUnknown
    {
        STDMETHOD(get_FileName)(THIS_
                                WCHAR* fn	// fn should point to a buffer allocated to at least the length of MAX_PATH (=260)
                               ) PURE;

        STDMETHOD(put_FileName)(THIS_
                                WCHAR* fn
                               ) PURE;

        STDMETHOD(get_LanguageCount)(THIS_
                                     int* nLangs
                                    ) PURE;

        STDMETHOD(get_LanguageName)(THIS_
                                    int iLanguage,
                                    WCHAR** ppName	// the returned *ppName is allocated with CoTaskMemAlloc
                                   ) PURE;

        STDMETHOD(get_SelectedLanguage)(THIS_
                                        int* iSelected
                                       ) PURE;

        STDMETHOD(put_SelectedLanguage)(THIS_
                                        int iSelected
                                       ) PURE;

        STDMETHOD(get_HideSubtitles)(THIS_
                                     bool* fHideSubtitles
                                    ) PURE;

        STDMETHOD(put_HideSubtitles)(THIS_
                                     bool fHideSubtitles
                                    ) PURE;

        STDMETHOD(get_PreBuffering)(THIS_
                                    bool* fDoPreBuffering
                                   ) PURE;

        STDMETHOD(put_PreBuffering)(THIS_
                                    bool fDoPreBuffering
                                   ) PURE;

        STDMETHOD(get_Placement)(THIS_
                                 bool* fOverridePlacement,
                                 int* xperc,
                                 int* yperc
                                ) PURE;

        STDMETHOD(put_Placement)(THIS_
                                 bool fOverridePlacement,
                                 int xperc,
                                 int yperc
                                ) PURE;

        STDMETHOD(get_VobSubSettings)(THIS_
                                      bool* fBuffer,
                                      bool* fOnlyShowForcedSubs,
                                      bool* fPolygonize
                                     ) PURE;

        STDMETHOD(put_VobSubSettings)(THIS_
                                      bool fBuffer,
                                      bool fOnlyShowForcedSubs,
                                      bool fPolygonize
                                     ) PURE;

        STDMETHOD(get_TextSettings)(THIS_
                                    void* lf,
                                    int lflen, // depending on lflen, lf must point to LOGFONTA or LOGFONTW
                                    COLORREF* color,
                                    bool* fShadow,
                                    bool* fOutline,
                                    bool* fAdvancedRenderer
                                   ) PURE;

        STDMETHOD(put_TextSettings)(THIS_
                                    void* lf,
                                    int lflen,
                                    COLORREF color,
                                    bool fShadow,
                                    bool fOutline,
                                    bool fAdvancedRenderer
                                   ) PURE;

        STDMETHOD(get_Flip)(THIS_
                            bool* fPicture,
                            bool* fSubtitles
                           ) PURE;

        STDMETHOD(put_Flip)(THIS_
                            bool fPicture,
                            bool fSubtitles
                           ) PURE;

        STDMETHOD(get_OSD)(THIS_
                           bool* fOSD
                          ) PURE;

        STDMETHOD(put_OSD)(THIS_
                           bool fOSD
                          ) PURE;

        STDMETHOD(get_SaveFullPath)(THIS_
                                    bool* fSaveFullPath
                                   ) PURE;

        STDMETHOD(put_SaveFullPath)(THIS_
                                    bool fSaveFullPath
                                   ) PURE;

        STDMETHOD(get_SubtitleTiming)(THIS_
                                      int* delay,
                                      int* speedmul,
                                      int* speeddiv
                                     ) PURE;

        STDMETHOD(put_SubtitleTiming)(THIS_
                                      int delay,
                                      int speedmul,
                                      int speeddiv
                                     ) PURE;

        STDMETHOD(get_MediaFPS)(THIS_
                                bool* fEnabled,
                                double* fps
                               ) PURE;

        STDMETHOD(put_MediaFPS)(THIS_
                                bool fEnabled,
                                double fps
                               ) PURE;

        // no longer supported

        STDMETHOD(get_ColorFormat)(THIS_
                                   int* iPosition
                                  ) PURE;

        STDMETHOD(put_ColorFormat)(THIS_
                                   int iPosition
                                  ) PURE;

        //

        STDMETHOD(get_ZoomRect)(THIS_
                                NORMALIZEDRECT* rect
                               ) PURE;

        STDMETHOD(put_ZoomRect)(THIS_
                                NORMALIZEDRECT* rect
                               ) PURE;

        //

        STDMETHOD(UpdateRegistry)(THIS_
                                 ) PURE;

        //

        STDMETHOD(HasConfigDialog)(THIS_
                                   int iSelected
                                  ) PURE;

        STDMETHOD(ShowConfigDialog)(THIS_	// if available, this will popup a child dialog allowing the user to edit the style options
                                    int iSelected,
                                    HWND hWndParent
                                   ) PURE;

        //

        STDMETHOD(IsSubtitleReloaderLocked)(THIS_
                                            bool* fLocked
                                           ) PURE;

        STDMETHOD(LockSubtitleReloader)(THIS_
                                        bool fLock
                                       ) PURE;

        STDMETHOD(get_SubtitleReloader)(THIS_
                                        bool* fDisabled
                                       ) PURE;

        STDMETHOD(put_SubtitleReloader)(THIS_
                                        bool fDisable
                                       ) PURE;

        //

        STDMETHOD(get_ExtendPicture)(THIS_
                                     int* horizontal, // 0 - disabled, 1 - mod32 extension (width = (width+31)&~31)
                                     int* vertical, // 0 - disabled, 1 - 16:9, 2 - 4:3, 0x80 - crop (use crop together with 16:9 or 4:3, eg 0x81 will crop to 16:9 if the picture was taller)
                                     int* resx2, // 0 - disabled, 1 - enabled, 2 - depends on the original resolution
                                     int* resx2minw, // resolution doubler will be used if width*height <= resx2minw*resx2minh (resx2minw*resx2minh equals to 384*288 by default)
                                     int* resx2minh
                                    ) PURE;

        STDMETHOD(put_ExtendPicture)(THIS_
                                     int horizontal,
                                     int vertical,
                                     int resx2,
                                     int resx2minw,
                                     int resx2minh
                                    ) PURE;

        STDMETHOD(get_LoadSettings)(THIS_
                                    int* level, // 0 - when needed, 1 - always, 2 - disabled
                                    bool* fExternalLoad,
                                    bool* fWebLoad,
                                    bool* fEmbeddedLoad
                                   ) PURE;

        STDMETHOD(put_LoadSettings)(THIS_
                                    int level,
                                    bool fExternalLoad,
                                    bool fWebLoad,
                                    bool fEmbeddedLoad
                                   ) PURE;
    };

    [uuid("FE6EC6A0-21CA-4970-9EF0-B296F7F38AF0")]
interface ISubClock :
    public IUnknown
    {
        STDMETHOD(SetTime)(REFERENCE_TIME rt) PURE;
        STDMETHOD_(REFERENCE_TIME, GetTime)() PURE;
    };

    [uuid("0665B760-FBC1-46C3-A35F-E471527C96A4")]
interface ISubClock2 :
    public ISubClock
    {
        STDMETHOD(SetAvgTimePerFrame)(REFERENCE_TIME rt) PURE;
        STDMETHOD(GetAvgTimePerFrame)(REFERENCE_TIME* prt) PURE; // return S_OK only if *prt was set and is valid
    };

    [uuid("AB52FC9C-2415-4dca-BC1C-8DCC2EAE8150")]
interface IDirectVobSub2 :
    public IDirectVobSub
    {
        STDMETHOD(AdviseSubClock)(THIS_
        ISubClock* pSubClock
                                 ) PURE;

        STDMETHOD_(bool, get_Forced)(THIS_
                                    ) PURE;

        STDMETHOD(put_Forced)(THIS_
        bool fForced
                             ) PURE;

        STDMETHOD(get_TextSettings)(THIS_
        STSStyle* pDefStyle
                                   ) PURE;

        STDMETHOD(put_TextSettings)(THIS_
        STSStyle* pDefStyle
                                   ) PURE;

        STDMETHOD(get_AspectRatioSettings)(THIS_
        CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType
                                          ) PURE;

        STDMETHOD(put_AspectRatioSettings)(THIS_
        CSimpleTextSubtitle::EPARCompensationType* ePARCompensationType
                                          ) PURE;
    };


#ifdef __cplusplus
}
#endif
