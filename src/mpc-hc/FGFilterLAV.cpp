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

#define LAV_FILTERS_VERSION_MAJOR    0
#define LAV_FILTERS_VERSION_MINOR    56
#define LAV_FILTERS_VERSION_REVISION 2
#define LAV_FILTERS_VERSION ((QWORD)LAV_FILTERS_VERSION_MAJOR << 48 | (QWORD)LAV_FILTERS_VERSION_MINOR << 32 | (QWORD)LAV_FILTERS_VERSION_REVISION << 16)

#define IDS_R_INTERNAL_LAVSPLITTER           IDS_R_INTERNAL_FILTERS  _T("\\LAVSplitter")
#define IDS_R_INTERNAL_LAVVIDEO              IDS_R_INTERNAL_FILTERS  _T("\\LAVVideo")
#define IDS_R_INTERNAL_LAVVIDEO_OUTPUTFORMAT IDS_R_INTERNAL_LAVVIDEO _T("\\OutputFormat")
#define IDS_R_INTERNAL_LAVVIDEO_HWACCEL      IDS_R_INTERNAL_LAVVIDEO _T("\\HWAccel")
#define IDS_R_INTERNAL_LAVAUDIO              IDS_R_INTERNAL_FILTERS  _T("\\LAVAudio")

//
// CFGFilterLAV
//

CList<const IBaseFilter*> CFGFilterLAV::s_instances;

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
        } else if (CComQIPtr<ILAVFSettings> pLAVFSettings = *ppBF) {
            // Take control over LAVSplitter's settings, the settings are reseted to default values
            hr = pLAVFSettings->SetRuntimeConfig(TRUE);

            if (SUCCEEDED(hr)) {
                Settings settings;
                if (settings.GetSettings(pLAVFSettings)) { // Get default settings from LAVSplitter
                    settings.LoadSettings(); // Load our current settings from registry/ini
                    settings.SetSettings(pLAVFSettings); // Set our settings in LAVSplitter
                }

                // Keep track of LAVFilters instances in runtime mode
                s_instances.AddTail(*ppBF);
            }
        } else {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

void CFGFilterLAVSplitterBase::Settings::LoadSettings()
{
    CMPlayerCApp* pApp = AfxGetMyApp();
    ASSERT(pApp);

    bTrayIcon = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("TrayIcon"), bTrayIcon);

    prefAudioLangs = pApp->GetProfileString(IDS_R_INTERNAL_LAVSPLITTER, _T("prefAudioLangs"), prefAudioLangs.c_str());
    prefSubLangs = pApp->GetProfileString(IDS_R_INTERNAL_LAVSPLITTER, _T("prefSubLangs"), prefSubLangs.c_str());
    subtitleAdvanced = pApp->GetProfileString(IDS_R_INTERNAL_LAVSPLITTER, _T("subtitleAdvanced"), subtitleAdvanced.c_str());

    subtitleMode = (LAVSubtitleMode)pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("subtitleMode"), subtitleMode);

    bPGSForcedStream = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("PGSForcedStream"), bPGSForcedStream);

    bPGSOnlyForced = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("PGSOnlyForced"), bPGSOnlyForced);

    iVC1Mode = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("vc1TimestampMode"), iVC1Mode);

    bSubstreams = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("substreams"), bSubstreams);

    bMatroskaExternalSegments = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("MatroskaExternalSegments"), bMatroskaExternalSegments);

    bStreamSwitchRemoveAudio = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("StreamSwitchRemoveAudio"), bStreamSwitchRemoveAudio);

    bPreferHighQualityAudio = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("PreferHighQualityAudio"), bPreferHighQualityAudio);

    bImpairedAudio = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("ImpairedAudio"), bImpairedAudio);

    dwQueueMaxSize = pApp->GetProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("QueueMaxSize"), dwQueueMaxSize);
}

