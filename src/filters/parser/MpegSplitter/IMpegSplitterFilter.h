/*
 * (C) 2011-2013 see Authors.txt
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

interface __declspec(uuid("1DC9C085-04AC-4BB8-B2BD-C49A4D30B104"))
IMpegSplitterFilter :
public IUnknown {
    STDMETHOD(Apply()) PURE;

    STDMETHOD(SetFastStreamChange(BOOL nValue)) PURE;
    STDMETHOD_(BOOL, GetFastStreamChange()) PURE;

    STDMETHOD(SetForcedSub(BOOL nValue)) PURE;
    STDMETHOD_(BOOL, GetForcedSub()) PURE;

    STDMETHOD(SetAudioLanguageOrder(WCHAR * nValue)) PURE;
    STDMETHOD_(WCHAR*, GetAudioLanguageOrder()) PURE;

    STDMETHOD(SetSubtitlesLanguageOrder(WCHAR * nValue)) PURE;
    STDMETHOD_(WCHAR*, GetSubtitlesLanguageOrder()) PURE;

    STDMETHOD(SetVC1_GuidFlag(int nValue)) PURE;
    STDMETHOD_(int, GetVC1_GuidFlag()) PURE;

    STDMETHOD(SetTrueHD(int nValue)) PURE;
    STDMETHOD_(int, GetTrueHD()) PURE;

    STDMETHOD(SetAlternativeDuration(BOOL nValue)) PURE;
    STDMETHOD_(BOOL, GetAlternativeDuration()) PURE;

    STDMETHOD_(int, GetMPEGType()) PURE;
};
