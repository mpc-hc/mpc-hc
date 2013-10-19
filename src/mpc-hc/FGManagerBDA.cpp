/*
 * (C) 2009-2013 see Authors.txt
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

#include <ks.h>
#include <ksmedia.h>
#include "BaseClasses/streams.h"
#include <mpeg2data.h>
#include <tuner.h>
#include <time.h>
#include <dvbsiparser.h>

#include "DSUtil.h"
#include "GolombBuffer.h"
#include "../filters/switcher/AudioSwitcher/AudioSwitcher.h"
#include "moreuuids.h"
#include "mplayerc.h"
#include "FGManagerBDA.h"
#include "DVBChannel.h"
#include "Mpeg2SectionData.h"
#include "MainFrm.h"
#include "SysVersion.h"


/// Format, Video MPEG2
static VIDEOINFOHEADER2 sMpv_fmt = {
    {0, 0, 720, 576},               // rcSource
    {0, 0, 0, 0},                   // rcTarget
    0,                              // dwBitRate
    0,                              // dwBitErrorRate
    400000,                         // AvgTimePerFrame
    0,                              // dwInterlaceFlags
    0,                              // dwCopyProtectFlags
    0,                              // dwPictAspectRatioX
    0,                              // dwPictAspectRatioY
    {0},                            // dwControlFlag & dwReserved1
    0,                              // dwReserved2
    {
        // bmiHeader
        sizeof(BITMAPINFOHEADER),   // biSize
        720,                        // biWidth
        576,                        // biHeight
        0,                          // biPlanes
        0,                          // biBitCount
        0                           // biCompression
    }
    // implicitly sets the others fields to 0
};

/// Media type, Video MPEG2
static AM_MEDIA_TYPE mt_Mpv = {
    MEDIATYPE_Video,                // majortype
    MEDIASUBTYPE_MPEG2_VIDEO,       // subtype
    FALSE,                          // bFixedSizeSamples
    TRUE,                           // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_VideoInfo2,              // formattype
    nullptr,                        // pUnk
    sizeof(sMpv_fmt),               // cbFormat
    (LPBYTE)& sMpv_fmt              // pbFormat
};

#define FCC_h264 MAKEFOURCC('h', '2', '6', '4')

/// Format, Video H264
static VIDEOINFOHEADER2 vih2_H264 = {
    {0, 0, 0, 0},                   // rcSource
    {0, 0, 0, 0},                   // rcTarget
    0,                              // dwBitRate,
    0,                              // dwBitErrorRate
    0,                              // AvgTimePerFrame
    0,                              // dwInterlaceFlags
    0,                              // dwCopyProtectFlags
    0,                              // dwPictAspectRatioX
    0,                              // dwPictAspectRatioY
    {0},                            // dwControlFlag & dwReserved1
    0,                              // dwReserved2
    {
        // bmiHeader
        sizeof(BITMAPINFOHEADER),   // biSize
        //720,                        // biWidth
        //576,                        // biHeight
        1920,                       // biWidth
        1080,                       // biHeight
        1,                          // biPlanes
        0,                          // biBitCount
        FCC_h264                    // biCompression
    }
    // implicitly sets the others fields to 0
};

/// Media type, Video H264
static AM_MEDIA_TYPE mt_H264 = {
    MEDIATYPE_Video,                // majortype
    MEDIASUBTYPE_H264,              // subtype
    FALSE,                          // bFixedSizeSamples
    TRUE,                           // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_VideoInfo2,              // formattype
    nullptr,                        // pUnk
    sizeof(vih2_H264),              // cbFormat
    (LPBYTE)& vih2_H264             // pbFormat
};

/// Format, Audio (common)
static const WAVEFORMATEX wf_Audio = {
    WAVE_FORMAT_PCM,                // wFormatTag
    2,                              // nChannels
    48000,                          // nSamplesPerSec
    4 * 48000,                      // nAvgBytesPerSec
    4,                              // nBlockAlign
    16,                             // wBitsPerSample
    0                               // cbSize
};

/// Media type, Audio MPEG2
static const AM_MEDIA_TYPE mt_Mpa = {
    MEDIATYPE_Audio,                // majortype
    MEDIASUBTYPE_MPEG2_AUDIO,       // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_WaveFormatEx,            // formattype
    nullptr,                        // pUnk
    sizeof(wf_Audio),               // cbFormat
    (LPBYTE)& wf_Audio              // pbFormat
};

/// Media type, Audio AC3
static const AM_MEDIA_TYPE mt_Ac3 = {
    MEDIATYPE_Audio,                // majortype
    MEDIASUBTYPE_DOLBY_AC3,         // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_WaveFormatEx,            // formattype
    nullptr,                        // pUnk
    sizeof(wf_Audio),               // cbFormat
    (LPBYTE)& wf_Audio,             // pbFormat
};

/// Media type, Audio EAC3
static const AM_MEDIA_TYPE mt_Eac3 = {
    MEDIATYPE_Audio,                // majortype
    MEDIASUBTYPE_DOLBY_DDPLUS,      // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_WaveFormatEx,            // formattype
    nullptr,                        // pUnk
    sizeof(wf_Audio),               // cbFormat
    (LPBYTE)& wf_Audio,             // pbFormat
};

/// Media type, Audio AAC LATM
static const AM_MEDIA_TYPE mt_latm = {
    MEDIATYPE_Audio,                // majortype
    MEDIASUBTYPE_LATM_AAC,          // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_WaveFormatEx,            // formattype
    nullptr,                        // pUnk
    sizeof(wf_Audio),               // cbFormat
    (LPBYTE)& wf_Audio,             // pbFormat
};

/// Media type, PSI
static const AM_MEDIA_TYPE mt_Psi = {
    MEDIATYPE_MPEG2_SECTIONS,       // majortype
    MEDIASUBTYPE_MPEG2DATA,         // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_None,                    // formattype
    nullptr,                        // pUnk
    0,                              // cbFormat
    nullptr                         // pbFormat
};

/// Media type, TIF
static const AM_MEDIA_TYPE mt_Tif = {
    MEDIATYPE_MPEG2_SECTIONS,       // majortype
    MEDIASUBTYPE_DVB_SI,            // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_None,                    // formattype
    nullptr,                        // pUnk
    0,                              // cbFormat
    nullptr                         // pbFormat
};

/// Media type, EPG
static const AM_MEDIA_TYPE mt_Epg = {
    MEDIATYPE_MPEG2_SECTIONS,       // majortype
    MEDIASUBTYPE_DVB_SI,            // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_None,                    // formattype
    nullptr,                        // pUnk
    0,                              // cbFormat
    nullptr,                        // pbFormat
};

/// Media type, PMT
static const AM_MEDIA_TYPE mt_Pmt = {
    MEDIATYPE_MPEG2_SECTIONS,       // majortype
    MEDIASUBTYPE_DVB_SI,            // subtype
    TRUE,                           // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_None,                    // formattype
    nullptr,                        // pUnk
    0,                              // cbFormat
    nullptr                         // pbFormat
};

static const SUBTITLEINFO SubFormat = { 0, "", L"" };

/// Media type, subtitle
static const AM_MEDIA_TYPE mt_Subtitle = {
    MEDIATYPE_Subtitle,             // majortype
    MEDIASUBTYPE_DVB_SUBTITLES,     // subtype
    FALSE,                          // bFixedSizeSamples
    FALSE,                          // bTemporalCompression
    0,                              // lSampleSize
    FORMAT_None,                    // formattype
    nullptr,                        // pUnk
    sizeof(SubFormat),              // cbFormat
    (LPBYTE)& SubFormat             // pbFormat
};

/// CLSID for TIF
// FC772AB0-0C7F-11D3-8FF2-00A0C9224CF4
static CLSID CLSID_BDA_MPEG2_TIF =
{0xFC772AB0, 0x0C7F, 0x11D3, {0x8F, 0xF2, 0x00, 0xA0, 0xC9, 0x22, 0x4C, 0xF4}};

CFGManagerBDA::CFGManagerBDA(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd)
    : CFGManagerPlayer(pName, pUnk, hWnd)
{
    LOG(_T("---------------------------------------------------------------->"));
    LOG(_T("Starting session..."));
    CAppSettings& s = AfxGetAppSettings();
    CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
    if (pChannel) {
        if (pChannel->GetVideoType() == DVB_H264) {
            UpdateMediaType(&vih2_H264, pChannel);
        } else if (pChannel->GetVideoType() == DVB_MPV) {
            UpdateMediaType(&sMpv_fmt, pChannel);
        }
    }
    m_DVBStreams[DVB_MPV]  = CDVBStream(L"mpv",  &mt_Mpv);
    m_DVBStreams[DVB_H264] = CDVBStream(L"h264", &mt_H264);
    m_DVBStreams[DVB_MPA]  = CDVBStream(L"mpa",  &mt_Mpa);
    m_DVBStreams[DVB_AC3]  = CDVBStream(L"ac3",  &mt_Ac3);
    m_DVBStreams[DVB_EAC3] = CDVBStream(L"eac3", &mt_Eac3);
    m_DVBStreams[DVB_LATM] = CDVBStream(L"latm", &mt_latm);
    m_DVBStreams[DVB_PSI]  = CDVBStream(L"psi",  &mt_Psi, true, MEDIA_MPEG2_PSI);
    m_DVBStreams[DVB_TIF]  = CDVBStream(L"tif",  &mt_Tif, true);
    m_DVBStreams[DVB_EPG]  = CDVBStream(L"epg",  &mt_Epg);

    // Warning : MEDIA_ELEMENTARY_STREAM didn't work for subtitles with Windows XP!
    if (SysVersion::IsVistaOrLater()) {
        m_DVBStreams[DVB_SUB] = CDVBStream(L"sub", &mt_Subtitle/*, false, MEDIA_TRANSPORT_PAYLOAD*/);
    } else {
        m_DVBStreams[DVB_SUB] = CDVBStream(L"sub", &mt_Subtitle, false, MEDIA_TRANSPORT_PAYLOAD);
    }

    if (pChannel) {
        m_nCurVideoType = pChannel->GetVideoType();
        m_nCurAudioType = pChannel->GetDefaultAudioType();
    } else {
        m_nCurVideoType = DVB_MPV;
        m_nCurAudioType = DVB_MPA;
    }
    m_fHideWindow = false;

    // Blacklist some unsupported filters (AddHead must be used to ensure higher priority):
    //  - audio switcher
    m_transform.AddHead(DEBUG_NEW CFGFilterRegistry(__uuidof(CAudioSwitcherFilter), MERIT64_DO_NOT_USE));
    //  - internal video decoder and ffdshow DXVA video decoder (cf ticket #730)
    m_transform.AddHead(DEBUG_NEW CFGFilterRegistry(CLSID_MPCVideoDecoder, MERIT64_DO_NOT_USE));
    m_transform.AddHead(DEBUG_NEW CFGFilterRegistry(CLSID_FFDShowDXVADecoder, MERIT64_DO_NOT_USE));
    //  - Microsoft DTV-DVD Audio Decoder
    m_transform.AddHead(DEBUG_NEW CFGFilterRegistry(CLSID_MSDVTDVDAudioDecoder, MERIT64_DO_NOT_USE));
    //  - ACM Wrapper
    m_transform.AddHead(DEBUG_NEW CFGFilterRegistry(CLSID_ACMWrapper, MERIT64_DO_NOT_USE));
    //  - ReClock
    m_transform.AddHead(DEBUG_NEW CFGFilterRegistry(CLSID_ReClock, MERIT64_DO_NOT_USE));

    LOG(_T("CFGManagerBDA object created."));
}