void CFGFilterLAVSplitterBase::Settings::SaveSettings()
{
    CMPlayerCApp* pApp = AfxGetMyApp();
    ASSERT(pApp);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("TrayIcon"), bTrayIcon);

    pApp->WriteProfileString(IDS_R_INTERNAL_LAVSPLITTER, _T("prefAudioLangs"), prefAudioLangs.c_str());
    pApp->WriteProfileString(IDS_R_INTERNAL_LAVSPLITTER, _T("prefSubLangs"), prefSubLangs.c_str());
    pApp->WriteProfileString(IDS_R_INTERNAL_LAVSPLITTER, _T("subtitleAdvanced"), subtitleAdvanced.c_str());

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("subtitleMode"), subtitleMode);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("PGSForcedStream"), bPGSForcedStream);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("PGSOnlyForced"), bPGSOnlyForced);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("vc1TimestampMode"), iVC1Mode);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("substreams"), bSubstreams);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("MatroskaExternalSegments"), bMatroskaExternalSegments);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("StreamSwitchRemoveAudio"), bStreamSwitchRemoveAudio);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("PreferHighQualityAudio"), bPreferHighQualityAudio);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("ImpairedAudio"), bImpairedAudio);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVSPLITTER, _T("QueueMaxSize"), dwQueueMaxSize);
}

bool CFGFilterLAVSplitterBase::Settings::GetSettings(CComQIPtr<ILAVFSettings> pLAVFSettings)
{
    if (!pLAVFSettings) {
        return false;
    }

    bTrayIcon = pLAVFSettings->GetTrayIcon();

    HRESULT hr;
    LPWSTR lpwstr = nullptr;
    hr = pLAVFSettings->GetPreferredLanguages(&lpwstr);
    if (SUCCEEDED(hr) && lpwstr) {
        prefAudioLangs = lpwstr;
        CoTaskMemFree(lpwstr);
    }
    lpwstr = nullptr;
    hr = pLAVFSettings->GetPreferredSubtitleLanguages(&lpwstr);
    if (SUCCEEDED(hr) && lpwstr) {
        prefSubLangs = lpwstr;
        CoTaskMemFree(lpwstr);
    }
    lpwstr = nullptr;
    hr = pLAVFSettings->GetAdvancedSubtitleConfig(&lpwstr);
    if (SUCCEEDED(hr) && lpwstr) {
        subtitleAdvanced = lpwstr;
        CoTaskMemFree(lpwstr);
    }

    subtitleMode = pLAVFSettings->GetSubtitleMode();

    bPGSForcedStream = pLAVFSettings->GetPGSForcedStream();

    bPGSOnlyForced = pLAVFSettings->GetPGSOnlyForced();

    iVC1Mode = pLAVFSettings->GetVC1TimestampMode();

    bSubstreams = pLAVFSettings->GetSubstreamsEnabled();

    bMatroskaExternalSegments = pLAVFSettings->GetLoadMatroskaExternalSegments();

    bStreamSwitchRemoveAudio = pLAVFSettings->GetStreamSwitchRemoveAudio();

    bImpairedAudio = pLAVFSettings->GetUseAudioForHearingVisuallyImpaired();

    bPreferHighQualityAudio = pLAVFSettings->GetPreferHighQualityAudioStreams();

    dwQueueMaxSize = pLAVFSettings->GetMaxQueueMemSize();

    return true;
}

