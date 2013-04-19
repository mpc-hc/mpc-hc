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
#include "FileVersionInfo.h"
#include "WinAPIUtils.h"
#include "AllocatorCommon7.h"
#include "AllocatorCommon.h"
#include "SyncAllocatorPresenter.h"
#include "moreuuids.h"

#include "LAVFilters/LAVSplitterSettings.h"
#include "LAVFilters/LAVVideoSettings.h"
#include "LAVFilters/LAVAudioSettings.h"

#define LAV_FILTERS_VERSION_MAJOR    0
#define LAV_FILTERS_VERSION_MINOR    56
#define LAV_FILTERS_VERSION_REVISION 2
#define LAV_FILTERS_VERSION ((QWORD)LAV_FILTERS_VERSION_MAJOR << 48 | (QWORD)LAV_FILTERS_VERSION_MINOR << 32 | (QWORD)LAV_FILTERS_VERSION_REVISION << 16)

//
// CFGFilterLAV
//

CFGFilterLAV::CFGFilterLAV(const CLSID& clsid, CString path, CStringW name, bool bAddLowMeritSuffix, UINT64 merit)
    : CFGFilterFile(clsid, path, name + (bAddLowMeritSuffix ? LowMeritSuffix : L""), merit)
{
}

CString CFGFilterLAV::GetFilterPath(LAVFILTER_TYPE filterType)
{
    // Default path
    CString filterPath = GetProgramPath() + _T("LAVFilters\\");
    CLSID filterCLSID;

    switch (filterType) {
        case SPLITTER:
        case SPLITTER_SOURCE:
            filterPath += CFGFilterLAVSplitterBase::filename;
            filterCLSID = GUID_LAVSplitter;
            break;
        case VIDEO_DECODER:
            filterPath += CFGFilterLAVVideo::filename;
            filterCLSID = GUID_LAVVideo;
            break;
        case AUDIO_DECODER:
            filterPath += CFGFilterLAVAudio::filename;
            filterCLSID = GUID_LAVAudio;
            break;
        default:
            ASSERT(FALSE); // This should never happen
            break;
    }

#if ENABLE_LOAD_EXTERNAL_LAVF_AS_INTERNAL
    // Check that the filter's version is correct
    if (!CheckVersion(filterPath)) {
        // If not, check if a registered version of the filter is available.
        filterPath = ::GetFilterPath(filterCLSID);
        // and if it can be used
        if (!CheckVersion(filterPath)) {
            filterPath = _T("");
        }
    }
#endif

    return filterPath;
}

bool CFGFilterLAV::CheckVersion(CString filterPath)
{
    QWORD version = CFileVersionInfo::GetFileVersionNum(filterPath);

    return (version >= LAV_FILTERS_VERSION);
}

CFGFilterLAV* CFGFilterLAV::CreateFilter(LAVFILTER_TYPE filterType, UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
{
    CFGFilterLAV* filter = nullptr;

    CString filterPath = GetFilterPath(filterType);

    switch (filterType) {
        case SPLITTER:
            filter = DEBUG_NEW CFGFilterLAVSplitter(filterPath, merit, bAddLowMeritSuffix);
            break;
        case SPLITTER_SOURCE:
            filter = DEBUG_NEW CFGFilterLAVSplitterSource(filterPath, merit, bAddLowMeritSuffix);
            break;
        case VIDEO_DECODER:
            filter = DEBUG_NEW CFGFilterLAVVideo(filterPath, merit, bAddLowMeritSuffix);
            break;
        case AUDIO_DECODER:
            filter = DEBUG_NEW CFGFilterLAVAudio(filterPath, merit, bAddLowMeritSuffix);
            break;
        default:
            ASSERT(FALSE); // This should never happen
            break;
    }

    return filter;
}

//
// CFGFilterLAVSplitterBase
//

const CString CFGFilterLAVSplitterBase::filename = _T("LAVSplitter.ax");

CFGFilterLAVSplitterBase::CFGFilterLAVSplitterBase(CString path, const CLSID& clsid, CStringW name, bool bAddLowMeritSuffix, UINT64 merit)
    : CFGFilterLAV(clsid, path, name, bAddLowMeritSuffix, merit)
{
}

HRESULT CFGFilterLAVSplitterBase::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    HRESULT hr = __super::Create(ppBF, pUnks);

    if (SUCCEEDED(hr)) {
        if (!CheckVersion(m_path)) {
            hr = E_FAIL;
        } else if (CComQIPtr<ILAVFSettings> pSettings = *ppBF) {
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

CFGFilterLAVSplitter::CFGFilterLAVSplitter(CString path, UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAVSplitterBase(path, GUID_LAVSplitter, L"LAV Splitter (internal)", bAddLowMeritSuffix, merit)
{
}

//
// CFGFilterLAVSplitterSource
//

CFGFilterLAVSplitterSource::CFGFilterLAVSplitterSource(CString path, UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAVSplitterBase(path, GUID_LAVSplitterSource, L"LAV Splitter Source (internal)", bAddLowMeritSuffix, merit)
{
}

//
// CFGFilterLAVVideo
//

const CString CFGFilterLAVVideo::filename = _T("LAVVideo.ax");

CFGFilterLAVVideo::CFGFilterLAVVideo(CString path, UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAV(GUID_LAVVideo, path, L"LAV Video Decoder (internal)", bAddLowMeritSuffix, merit)
{
}

HRESULT CFGFilterLAVVideo::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    HRESULT hr = __super::Create(ppBF, pUnks);

    if (SUCCEEDED(hr)) {
        if (!CheckVersion(m_path)) {
            hr = E_FAIL;
        } else if (CComQIPtr<ILAVVideoSettings> pSettings = *ppBF) {
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

const CString CFGFilterLAVAudio::filename = _T("LAVAudio.ax");

CFGFilterLAVAudio::CFGFilterLAVAudio(CString path, UINT64 merit /*= MERIT64_DO_USE*/, bool bAddLowMeritSuffix /*= false*/)
    : CFGFilterLAV(GUID_LAVAudio, path, L"LAV Audio Decoder (internal)", bAddLowMeritSuffix, merit)
{
}

HRESULT CFGFilterLAVAudio::Create(IBaseFilter** ppBF, CInterfaceList<IUnknown, &IID_IUnknown>& pUnks)
{
    HRESULT hr = __super::Create(ppBF, pUnks);

    if (SUCCEEDED(hr)) {
        if (!CheckVersion(m_path)) {
            hr = E_FAIL;
        } else if (CComQIPtr<ILAVAudioSettings> pSettings = *ppBF) {
            hr = pSettings->SetRuntimeConfig(TRUE);
        } else {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