CFGManagerBDA::~CFGManagerBDA()
{
    m_DVBStreams.RemoveAll();
    LOG(_T("CFGManagerBDA object destroyed."));
    LOG(_T("<----------------------------------------------------------------\n\n"));
}

HRESULT CFGManagerBDA::CreateKSFilter(IBaseFilter** ppBF, CLSID KSCategory, const CStringW& DisplayName)
{
    HRESULT hr = VFW_E_NOT_FOUND;
    BeginEnumSysDev(KSCategory, pMoniker) {
        CComPtr<IPropertyBag> pPB;
        CComVariant var;
        LPOLESTR strName = nullptr;
        if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPB))) &&
                SUCCEEDED(pMoniker->GetDisplayName(nullptr, nullptr, &strName)) &&
                SUCCEEDED(pPB->Read(CComBSTR(_T("FriendlyName")), &var, nullptr))) {
            CStringW Name = CStringW(strName);
            if (Name != DisplayName) {
                continue;
            }

            hr = pMoniker->BindToObject(nullptr, nullptr, IID_PPV_ARGS(ppBF));
            if (SUCCEEDED(hr)) {
                hr = AddFilter(*ppBF, CStringW(var.bstrVal));
            }
            break;
        }

        if (strName) {
            CoTaskMemFree(strName);
        }
    }
    EndEnumSysDev;

    return hr;
}

HRESULT CFGManagerBDA::SearchIBDATopology(const CComPtr<IBaseFilter>& pTuner, REFIID iid, CComPtr<IUnknown>& pUnk)
{
    CComQIPtr<IBDA_Topology> pTop(pTuner);
    CheckPointer(pTop, E_NOINTERFACE);

    ULONG NodeTypes;
    ULONG NodeType[32];

    HRESULT hr = pTop->GetNodeTypes(&NodeTypes, _countof(NodeType), NodeType);

    if (FAILED(hr)) {
        return hr;
    }

    for (ULONG i = 0; i < NodeTypes; i++) {
        ULONG nInterfaces;
        GUID  aInterface[32];

        hr = pTop->GetNodeInterfaces(NodeType[i], &nInterfaces, _countof(aInterface), aInterface);

        if (FAILED(hr)) {
            continue;
        }

        for (ULONG j = 0; j < nInterfaces; j++) {
            if (aInterface[j] == iid) {
                return pTop->GetControlNode(0, 1, NodeType[i], &pUnk);
            }
        }
    }

    return FAILED(hr) ? hr : E_NOINTERFACE;
}