bool CFGFilterLAVSplitterBase::Settings::SetSettings(CComQIPtr<ILAVFSettings> pLAVFSettings)
{
    if (!pLAVFSettings) {
        return false;
    }

    pLAVFSettings->SetTrayIcon(bTrayIcon);

    pLAVFSettings->SetPreferredLanguages(const_cast<WCHAR*>(prefAudioLangs.c_str()));
    pLAVFSettings->SetPreferredSubtitleLanguages(const_cast<WCHAR*>(prefSubLangs.c_str()));
    pLAVFSettings->SetAdvancedSubtitleConfig(const_cast<WCHAR*>(subtitleAdvanced.c_str()));

    pLAVFSettings->SetSubtitleMode(subtitleMode);

    pLAVFSettings->SetPGSForcedStream(bPGSForcedStream);

    pLAVFSettings->SetPGSOnlyForced(bPGSOnlyForced);

    pLAVFSettings->SetVC1TimestampMode(iVC1Mode);

    pLAVFSettings->SetSubstreamsEnabled(bSubstreams);

    pLAVFSettings->SetLoadMatroskaExternalSegments(bMatroskaExternalSegments);

    pLAVFSettings->SetStreamSwitchRemoveAudio(bStreamSwitchRemoveAudio);

    pLAVFSettings->SetUseAudioForHearingVisuallyImpaired(bImpairedAudio);

    pLAVFSettings->SetPreferHighQualityAudioStreams(bPreferHighQualityAudio);

    pLAVFSettings->SetMaxQueueMemSize(dwQueueMaxSize);

    return true;
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
        } else if (CComQIPtr<ILAVVideoSettings> pLAVFSettings = *ppBF) {
            // Take control over LAVVideo's settings, the settings are reseted to default values
            hr = pLAVFSettings->SetRuntimeConfig(TRUE);

            if (SUCCEEDED(hr)) {
                Settings settings;
                if (settings.GetSettings(pLAVFSettings)) { // Get default settings from LAVVideo
                    settings.LoadSettings(); // Load our current settings from registry/ini
                    settings.SetSettings(pLAVFSettings); // Set our settings in LAVVideo
                }

                // Keep track of LAVFilters instances in runtime mode
                s_instances.AddTail(*ppBF);
            }
        } else {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

static LPCTSTR pixFmtSettingsMap[LAVOutPixFmt_NB] = {
    _T("yv12"), _T("nv12"), _T("yuy2"), _T("uyvy"), _T("ayuv"), _T("p010"), _T("p210"), _T("y410"),
    _T("p016"), _T("p216"), _T("y416"), _T("rgb32"), _T("rgb24"), _T("v210"), _T("v410"), _T("yv16"), _T("yv24")
};

void CFGFilterLAVVideo::Settings::LoadSettings()
{
    CMPlayerCApp* pApp = AfxGetMyApp();
    ASSERT(pApp);

    bTrayIcon = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("TrayIcon"), bTrayIcon);

    dwStreamAR = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("StreamAR"), dwStreamAR);

    dwNumThreads = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("NumThreads"), dwNumThreads);

    dwDeintFieldOrder = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("DeintFieldOrder"), dwDeintFieldOrder);

    deintMode = (LAVDeintMode)pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("DeintMode"), deintMode);

    dwRGBRange = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("RGBRange"), dwRGBRange);

    dwSWDeintMode = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("SWDeintMode"), dwSWDeintMode);

    dwSWDeintOutput = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("SWDeintOutput"), dwSWDeintOutput);

    dwDitherMode = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("DitherMode"), dwDitherMode);

    for (int i = 0; i < LAVOutPixFmt_NB; ++i) {
        bPixFmts[i] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_OUTPUTFORMAT, pixFmtSettingsMap[i], bPixFmts[i]);
    }
    // Force disable, for future use
    bPixFmts[LAVOutPixFmt_YV16] = FALSE;

    dwHWAccel = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWAccel"), dwHWAccel);

    bHWFormats[HWCodec_H264] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("h264"), bHWFormats[HWCodec_H264]);

    bHWFormats[HWCodec_VC1] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("vc1"), bHWFormats[HWCodec_VC1]);

    bHWFormats[HWCodec_MPEG2] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("mpeg2"), bHWFormats[HWCodec_MPEG2]);

    bHWFormats[HWCodec_MPEG4] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("mpeg4"), bHWFormats[HWCodec_MPEG4]);

    bHWFormats[HWCodec_MPEG2DVD] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("dvd"), bHWFormats[HWCodec_MPEG2DVD]);

    dwHWAccelResFlags = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWResFlags"), dwHWAccelResFlags);

    dwHWDeintMode = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWDeintMode"), dwHWDeintMode);

    dwHWDeintOutput = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWDeintOutput"), dwHWDeintOutput);

    bHWDeintHQ = pApp->GetProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWDeintHQ"), bHWDeintHQ);
}

