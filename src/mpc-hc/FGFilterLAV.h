/*
 * (C) 2013 see Authors.txt
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

DEFINE_GUID(GUID_LAVSplitter, 0x171252A0, 0x8820, 0x4AFE, 0x9D, 0xF8, 0x5C, 0x92, 0xB2, 0xD6, 0x6B, 0x04);
DEFINE_GUID(GUID_LAVSplitterSource, 0xB98D13E7, 0x55DB, 0x4385, 0xA3, 0x3D, 0x09, 0xFD, 0x1B, 0xA2, 0x63, 0x38);
DEFINE_GUID(GUID_LAVVideo, 0xEE30215D, 0x164F, 0x4A92, 0xA4, 0xEB, 0x9D, 0x4C, 0x13, 0x39, 0x0F, 0x9F);
DEFINE_GUID(GUID_LAVAudio, 0xE8E73B6B, 0x4CB3, 0x44A4, 0xBE, 0x99, 0x4F, 0x7B, 0xCB, 0x96, 0xE4, 0x91);

#include "FGFilter.h"

class CFGFilterLAV : public CFGFilterFile
{
protected:
    CFGFilterLAV(const CLSID& clsid, CString path, CStringW name, bool bAddLowMeritSuffix, UINT64 merit);

public:
    enum LAVFILTER_TYPE {
        SPLITTER,
        SPLITTER_SOURCE,
        VIDEO_DECODER,
        AUDIO_DECODER
    };

    static CString GetFilterPath(LAVFILTER_TYPE filterType);
    static bool CheckVersion(CString filtersPath);

    static CFGFilterLAV* CreateFilter(LAVFILTER_TYPE filterType, UINT64 merit = MERIT64_DO_USE, bool bAddLowMeritSuffix = false);
};

class CFGFilterLAVSplitterBase : public CFGFilterLAV
{
protected:
    CFGFilterLAVSplitterBase(CString path, const CLSID& clsid, CStringW name, bool bAddLowMeritSuffix, UINT64 merit);

public:
    static const CString filename;

    virtual HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
};

class CFGFilterLAVSplitter : public CFGFilterLAVSplitterBase
{
public:
    CFGFilterLAVSplitter(CString path, UINT64 merit = MERIT64_DO_USE, bool bAddLowMeritSuffix = false);
};

class CFGFilterLAVSplitterSource : public CFGFilterLAVSplitterBase
{
public:
    CFGFilterLAVSplitterSource(CString path, UINT64 merit = MERIT64_DO_USE, bool bAddLowMeritSuffix = false);
};

class CFGFilterLAVVideo : public CFGFilterLAV
{
public:
    static const CString filename;

    CFGFilterLAVVideo(CString path, UINT64 merit = MERIT64_DO_USE, bool bAddLowMeritSuffix = false);

    virtual HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
};

class CFGFilterLAVAudio : public CFGFilterLAV
{
public:
    static const CString filename;

    CFGFilterLAVAudio(CString path, UINT64 merit = MERIT64_DO_USE, bool bAddLowMeritSuffix = false);

    virtual HRESULT Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks);
};