HRESULT CFGManagerBDA::ConnectFilters(IBaseFilter* pOutFilter, IBaseFilter* pInFilter)
{
    HRESULT hr = VFW_E_CANNOT_CONNECT;
    BeginEnumPins(pOutFilter, pEP, pOutPin) {
        if (S_OK == IsPinDirection(pOutPin, PINDIR_OUTPUT)
                && S_OK != IsPinConnected(pOutPin)) {
            BeginEnumPins(pInFilter, pEP, pInPin) {
                if (S_OK == IsPinDirection(pInPin, PINDIR_INPUT)
                        && S_OK != IsPinConnected(pInPin)) {
                    hr = this->ConnectDirect(pOutPin, pInPin, nullptr);

                    /*#ifdef _DEBUG
                    PIN_INFO InfoPinIn, InfoPinOut;
                    FILTER_INFO InfoFilterIn, InfoFilterOut;
                    pInPin->QueryPinInfo (&InfoPinIn);
                    pOutPin->QueryPinInfo (&InfoPinOut);
                    InfoPinIn.pFilter->QueryFilterInfo(&InfoFilterIn);
                    InfoPinOut.pFilter->QueryFilterInfo(&InfoFilterOut);

                    TRACE(_T("%s - %s => %s - %s (hr=0x%08x)\n"), InfoFilterOut.achName, InfoPinOut.achName, InfoFilterIn.achName, InfoPinIn.achName, hr);

                    InfoPinIn.pFilter->Release();
                    InfoPinOut.pFilter->Release();
                    #endif*/
                    if (SUCCEEDED(hr)) {
                        return hr;
                    }
                }
            }
            EndEnumPins;
        }
    }
    EndEnumPins;

    return hr;
}

STDMETHODIMP CFGManagerBDA::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
    HRESULT hr = S_OK;
    const CAppSettings& s = AfxGetAppSettings();
    CComPtr<IBaseFilter> pNetwork;
    CComPtr<IBaseFilter> pTuner;
    CComPtr<IBaseFilter> pReceiver;

    m_nDVBRebuildFilterGraph = s.nDVBRebuildFilterGraph;
    LOG(_T("Creating BDA filters..."));
    CheckAndLog(CreateKSFilter(&pNetwork, KSCATEGORY_BDA_NETWORK_PROVIDER, s.strBDANetworkProvider), _T("BDA: Network provider creation"));
    if (FAILED(hr = CreateKSFilter(&pTuner, KSCATEGORY_BDA_NETWORK_TUNER, s.strBDATuner))) {
        MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, ResStr(IDS_BDA_ERROR_CREATE_TUNER), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
        TRACE(_T("BDA: Network tuner creation: 0x%08x\n"), hr);
        LOG(_T("Network tuner creation: 0x%08x"), hr);
        return hr;
    }

    if (FAILED(hr = ConnectFilters(pNetwork, pTuner))) {
        MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, ResStr(IDS_BDA_ERROR_CONNECT_TUNER), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
        TRACE(_T("BDA: Network <-> Tuner: 0x%08x\n"), hr);
        LOG(_T("Network <-> Tuner: 0x%08x"), hr);
        return hr;
    }
    m_pBDAControl = pTuner;

    if (FAILED(hr = SearchIBDATopology(pTuner, m_pBDAFreq))) {
        MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, _T("IBDA_FrequencyFilter topology failed."), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
        LOG(_T("IBDA_FrequencyFilter topology failed."));
        return hr;
    }
    m_pBDATunerStats = m_pBDAFreq;
    if (FAILED(hr = SearchIBDATopology(pTuner, m_pBDADemodulator))) {
        TRACE(_T("BDA: IBDA_DigitalDemodulator topology failed: 0x%08x\n"), hr);
        LOG(_T("IBDA_DigitalDemodulator topology failed. Result 0x%08x"), hr);
    }
    m_pBDADemodStats = m_pBDADemodulator;

    if (!m_pBDATunerStats || !m_pBDADemodStats) {
        if (m_pBDATunerStats) {
            TRACE(_T("BDA: no statistics interface on the demodulator node --> using the statistics from the RF node only\n"));
            LOG(_T("No statistics interface on the demodulator node --> using the statistics from the RF node only."));
            m_pBDADemodStats = m_pBDATunerStats;
        } else if (m_pBDADemodStats) {
            TRACE(_T("BDA: no statistics interface on the RF node --> using the statistics from the demodulator node only\n"));
            LOG(_T("No statistics interface on the RF node --> using the statistics from the demodulator node only."));
            m_pBDATunerStats = m_pBDADemodStats;
        } else { // if (!m_pBDATunerStats && !m_pBDADemodStats)
            MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, _T("No statistics interface available."), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
            TRACE(_T("BDA: Not statistics interface available\n"));
            LOG(_T("Not statistics interface available."));
            return E_NOINTERFACE;
        }
    }

    if (SUCCEEDED(hr = SearchIBDATopology(pTuner, m_pBDAAutoDemulate))) {
        if (SUCCEEDED(hr = m_pBDAAutoDemulate->put_AutoDemodulate())) {
            LOG(_T("Auto Demulate is on."));
        } else {
            LOG(_T("Auto Demulate could not be switched: 0x%08x."), hr);
        }
    } else {
        LOG(_T("AutoDemulate topology not found."));
        TRACE(_T("BDA: AutoDemulate topology not found: 0x%08x\n"), hr);
    }

    if (FAILED(hr = m_pDemux.CoCreateInstance(CLSID_MPEG2Demultiplexer, nullptr, CLSCTX_INPROC_SERVER))) {
        MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, ResStr(IDS_BDA_ERROR_DEMULTIPLEXER), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
        TRACE(_T("BDA: Microsoft demux creation: 0x%08x\n"), hr);
        return hr;
    }
    CheckNoLog(AddFilter(m_pDemux, _T("MPEG-2 Demultiplexer")));
    if (FAILED(ConnectFilters(pTuner, m_pDemux))) { // Separate receiver is required
        if (FAILED(hr = CreateKSFilter(&pReceiver, KSCATEGORY_BDA_RECEIVER_COMPONENT, s.strBDAReceiver))) {
            MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, ResStr(IDS_BDA_ERROR_CREATE_RECEIVER), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
            TRACE(_T("BDA: Receiver creation: 0x%08x\n"), hr);
            return hr;
        }
        if (FAILED(hr = ConnectFilters(pTuner, pReceiver))) {
            MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, ResStr(IDS_BDA_ERROR_CONNECT_TUNER_REC), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
            TRACE(_T("BDA: Tuner <-> Receiver: 0x%08x\n"), hr);
            return hr;
        }
        if (FAILED(ConnectFilters(pReceiver, m_pDemux))) {
            MessageBox(AfxGetMyApp()->GetMainWnd()->m_hWnd, ResStr(IDS_BDA_ERROR_DEMULTIPLEXER), ResStr(IDS_BDA_ERROR), MB_ICONERROR | MB_OK);
            TRACE(_T("BDA: Receiver <-> Demux: 0x%08x\n"), hr);
            return hr;
        }
        LOG(_T("Network -> Tuner -> Receiver connected."));

    } else { // The selected filter is performing both tuner and receiver functions
        LOG(_T("Network -> Receiver connected."));
    }

    CheckNoLog(CreateMicrosoftDemux(m_pDemux));