void CFGFilterLAVVideo::Settings::SaveSettings()
{
    CMPlayerCApp* pApp = AfxGetMyApp();
    ASSERT(pApp);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("TrayIcon"), bTrayIcon);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("StreamAR"), dwStreamAR);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("NumThreads"), dwNumThreads);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("DeintFieldOrder"), dwDeintFieldOrder);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("DeintMode"), deintMode);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("RGBRange"), dwRGBRange);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("SWDeintMode"), dwSWDeintMode);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("SWDeintOutput"), dwSWDeintOutput);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO, _T("DitherMode"), dwDitherMode);

    for (int i = 0; i < LAVOutPixFmt_NB; ++i) {
        pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_OUTPUTFORMAT, pixFmtSettingsMap[i], bPixFmts[i]);
    }

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWAccel"), dwHWAccel);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("h264"), bHWFormats[HWCodec_H264]);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("vc1"), bHWFormats[HWCodec_VC1]);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("mpeg2"), bHWFormats[HWCodec_MPEG2]);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("mpeg4"), bHWFormats[HWCodec_MPEG4]);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("dvd"), bHWFormats[HWCodec_MPEG2DVD]);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWResFlags"), dwHWAccelResFlags);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWDeintMode"), dwHWDeintMode);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWDeintOutput"), dwHWDeintOutput);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVVIDEO_HWACCEL, _T("HWDeintHQ"), bHWDeintHQ);
}

bool CFGFilterLAVVideo::Settings::GetSettings(CComQIPtr<ILAVVideoSettings> pLAVFSettings)
{
    if (!pLAVFSettings) {
        return false;
    }

    bTrayIcon = pLAVFSettings->GetTrayIcon();

    dwStreamAR = pLAVFSettings->GetStreamAR();

    dwNumThreads = pLAVFSettings->GetNumThreads();

    dwDeintFieldOrder = pLAVFSettings->GetDeintFieldOrder();

    deintMode = pLAVFSettings->GetDeinterlacingMode();

    dwRGBRange = pLAVFSettings->GetRGBOutputRange();

    dwSWDeintMode = pLAVFSettings->GetSWDeintMode();

    dwSWDeintOutput = pLAVFSettings->GetSWDeintOutput();

    dwDitherMode = pLAVFSettings->GetDitherMode();

    for (int i = 0; i < LAVOutPixFmt_NB; ++i) {
        bPixFmts[i] = pLAVFSettings->GetPixelFormat((LAVOutPixFmts)i);
    }

    dwHWAccel = pLAVFSettings->GetHWAccel();

    for (int i = 0; i < HWCodec_NB; ++i) {
        bHWFormats[i] = pLAVFSettings->GetHWAccelCodec((LAVVideoHWCodec)i);
    }

    dwHWAccelResFlags = pLAVFSettings->GetHWAccelResolutionFlags();

    dwHWDeintMode = pLAVFSettings->GetHWAccelDeintMode();

    dwHWDeintOutput = pLAVFSettings->GetHWAccelDeintOutput();

    bHWDeintHQ = pLAVFSettings->GetHWAccelDeintHQ();

    return true;
}

