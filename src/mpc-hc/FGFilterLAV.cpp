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

#include "stdafx.h"
#include <mpconfig.h>
#include "FGFilterLAV.h"
#include "MainFrm.h"
#include "DSUtil.h"
#include "AllocatorCommon7.h"
#include "AllocatorCommon.h"
#include "SyncAllocatorPresenter.h"
#include "moreuuids.h"

#include "LAVFilters/LAVSplitterSettings.h"
#include "LAVFilters/LAVVideoSettings.h"
#include "LAVFilters/LAVAudioSettings.h"

//
// CFGFilterLAV
//

CFGFilterLAV::CFGFilterLAV(const CLSID& clsid, CString path, CStringW name, bool bAddLowMeritSuffix, UINT64 merit)
    : CFGFilterFile(clsid, path, name + (bAddLowMeritSuffix ? LowMeritSuffix : L""), merit)
{
}

//
// CFGFilterLAVSplitterBase
//

CFGFilterLAVSplitterBase::CFGFilterLAVSplitterBase(const CLSID& clsid, CStringW name, bool bAddLowMeritSuffix, UINT64 merit)
    : CFGFilterLAV(clsid, _T(".\\LAVFilters\\x86\\LAVSplitter.ax"), name, bAddLowMeritSuffix, merit)
{
}

HRESULT CFGFilterLAVSplitterBase::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    HRESULT hr = __super::Create(ppBF, pUnks);

    if (SUCCEEDED(hr)) {
        if (CComQIPtr<ILAVFSettings> pSettings = *ppBF) {
            hr = pSettings->SetRuntimeConfig(TRUE);
        } else {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

//
// CFGFilterLAVSplitter
//

CFGFilterLAVSplitter::CFGFilterLAVSplitter(UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAVSplitterBase(GUID_LAVSplitter, L"LAV Splitter (internal)", bAddLowMeritSuffix, merit)
{
}

//
// CFGFilterLAVSplitterSource
//

CFGFilterLAVSplitterSource::CFGFilterLAVSplitterSource(UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAVSplitterBase(GUID_LAVSplitterSource, L"LAV Splitter Source (internal)", bAddLowMeritSuffix, merit)
{
}

//
// CFGFilterLAVVideo
//

CFGFilterLAVVideo::CFGFilterLAVVideo(UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAV(GUID_LAVVideo, _T(".\\LAVFilters\\x86\\LAVVideo.ax"), L"LAV Video Decoder (internal)", bAddLowMeritSuffix, merit)
{
}

HRESULT CFGFilterLAVVideo::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    HRESULT hr = __super::Create(ppBF, pUnks);

    if (SUCCEEDED(hr)) {
        if (CComQIPtr<ILAVVideoSettings> pSettings = *ppBF) {
            hr = pSettings->SetRuntimeConfig(TRUE);
        } else {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

//
// CFGFilterLAVAudio
//

CFGFilterLAVAudio::CFGFilterLAVAudio(UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAV(GUID_LAVAudio, _T(".\\LAVFilters\\x86\\LAVAudio.ax"), L"LAV Audio Decoder (internal)", bAddLowMeritSuffix, merit)
{
}

HRESULT CFGFilterLAVAudio::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    HRESULT hr = __super::Create(ppBF, pUnks);

    if (SUCCEEDED(hr)) {
        if (CComQIPtr<ILAVAudioSettings> pSettings = *ppBF) {
            hr = pSettings->SetRuntimeConfig(TRUE);
        } else {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}