#ifdef _DEBUG
    LOG(_T("Filter list:"));
    BeginEnumFilters(this, pEF, pBF) {
        LOG(_T("  ") + GetFilterName(pBF));
    }
    EndEnumFilters;
    LOG(_T("Filter list end.\n"));
#endif

    return hr;
}

STDMETHODIMP CFGManagerBDA::ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt)
{
    // Bypass CFGManagerPlayer limitation (IMediaSeeking for Mpeg2 demux)
    return CFGManagerCustom::ConnectDirect(pPinOut, pPinIn, pmt);
}

STDMETHODIMP CFGManagerBDA::SetChannel(int nChannelPrefNumber)
{
    HRESULT hr = E_INVALIDARG;
    CAppSettings& s = AfxGetAppSettings();

    CDVBChannel* pChannel = s.FindChannelByPref(nChannelPrefNumber);
    LOG(_T("Start SetChannel %d."), nChannelPrefNumber);
    if (pChannel) {
        if ((m_nDVBRebuildFilterGraph == DVB_REBUILD_FG_NEVER) ||
                ((m_nDVBRebuildFilterGraph == DVB_REBUILD_FG_WHEN_SWITCHING) && (m_nCurVideoType == pChannel->GetVideoType())) ||
                ((m_nDVBRebuildFilterGraph == DVB_REBUILD_FG_ALWAYS) && (s.nDVBLastChannel == nChannelPrefNumber))) {
            hr = SetChannelInternal(pChannel);
        } else {
            s.nDVBLastChannel = nChannelPrefNumber;
            return S_FALSE;
        }

        if (SUCCEEDED(hr)) {
            s.nDVBLastChannel = nChannelPrefNumber;
            m_nCurVideoType = pChannel->GetVideoType();
            m_nCurAudioType = pChannel->GetDefaultAudioType();
            LOG(_T("SetChannel %d successful.\n"), nChannelPrefNumber);
        } else {
            LOG(_T("SetChannel %d failed. Result: 0x%08x.\n"), nChannelPrefNumber, hr);
        }
    }
    return hr;
}

STDMETHODIMP CFGManagerBDA::SetAudio(int nAudioIndex)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFGManagerBDA::SetFrequency(ULONG freq)
{
    HRESULT hr;
    const CAppSettings& s = AfxGetAppSettings();
    LOG(_T("SetFrequency to %d."), freq);
    CheckPointer(m_pBDAControl, E_FAIL);
    CheckPointer(m_pBDAFreq, E_FAIL);

    CheckAndLog(m_pBDAControl->StartChanges(), _T("BDA: SetFrequency StartChanges"));
    m_pBDAFreq->put_FrequencyMultiplier(1000);
    CheckAndLog(m_pBDAFreq->put_Bandwidth(s.iBDABandwidth), _T("BDA: SetFrequency put_Bandwidth"));
    CheckAndLog(m_pBDAFreq->put_Frequency(freq), _T("BDA: SetFrequency put_Frequency"));
    CheckAndLog(m_pBDAControl->CheckChanges(), _T("BDA: SetFrequency CheckChanges"));
    CheckAndLog(m_pBDAControl->CommitChanges(), _T("BDA: SetFrequency CommitChanges"));

    int i = 50;
    ULONG pState = BDA_CHANGES_PENDING;
    while (SUCCEEDED(hr = m_pBDAControl->GetChangeState(&pState)) && pState == BDA_CHANGES_PENDING && i-- > 0) {
        LOG(_T("SetFrequency changes pending, waiting for tuner..."));
        Sleep(50);
    }

    if (SUCCEEDED(hr)) {
        if (pState == BDA_CHANGES_PENDING) {
            LOG(_T("SetFrequency changes pending (timeout error----)"));
            hr = VFW_E_TIMEOUT;
        } else {
            LOG(_T("Frequency set to %d."), freq);
#ifdef _DEBUG
            BOOLEAN bPresent;
            BOOLEAN bLocked;
            LONG lDbStrength;
            LONG lPercentQuality;

            if (SUCCEEDED(GetStats(bPresent, bLocked, lDbStrength, lPercentQuality)) && bPresent) {
                LOG(_T("Signal stats: Strength %d dB, Quality %d%%"), lDbStrength, lPercentQuality);
            }
#endif
        }
    } else {
        LOG(_T("Frequency change failed. Result: 0x%08x."), hr);
    }

    return hr;
}

HRESULT CFGManagerBDA::ClearMaps()
{
    HRESULT hr = S_OK;

    if (m_DVBStreams[DVB_MPV].GetMappedPID()) {
        CheckNoLog(m_DVBStreams[DVB_MPV].Unmap(m_DVBStreams[DVB_MPV].GetMappedPID()));
    }
    if (m_DVBStreams[DVB_H264].GetMappedPID()) {
        CheckNoLog(m_DVBStreams[DVB_H264].Unmap(m_DVBStreams[DVB_H264].GetMappedPID()));
    }
    if (m_DVBStreams[DVB_MPA].GetMappedPID()) {
        CheckNoLog(m_DVBStreams[DVB_MPA].Unmap(m_DVBStreams[DVB_MPA].GetMappedPID()));
    }
    if (m_DVBStreams[DVB_AC3].GetMappedPID()) {
        CheckNoLog(m_DVBStreams[DVB_AC3].Unmap(m_DVBStreams[DVB_AC3].GetMappedPID()));
    }
    if (m_DVBStreams[DVB_EAC3].GetMappedPID()) {
        CheckNoLog(m_DVBStreams[DVB_EAC3].Unmap(m_DVBStreams[DVB_EAC3].GetMappedPID()));
    }
    if (m_DVBStreams[DVB_LATM].GetMappedPID()) {
        CheckNoLog(m_DVBStreams[DVB_LATM].Unmap(m_DVBStreams[DVB_LATM].GetMappedPID()));
    }
    if (m_DVBStreams[DVB_SUB].GetMappedPID()) {
        CheckNoLog(m_DVBStreams[DVB_SUB].Unmap(m_DVBStreams[DVB_SUB].GetMappedPID()));
    }

    return hr;
}

STDMETHODIMP CFGManagerBDA::Scan(ULONG ulFrequency, HWND hWnd)
{
    HRESULT hr = S_OK;

    if (ulFrequency == 0) {
        ClearMaps();
    } else {
        CMpeg2DataParser Parser(m_DVBStreams[DVB_PSI].GetFilter());

        LOG(_T("Scanning frequency %d.........."), ulFrequency);

        if (FAILED(hr = Parser.ParseSDT(ulFrequency))) {
            LOG(_T("ParseSDT failed. Result: 0x%08x."), hr);
        } else if (FAILED(hr = Parser.ParsePAT())) {
            LOG(_T("ParsePAT failed. Result: 0x%08x."), hr);
        } else if (FAILED(hr = Parser.ParseNIT())) {
            LOG(_T("ParseNIT failed. Result: 0x%08x."), hr);
        }

        POSITION pos = Parser.Channels.GetStartPosition();
        while (pos) {
            CDVBChannel& Channel = Parser.Channels.GetNextValue(pos);
            if (Channel.HasName()) {
                ::SendMessage(hWnd, WM_TUNER_NEW_CHANNEL, 0, (LPARAM)(LPCTSTR)Channel.ToString());
            }
        }
        LOG(_T("Scanning frequency %d done."), ulFrequency);
    }

    return hr;
}