bool CFGFilterLAVVideo::Settings::SetSettings(CComQIPtr<ILAVVideoSettings> pLAVFSettings)
{
    if (!pLAVFSettings) {
        return false;
    }

    pLAVFSettings->SetTrayIcon(bTrayIcon);

    pLAVFSettings->SetStreamAR(dwStreamAR);

    pLAVFSettings->SetNumThreads(dwNumThreads);

    pLAVFSettings->SetDeintFieldOrder((LAVDeintFieldOrder)dwDeintFieldOrder);

    pLAVFSettings->SetDeinterlacingMode(deintMode);

    pLAVFSettings->SetRGBOutputRange(dwRGBRange);

    pLAVFSettings->SetSWDeintMode((LAVSWDeintModes)dwSWDeintMode);

    pLAVFSettings->SetSWDeintOutput((LAVDeintOutput)dwSWDeintOutput);

    pLAVFSettings->SetDitherMode((LAVDitherMode)dwDitherMode);

    for (int i = 0; i < LAVOutPixFmt_NB; ++i) {
        pLAVFSettings->SetPixelFormat((LAVOutPixFmts)i, bPixFmts[i]);
    }

    pLAVFSettings->SetHWAccel((LAVHWAccel)dwHWAccel);

    for (int i = 0; i < HWCodec_NB; ++i) {
        pLAVFSettings->SetHWAccelCodec((LAVVideoHWCodec)i, bHWFormats[i]);
    }

    pLAVFSettings->SetHWAccelResolutionFlags(dwHWAccelResFlags);

    pLAVFSettings->SetHWAccelDeintMode((LAVHWDeintModes)dwHWDeintMode);

    pLAVFSettings->SetHWAccelDeintOutput((LAVDeintOutput)dwHWDeintOutput);

    pLAVFSettings->SetHWAccelDeintHQ(bHWDeintHQ);

    return true;
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
        } else if (CComQIPtr<ILAVAudioSettings> pLAVFSettings = *ppBF) {
            // Take control over LAVAudio's settings, the settings are reseted to default values
            hr = pLAVFSettings->SetRuntimeConfig(TRUE);

            if (SUCCEEDED(hr)) {
                Settings settings;
                if (settings.GetSettings(pLAVFSettings)) { // Get default settings from LAVAudio
                    settings.LoadSettings(); // Load our current settings from registry/ini
                    settings.SetSettings(pLAVFSettings); // Set our settings in LAVAudio
                }

                // Keep track of LAVFilters instances in runtime mode
                s_instances.AddTail(*ppBF);
            }
        } else {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

static LPCTSTR bitstreamingCodecs[Bitstream_NB] = {
    _T("ac3"), _T("eac3"), _T("truehd"), _T("dts"), _T("dtshd")
};

static LPCTSTR sampleFormats[SampleFormat_Bitstream] = {
    _T("s16"), _T("s24"), _T("s32"), _T("u8"), _T("fp32")
};

void CFGFilterLAVAudio::Settings::LoadSettings()
{
    CMPlayerCApp* pApp = AfxGetMyApp();
    ASSERT(pApp);

    bTrayIcon = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("TrayIcon"), bTrayIcon);

    bDRCEnabled = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("DRCEnabled"), bDRCEnabled);

    iDRCLevel = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("DRCLevel"), iDRCLevel);

    bDTSHDFraming = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("DTSHDFraming"), bDTSHDFraming);

    bAutoAVSync = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("AutoAVSync"), bAutoAVSync);

    bExpandMono = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("ExpandMono"), bExpandMono);

    bExpand61 = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("Expand61"), bExpand61);

    bOutputStandardLayout = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("OutputStandardLayout"), bOutputStandardLayout);

    bMixingEnabled = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("Mixing"), bMixingEnabled);

    dwMixingLayout = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingLayout"), dwMixingLayout);

    dwMixingFlags = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingFlags"), dwMixingFlags);

    dwMixingMode = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingMode"), dwMixingMode);

    dwMixingCenterLevel = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingCenterLevel"), dwMixingCenterLevel);

    dwMixingSurroundLevel = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingSurroundLevel"), dwMixingSurroundLevel);

    dwMixingLFELevel = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingLFELevel"), dwMixingLFELevel);

    bAudioDelayEnabled = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("AudioDelayEnabled"), bAudioDelayEnabled);

    iAudioDelay = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("AudioDelay"), iAudioDelay);

    for (int i = 0; i < Bitstream_NB; ++i) {
        CString key = CString(_T("Bitstreaming_")) + bitstreamingCodecs[i];
        bBitstream[i] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, key, bBitstream[i]);
    }

    for (int i = 0; i < SampleFormat_Bitstream; ++i) {
        CString key = CString(_T("SampleFormat_")) + sampleFormats[i];
        bSampleFormats[i] = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, key, bSampleFormats[i]);
    }

    bSampleConvertDither = pApp->GetProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("SampleConvertDither"), bSampleConvertDither);
}

void CFGFilterLAVAudio::Settings::SaveSettings()
{
    CMPlayerCApp* pApp = AfxGetMyApp();
    ASSERT(pApp);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("TrayIcon"), bTrayIcon);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("DRCEnabled"), bDRCEnabled);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("DRCLevel"), iDRCLevel);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("DTSHDFraming"), bDTSHDFraming);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("AutoAVSync"), bAutoAVSync);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("ExpandMono"), bExpandMono);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("Expand61"), bExpand61);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("OutputStandardLayout"), bOutputStandardLayout);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("Mixing"), bMixingEnabled);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingLayout"), dwMixingLayout);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingFlags"), dwMixingFlags);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingMode"), dwMixingMode);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingCenterLevel"), dwMixingCenterLevel);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingSurroundLevel"), dwMixingSurroundLevel);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("MixingLFELevel"), dwMixingLFELevel);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("AudioDelayEnabled"), bAudioDelayEnabled);

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("AudioDelay"), iAudioDelay);

    for (int i = 0; i < Bitstream_NB; ++i) {
        CString key = CString(_T("Bitstreaming_")) + bitstreamingCodecs[i];
        pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, key, bBitstream[i]);
    }

    for (int i = 0; i < SampleFormat_Bitstream; ++i) {
        CString key = CString(_T("SampleFormat_")) + sampleFormats[i];
        pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, key, bSampleFormats[i]);
    }

    pApp->WriteProfileInt(IDS_R_INTERNAL_LAVAUDIO, _T("SampleConvertDither"), bSampleConvertDither);
}