STDMETHODIMP CFGManagerBDA::GetStats(BOOLEAN& bPresent, BOOLEAN& bLocked, LONG& lDbStrength, LONG& lPercentQuality)
{
    HRESULT hr = S_OK;
    CheckPointer(m_pBDATunerStats, E_UNEXPECTED);
    CheckPointer(m_pBDADemodStats, E_UNEXPECTED);

    if (FAILED(hr = m_pBDATunerStats->get_SignalPresent(&bPresent)) && FAILED(hr = m_pBDADemodStats->get_SignalPresent(&bPresent))) {
        return hr;
    }
    if (FAILED(hr = m_pBDADemodStats->get_SignalLocked(&bLocked)) && FAILED(hr = m_pBDATunerStats->get_SignalLocked(&bLocked))) {
        return hr;
    }
    if (FAILED(hr = m_pBDATunerStats->get_SignalStrength(&lDbStrength)) && FAILED(hr = m_pBDADemodStats->get_SignalStrength(&lDbStrength))) {
        return hr;
    }
    if (FAILED(hr = m_pBDADemodStats->get_SignalQuality(&lPercentQuality)) && FAILED(hr = m_pBDATunerStats->get_SignalQuality(&lPercentQuality))) {
        return hr;
    }

    return hr;
}

// IAMStreamSelect
STDMETHODIMP CFGManagerBDA::Count(DWORD* pcStreams)
{
    CheckPointer(pcStreams, E_POINTER);
    CAppSettings& s = AfxGetAppSettings();
    CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);

    *pcStreams = 0;

    if (pChannel) {
        int Streams = pChannel->GetAudioCount() + pChannel->GetSubtitleCount();
        *pcStreams = pChannel->GetSubtitleCount() ? Streams + 1 : Streams;
    }

    return S_OK;
}

STDMETHODIMP CFGManagerBDA::Enable(long lIndex, DWORD dwFlags)
{
    HRESULT hr = E_INVALIDARG;
    CAppSettings& s = AfxGetAppSettings();
    CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
    DVBStreamInfo* pStreamInfo = nullptr;
    CDVBStream* pStream = nullptr;
    FILTER_STATE nState;

    if (pChannel) {
        if (lIndex >= 0 && lIndex < pChannel->GetAudioCount()) {
            pStreamInfo = pChannel->GetAudio(lIndex);
            if (pStreamInfo) {
                pStream = &m_DVBStreams[pStreamInfo->Type];
                if (pStream) {
                    if (pStream->GetMappedPID()) {
                        pStream->Unmap(pStream->GetMappedPID());
                    }
                    nState = GetState();
                    if (m_nCurAudioType != pStreamInfo->Type) {
                        if ((s.nDVBStopFilterGraph == DVB_STOP_FG_ALWAYS) && (GetState() != State_Stopped)) {
                            ChangeState(State_Stopped);
                        }
                        SwitchStream(m_nCurAudioType, pStreamInfo->Type);
                        m_nCurAudioType = pStreamInfo->Type;
                        CheckNoLog(Flush(m_nCurVideoType, m_nCurAudioType));
                    }
                    pStream->Map(pStreamInfo->PID);
                    ChangeState((FILTER_STATE)nState);

                    hr = S_OK;
                } else {
                    ASSERT(FALSE);
                }
            } else {
                ASSERT(FALSE);
            }
        } else if (lIndex > 0 && lIndex < pChannel->GetAudioCount() + pChannel->GetSubtitleCount()) {
            pStreamInfo = pChannel->GetSubtitle(lIndex - pChannel->GetAudioCount());

            if (pStreamInfo) {
                m_DVBStreams[DVB_SUB].Map(pStreamInfo->PID);
                hr = S_OK;
            }
        } else if (lIndex > 0 && m_DVBStreams[DVB_SUB].GetMappedPID() && lIndex == pChannel->GetAudioCount() + pChannel->GetSubtitleCount()) {
            m_DVBStreams[DVB_SUB].Unmap(m_DVBStreams[DVB_SUB].GetMappedPID());
            hr = S_OK;
        }
    } else {
        ASSERT(FALSE);
    }

    return hr;
}