bool CFGFilterLAVAudio::Settings::GetSettings(CComQIPtr<ILAVAudioSettings> pLAVFSettings)
{
    if (!pLAVFSettings) {
        return false;
    }

    bTrayIcon = pLAVFSettings->GetTrayIcon();

    pLAVFSettings->GetDRC(&bDRCEnabled, &iDRCLevel);

    bDTSHDFraming = pLAVFSettings->GetDTSHDFraming();

    bAutoAVSync = pLAVFSettings->GetAutoAVSync();

    bExpandMono = pLAVFSettings->GetExpandMono();

    bExpand61 = pLAVFSettings->GetExpand61();

    bOutputStandardLayout = pLAVFSettings->GetOutputStandardLayout();

    bMixingEnabled = pLAVFSettings->GetMixingEnabled();

    dwMixingLayout = pLAVFSettings->GetMixingLayout();

    dwMixingFlags = pLAVFSettings->GetMixingFlags();

    dwMixingMode = pLAVFSettings->GetMixingMode();

    pLAVFSettings->GetMixingLevels(&dwMixingCenterLevel, &dwMixingSurroundLevel, &dwMixingLFELevel);

    pLAVFSettings->GetAudioDelay(&bAudioDelayEnabled, &iAudioDelay);

    for (int i = 0; i < Bitstream_NB; ++i) {
        bBitstream[i] = pLAVFSettings->GetBitstreamConfig((LAVBitstreamCodec)i);
    }

    for (int i = 0; i < SampleFormat_Bitstream; ++i) {
        bSampleFormats[i] = pLAVFSettings->GetSampleFormat((LAVAudioSampleFormat)i);
    }

    bSampleConvertDither = pLAVFSettings->GetSampleConvertDithering();

    return true;
}

bool CFGFilterLAVAudio::Settings::SetSettings(CComQIPtr<ILAVAudioSettings> pLAVFSettings)
{
    if (!pLAVFSettings) {
        return false;
    }

    pLAVFSettings->SetTrayIcon(bTrayIcon);

    pLAVFSettings->SetDRC(bDRCEnabled, iDRCLevel);

    pLAVFSettings->SetDTSHDFraming(bDTSHDFraming);

    pLAVFSettings->SetAutoAVSync(bAutoAVSync);

    pLAVFSettings->SetExpandMono(bExpandMono);

    pLAVFSettings->SetExpand61(bExpand61);

    pLAVFSettings->SetOutputStandardLayout(bOutputStandardLayout);

    pLAVFSettings->SetMixingEnabled(bMixingEnabled);

    pLAVFSettings->SetMixingLayout(dwMixingLayout);

    pLAVFSettings->SetMixingFlags(dwMixingFlags);

    pLAVFSettings->SetMixingMode((LAVAudioMixingMode)dwMixingMode);

    pLAVFSettings->SetMixingLevels(dwMixingCenterLevel, dwMixingSurroundLevel, dwMixingLFELevel);

    pLAVFSettings->SetAudioDelay(bAudioDelayEnabled, iAudioDelay);

    for (int i = 0; i < Bitstream_NB; ++i) {
        pLAVFSettings->SetBitstreamConfig((LAVBitstreamCodec)i, bBitstream[i]);
    }

    for (int i = 0; i < SampleFormat_Bitstream; ++i) {
        pLAVFSettings->SetSampleFormat((LAVAudioSampleFormat)i, bSampleFormats[i]);
    }

    pLAVFSettings->SetSampleConvertDithering(bSampleConvertDither);

    return true;
}