STDMETHODIMP CFGManagerBDA::Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk)
{
    HRESULT hr = E_INVALIDARG;
    CAppSettings& s = AfxGetAppSettings();
    CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
    DVBStreamInfo* pStreamInfo = nullptr;
    CDVBStream* pStream = nullptr;
    CDVBStream* pCurrentStream = nullptr;

    if (pChannel) {
        if (lIndex >= 0 && lIndex < pChannel->GetAudioCount()) {
            pCurrentStream = &m_DVBStreams[m_nCurAudioType];
            pStreamInfo = pChannel->GetAudio(lIndex);
            if (pStreamInfo) {
                pStream = &m_DVBStreams[pStreamInfo->Type];
            }
            if (pdwGroup) {
                *pdwGroup = 1;    // Audio group
            }
        } else if (lIndex > 0 && lIndex < pChannel->GetAudioCount() + pChannel->GetSubtitleCount()) {
            pCurrentStream = &m_DVBStreams[DVB_SUB];
            pStreamInfo = pChannel->GetSubtitle(lIndex - pChannel->GetAudioCount());
            if (pStreamInfo) {
                pStream = &m_DVBStreams[pStreamInfo->Type];
            }
            if (pdwGroup) {
                *pdwGroup = 2;    // Subtitle group
            }
        } else if (lIndex > 0 && lIndex == pChannel->GetAudioCount() + pChannel->GetSubtitleCount()) {
            pCurrentStream = &m_DVBStreams[DVB_SUB];

            if (pCurrentStream) {
                if (pdwFlags) {
                    *pdwFlags = (!pCurrentStream->GetMappedPID()) ? AMSTREAMSELECTINFO_ENABLED | AMSTREAMSELECTINFO_EXCLUSIVE : 0;
                }
                if (plcid) {
                    *plcid  = (LCID)LCID_NOSUBTITLES;
                }
                if (pdwGroup) {
                    *pdwGroup = 2;    // Subtitle group
                }
                if (ppszName) {
                    CStringW str;
                    str = _T("No subtitles");

                    *ppszName = (WCHAR*)CoTaskMemAlloc((str.GetLength() + 1) * sizeof(WCHAR));
                    if (*ppszName == nullptr) {
                        return E_OUTOFMEMORY;
                    }
                    wcscpy_s(*ppszName, str.GetLength() + 1, str);
                }
            }

            return S_OK;
        }

        if (pStreamInfo && pStream && pCurrentStream) {
            if (ppmt) {
                *ppmt = CreateMediaType(pStream->GetMediaType());
            }
            if (pdwFlags) {
                *pdwFlags = (pCurrentStream->GetMappedPID() == pStreamInfo->PID) ? AMSTREAMSELECTINFO_ENABLED | AMSTREAMSELECTINFO_EXCLUSIVE : 0;
            }
            if (plcid) {
                *plcid = pStreamInfo->GetLCID();
            }
            if (ppObject) {
                *ppObject = nullptr;
            }
            if (ppUnk) {
                *ppUnk = nullptr;
            }
            if (ppszName) {
                CStringW str;

                str = StreamTypeToName(pStreamInfo->PesType);

                if (!pStreamInfo->Language.IsEmpty() && pStreamInfo->GetLCID() == 0) {
                    // Try to convert language code even if LCID was not found.
                    str += _T(" [") + ISO6392ToLanguage(CStringA(pStreamInfo->Language)) + _T("]");
                }

                *ppszName = (WCHAR*)CoTaskMemAlloc((str.GetLength() + 1) * sizeof(WCHAR));
                if (*ppszName == nullptr) {
                    return E_OUTOFMEMORY;
                }
                wcscpy_s(*ppszName, str.GetLength() + 1, str);
            }

            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CFGManagerBDA::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IBDATuner)
        QI(IAMStreamSelect)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CFGManagerBDA::CreateMicrosoftDemux(CComPtr<IBaseFilter>& pMpeg2Demux)
{
    CComPtr<IMpeg2Demultiplexer> pDemux;
    HRESULT hr = S_OK;
    bool bAudioMPA = false;
    bool bAudioAC3 = false;
    bool bAudioEAC3 = false;
    bool bAudioLATM = false;

    CheckNoLog(pMpeg2Demux->QueryInterface(IID_PPV_ARGS(&pDemux)));

    LOG(_T("Receiver -> Demux connected."));
    CAppSettings& s = AfxGetAppSettings();
    CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
    if (pChannel && (m_nDVBRebuildFilterGraph == DVB_REBUILD_FG_ALWAYS)) {
        for (int i = 0; i < pChannel->GetAudioCount(); i++) {
            switch ((pChannel->GetAudio(i))->Type) {
                case DVB_MPA:
                    bAudioMPA = true;
                    break;
                case DVB_AC3:
                    bAudioAC3 = true;
                    break;
                case DVB_EAC3:
                    bAudioEAC3 = true;
                    break;
                case DVB_LATM:
                    bAudioLATM = true;
                    break;
            }
        }
    } else {  // All the possible audio filters will be present in the filter graph
        bAudioMPA = true;
        bAudioAC3 = true;
        bAudioEAC3 = true;
        bAudioLATM = true;
    }

    POSITION pos = m_DVBStreams.GetStartPosition();
    while (pos) {
        CComPtr<IPin> pPin;
        DVB_STREAM_TYPE nType = m_DVBStreams.GetNextKey(pos);
        CDVBStream& Stream = m_DVBStreams[nType];

        switch (nType) {
            case DVB_TIF:
            case DVB_PSI:
                if (!Stream.GetFindExisting() ||
                        (pPin = FindPin(pMpeg2Demux, PINDIR_OUTPUT, Stream.GetMediaType())) == nullptr) {
                    CheckNoLog(pDemux->CreateOutputPin((AM_MEDIA_TYPE*)Stream.GetMediaType(), Stream.GetName(), &pPin));
                }
                CheckNoLog(Connect(pPin, nullptr, false));
                Stream.SetPin(pPin);
                LOG(_T("Filter connected to Demux for media type %d."), nType);
                break;

            case DVB_MPV:
            case DVB_H264:
                if ((nType == m_nCurVideoType) || (m_nDVBRebuildFilterGraph == DVB_REBUILD_FG_NEVER)) {
                    if (!Stream.GetFindExisting() ||
                            (pPin = FindPin(pMpeg2Demux, PINDIR_OUTPUT, Stream.GetMediaType())) == nullptr) {
                        CheckNoLog(pDemux->CreateOutputPin((AM_MEDIA_TYPE*)Stream.GetMediaType(), Stream.GetName(), &pPin));
                    }
                    if (m_nCurVideoType == nType) {
                        CheckNoLog(Connect(pPin, nullptr, true));
                        Stream.SetPin(pPin);
                        LOG(_T("Graph completed for stream type %d."), nType);
                    } else {
                        CheckNoLog(Connect(pPin, nullptr, false));
                        Stream.SetPin(pPin);
                        LOG(_T("Filter connected to Demux for media type %d."), nType);
                    }
                }
                break;

            case DVB_MPA:
            case DVB_AC3:
            case DVB_EAC3:
            case DVB_LATM:
                if ((bAudioMPA && (nType == DVB_MPA)) || (bAudioAC3 && (nType == DVB_AC3)) ||
                        (bAudioEAC3 && (nType == DVB_EAC3)) || (bAudioLATM && (nType == DVB_LATM))) {
                    if (!Stream.GetFindExisting() ||
                            (pPin = FindPin(pMpeg2Demux, PINDIR_OUTPUT, Stream.GetMediaType())) == nullptr) {
                        CheckNoLog(pDemux->CreateOutputPin((AM_MEDIA_TYPE*)Stream.GetMediaType(), Stream.GetName(), &pPin));
                    }
                    if (m_nCurAudioType == nType) {
                        CheckNoLog(Connect(pPin, nullptr, true));
                        Stream.SetPin(pPin);
                        LOG(_T("Graph completed for stream type %d."), nType);
                    } else {
                        CheckNoLog(Connect(pPin, nullptr, false));
                        Stream.SetPin(pPin);
                        LOG(_T("Filter connected to Demux for media type %d."), nType);
                    }
                }
                break;

            case DVB_SUB:
                if (!Stream.GetFindExisting() ||
                        (pPin = FindPin(pMpeg2Demux, PINDIR_OUTPUT, Stream.GetMediaType())) == nullptr) {
                    CheckNoLog(pDemux->CreateOutputPin((AM_MEDIA_TYPE*)Stream.GetMediaType(), Stream.GetName(), &pPin));
                }
                CheckNoLog(Connect(pPin, nullptr, false));
                CComPtr<IPin> pPinTo;
                pPin->ConnectedTo(&pPinTo);
                if (SUCCEEDED(hr = ((CMainFrame*)AfxGetMainWnd())->InsertTextPassThruFilter(pMpeg2Demux, pPin, pPinTo))) {
                    Stream.SetPin(pPin);
                    LOG(_T("Filter connected to Demux for media type %d."), nType);
                } else {
                    LOG(_T("DVB_SUB Filter connection failed. Error 0x%08x."), hr);
                }
                break;
        }
    }

    LOG(_T("CreateMicrosoftDemux succeeded.\n"));

    return hr;
}

HRESULT CFGManagerBDA::SetChannelInternal(CDVBChannel* pChannel)
{
    HRESULT hr = E_ABORT;
    bool fRadioToTV = false;
    const CAppSettings& s = AfxGetAppSettings();
    ClearMaps();

    if ((s.nDVBStopFilterGraph == DVB_STOP_FG_ALWAYS) && (GetState() != State_Stopped)) {
        ChangeState(State_Stopped);
    }

    if (pChannel->GetVideoPID() != 0) {
        CComPtr<IMpeg2Demultiplexer> pDemux;
        m_pDemux->QueryInterface(IID_PPV_ARGS(&pDemux));

        if (pChannel->GetVideoType() == DVB_H264) {
            UpdateMediaType(&vih2_H264, pChannel);
            hr = pDemux->SetOutputPinMediaType(L"h264", const_cast<AM_MEDIA_TYPE*>(&mt_H264));
        } else {
            UpdateMediaType(&sMpv_fmt, pChannel);
            hr = pDemux->SetOutputPinMediaType(L"mpv", const_cast<AM_MEDIA_TYPE*>(&mt_Mpv));
        }
        if ((m_nCurVideoType != pChannel->GetVideoType()) || (s.nDVBStopFilterGraph == DVB_STOP_FG_ALWAYS)) {
            if ((s.nDVBStopFilterGraph == DVB_STOP_FG_WHEN_SWITCHING) && (GetState() != State_Stopped)) {
                ChangeState(State_Stopped);
            }
            if (FAILED(hr = SwitchStream(m_nCurVideoType, pChannel->GetVideoType()))) {
                LOG(_T("Video switchStream failed. Result: 0x%08x"), hr);
                return hr;
            }
        }

        if (m_fHideWindow) {
            fRadioToTV = true;
        }
    } else {
        m_fHideWindow = true;
        ((CMainFrame*)AfxGetMainWnd())->HideVideoWindow(m_fHideWindow);
    }

    if (m_nCurAudioType != pChannel->GetDefaultAudioType()) {
        if (FAILED(hr = SwitchStream(m_nCurAudioType, pChannel->GetDefaultAudioType()))) {
            LOG(_T("Audio switchStream failed. Result: 0x%08x"), hr);
            return hr;
        }

    }

    if (GetState() == State_Stopped) {
        CheckNoLog(ChangeState(State_Running));
    }

    CheckNoLog(SetFrequency(pChannel->GetFrequency()));

    CheckNoLog(Flush(pChannel->GetVideoType(), pChannel->GetDefaultAudioType()));

    if (pChannel->GetVideoPID() != 0) {
        CheckNoLog(m_DVBStreams[pChannel->GetVideoType()].Map(pChannel->GetVideoPID()));
    }

    CheckNoLog(m_DVBStreams[pChannel->GetDefaultAudioType()].Map(pChannel->GetDefaultAudioPID()));

    if (pChannel->GetSubtitleCount() > 0 && pChannel->GetDefaultSubtitle() != -1 && pChannel->GetDefaultSubtitle() != pChannel->GetSubtitleCount()) {
        CheckNoLog(m_DVBStreams[DVB_SUB].Map(pChannel->GetDefaultSubtitlePID()));
    }
    LOG(_T("Stream maps:"));
    LOG(_T("Mapped PID MPEG-2: %d, Mapped PID H.264: %d."), m_DVBStreams[DVB_MPV].GetMappedPID(), m_DVBStreams[DVB_H264].GetMappedPID());
    LOG(_T("Mapped PID MPA: %d, Mapped PID AC3: %d, Mapped PID EAC3: %d, Mapped PID AAC-LATM: %d."), m_DVBStreams[DVB_MPA].GetMappedPID(),
        m_DVBStreams[DVB_AC3].GetMappedPID(), m_DVBStreams[DVB_EAC3].GetMappedPID(), m_DVBStreams[DVB_LATM].GetMappedPID());
    LOG(_T("Mapped PID Subtitles: %d."), m_DVBStreams[DVB_SUB].GetMappedPID());

    if (fRadioToTV) {
        m_fHideWindow = false;
        Sleep(1800);
        ((CMainFrame*)AfxGetMainWnd())->HideVideoWindow(m_fHideWindow);
    }

    return hr;
}

HRESULT CFGManagerBDA::Flush(DVB_STREAM_TYPE nVideoType, DVB_STREAM_TYPE nAudioType)
{
    HRESULT hr = S_OK;

    CComPtr<IBaseFilter> pFilterSub = m_DVBStreams[DVB_SUB].GetFilter();
    if (pFilterSub) {
        CComPtr<IPin> pSubPinIn = GetFirstPin(pFilterSub, PINDIR_INPUT);
        hr = pSubPinIn->BeginFlush();
        hr = pSubPinIn->EndFlush();
        hr = pSubPinIn->NewSegment(0, MAXLONG, 1);
    }
    CComPtr<IBaseFilter> pFilterAudio = m_DVBStreams[nAudioType].GetFilter();
    if (pFilterAudio) {
        CComPtr<IPin> pAudPinIn = GetFirstPin(pFilterAudio, PINDIR_INPUT);
        hr = pAudPinIn->BeginFlush();
        hr = pAudPinIn->EndFlush();
        hr = pAudPinIn->NewSegment(0, MAXLONG, 1);
    }
    CComPtr<IBaseFilter> pFilterVideo = m_DVBStreams[nVideoType].GetFilter();
    if (pFilterVideo) {
        CComPtr<IPin> pVidPinIn = GetFirstPin(pFilterVideo, PINDIR_INPUT);
        hr = pVidPinIn->BeginFlush();
        hr = pVidPinIn->EndFlush();
        hr = pVidPinIn->NewSegment(0, MAXLONG, 1);
    }

    return hr;
}

HRESULT CFGManagerBDA::SwitchStream(DVB_STREAM_TYPE nOldType, DVB_STREAM_TYPE nNewType)
{
    HRESULT hr = S_OK;
    CComPtr<IBaseFilter> pFGOld = m_DVBStreams[nOldType].GetFilter();
    CComPtr<IBaseFilter> pFGNew = m_DVBStreams[nNewType].GetFilter();
    CComPtr<IPin> pOldOut = GetFirstPin(pFGOld, PINDIR_OUTPUT);
    CComPtr<IPin> pInPin;
    if (pOldOut && pFGNew) {
        pOldOut->ConnectedTo(&pInPin);
        CComPtr<IPin> pNewOut = GetFirstPin(pFGNew, PINDIR_OUTPUT);
        CComPtr<IPinConnection> pNewOutDynamic;

        if ((nNewType != DVB_H264) && (nNewType != DVB_MPV) && GetState() != State_Stopped) {
            CComPtr<IMpeg2Demultiplexer> pDemux;
            m_pDemux->QueryInterface(IID_PPV_ARGS(&pDemux));

            switch (nNewType) {
                case DVB_MPA:
                    hr = pDemux->SetOutputPinMediaType(L"mpa", const_cast<AM_MEDIA_TYPE*>(&mt_Mpa));
                    break;
                case DVB_AC3:
                    hr = pDemux->SetOutputPinMediaType(L"ac3", const_cast<AM_MEDIA_TYPE*>(&mt_Ac3));
                    break;
                case DVB_EAC3:
                    hr = pDemux->SetOutputPinMediaType(L"eac3", const_cast<AM_MEDIA_TYPE*>(&mt_Eac3));
                    break;
                case DVB_LATM:
                    hr = pDemux->SetOutputPinMediaType(L"latm", const_cast<AM_MEDIA_TYPE*>(&mt_latm));
                    break;
            }
        }

        CComPtr<IPinConnection> pOldOutDynamic;
        pOldOut->QueryInterface(__uuidof(IPinConnection), (void**) &pOldOutDynamic);
        CComPtr<IPinConnection> pInPinDynamic;
        pInPin->QueryInterface(__uuidof(IPinConnection), (void**) &pInPinDynamic);
        CComPtr<IPinFlowControl> pOldOutControl;
        if ((GetState() != State_Stopped) && pInPinDynamic && pOldOutDynamic) {       // Try dynamic switch
            pOldOut->QueryInterface(__uuidof(IPinFlowControl), (void**) &pOldOutControl);
            pOldOutControl->Block(AM_PIN_FLOW_CONTROL_BLOCK, nullptr);
            CComPtr<IGraphConfig> pGraph;
            QueryInterface(__uuidof(IGraphConfig), (void**) &pGraph);
            hr = pGraph->Reconnect(pNewOut, pInPin, nullptr, nullptr, nullptr, AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT);
            pOldOutControl->Block(0, nullptr);
        } else {                                     // Dynamic pins not supported
            LOG(_T("Dynamic pin interface not supported."));
            hr = Disconnect(pOldOut);
            if (FAILED(hr) && (GetState() != State_Stopped)) {
                ChangeState(State_Stopped);
                hr = Disconnect(pOldOut);
            }
            if (SUCCEEDED(hr)) {
                hr = Disconnect(pInPin);
                if (FAILED(hr) && (GetState() != State_Stopped)) {
                    ChangeState(State_Stopped);
                    hr = Disconnect(pInPin);
                }
            }
            if (SUCCEEDED(hr)) {
                hr = ConnectDirect(pNewOut, pInPin, nullptr);
                if (FAILED(hr) && (GetState() != State_Stopped)) {
                    ChangeState(State_Stopped);
                    hr = ConnectDirect(pNewOut, pInPin, nullptr);
                }
                if (FAILED(hr)) {
                    hr = E_UNEXPECTED;
                }
            }
        }
    } else {
        hr = E_POINTER;
        ASSERT(FALSE);
    }

    LOG(_T("SwitchStream - Stream type: %d. Result: 0x%08x"), nNewType, hr);

    return hr;
}

void CFGManagerBDA::UpdateMediaType(VIDEOINFOHEADER2* NewVideoHeader, CDVBChannel* pChannel)
{
    NewVideoHeader->AvgTimePerFrame = pChannel->GetAvgTimePerFrame();
    if ((pChannel->GetVideoFps() == DVB_FPS_25_0) ||
            (pChannel->GetVideoFps() == DVB_FPS_29_97) ||
            (pChannel->GetVideoFps() == DVB_FPS_30_0)) {
        NewVideoHeader->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOrWeave;
    } else {
        NewVideoHeader->dwInterlaceFlags = 0;
    }
    if ((pChannel->GetVideoARx() != 0) && (pChannel->GetVideoARy() != 0)) {
        NewVideoHeader->dwPictAspectRatioX = pChannel->GetVideoARx();
        NewVideoHeader->dwPictAspectRatioY = pChannel->GetVideoARy();
    } else {
        NewVideoHeader->dwPictAspectRatioX = 16;
        NewVideoHeader->dwPictAspectRatioY = 9;
    }

    if (pChannel->GetVideoHeight()) {
        NewVideoHeader->bmiHeader.biHeight = pChannel->GetVideoHeight();
        NewVideoHeader->bmiHeader.biWidth = pChannel->GetVideoWidth();
    } else if (pChannel->GetVideoType() == DVB_H264) {
        NewVideoHeader->bmiHeader.biHeight = 1080;   // 1080 was the default before this change (should be 576)
        NewVideoHeader->bmiHeader.biWidth = 1920;    // 1920 was the default before this change (should be 720)
    } else {
        NewVideoHeader->bmiHeader.biHeight = 576;
        NewVideoHeader->bmiHeader.biWidth = 720;
    }

    if (NewVideoHeader->dwPictAspectRatioX && NewVideoHeader->dwPictAspectRatioY) {
        NewVideoHeader->bmiHeader.biWidth = (LONG)(NewVideoHeader->bmiHeader.biHeight * NewVideoHeader->dwPictAspectRatioX / NewVideoHeader->dwPictAspectRatioY);
    }

    NewVideoHeader->rcSource.top = 0;
    NewVideoHeader->rcSource.left = 0;
    NewVideoHeader->rcSource.right = NewVideoHeader->bmiHeader.biWidth;
    NewVideoHeader->rcSource.bottom = NewVideoHeader->bmiHeader.biHeight;
    NewVideoHeader->rcTarget.top = 0;
    NewVideoHeader->rcTarget.left = 0;
    NewVideoHeader->rcTarget.right = 0;
    NewVideoHeader->rcTarget.bottom = 0;
}

HRESULT CFGManagerBDA::UpdatePSI(EventDescriptor& NowNext)
{
    HRESULT hr = S_FALSE;
    CAppSettings& s = AfxGetAppSettings();
    CDVBChannel* pChannel = s.FindChannelByPref(s.nDVBLastChannel);
    CMpeg2DataParser Parser(m_DVBStreams[DVB_PSI].GetFilter());

    if (pChannel->GetNowNextFlag()) {
        hr = Parser.ParseEIT(pChannel->GetSID(), NowNext);
    }

    return hr;
}

HRESULT CFGManagerBDA::ChangeState(FILTER_STATE nRequested)
{
    HRESULT hr = S_OK;
    OAFilterState nState = nRequested + 1;

    CComPtr<IMediaControl> pMC;
    QueryInterface(__uuidof(IMediaControl), (void**) &pMC);
    pMC->GetState(500, &nState);
    if (nState != nRequested) {
        switch (nRequested) {
            case State_Stopped: {
                if (SUCCEEDED(hr = pMC->Stop())) {
                    ((CMainFrame*)AfxGetMainWnd())->KillTimersStop();
                }
                LOG(_T("IMediaControl stop: 0x%08x."), hr);
                return hr;
            }
            case State_Paused: {
                LOG(_T("IMediaControl pause."));
                return pMC->Pause();
            }
            case State_Running: {
                if (SUCCEEDED(hr = pMC->Run()) && SUCCEEDED(hr = pMC->GetState(500, &nState)) && nState == State_Running) {
                    ((CMainFrame*)AfxGetMainWnd())->SetTimersPlay();
                }
                LOG(_T("IMediaControl play: 0x%08x."), hr);
                return hr;
            }
        }
    }
    return hr;
}

FILTER_STATE CFGManagerBDA::GetState()
{
    CComPtr<IMediaControl> pMC;
    OAFilterState nState;
    QueryInterface(__uuidof(IMediaControl), (void**) &pMC);
    pMC->GetState(500, &nState);

    return (FILTER_STATE) nState;
}
