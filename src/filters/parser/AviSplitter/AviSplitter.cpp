/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include <MMReg.h>
#include "AviFile.h"
#include "AviSplitter.h"

#define MAXPACKETS_AVI 1000

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_Avi},
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] = {
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, 0, nullptr}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CAviSplitterFilter), AviSplitterName, MERIT_NORMAL + 1, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CAviSourceFilter), AviSourceName, MERIT_NORMAL + 1, 0, nullptr, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CAviSplitterFilter>, nullptr, &sudFilter[0]},
    {sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CAviSourceFilter>, nullptr, &sudFilter[1]},
    {L"CAviSplitterPropertyPage", &__uuidof(CAviSplitterSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CAviSplitterSettingsWnd>>},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    CAtlList<CString> chkbytes;
    chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564920")); // 'RIFF' ... 'AVI '
    chkbytes.AddTail(_T("0,4,,52494646,8,4,,41564958")); // 'RIFF' ... 'AVIX'
    chkbytes.AddTail(_T("0,4,,52494646,8,4,,414D5620")); // 'RIFF' ... 'AMV '

    RegisterSourceFilter(
        CLSID_AsyncReader,
        MEDIASUBTYPE_Avi,
        chkbytes,
        _T(".avi"), _T(".divx"), _T(".vp6"), _T(".amv"), nullptr);

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    //UnRegisterSourceFilter(MEDIASUBTYPE_Avi);

    return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

class CAviSplitterApp : public CWinApp
{
public:
    CAviSplitterApp() {}

    BOOL InitInstance() {
        if (!__super::InitInstance()) {
            return FALSE;
        }
        DllEntryPoint(m_hInstance, DLL_PROCESS_ATTACH, 0);
        return TRUE;
    }

    BOOL ExitInstance() {
        DllEntryPoint(m_hInstance, DLL_PROCESS_DETACH, 0);
        return __super::ExitInstance();
    }

    void SetDefaultRegistryKey() {
        SetRegistryKey(_T("Gabest"));
    }

    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAviSplitterApp, CWinApp)
END_MESSAGE_MAP()

CAviSplitterApp theApp;

#endif

//
// CAviSplitterFilter
//

CAviSplitterFilter::CAviSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CBaseSplitterFilter(NAME("CAviSplitterFilter"), pUnk, phr, __uuidof(this), MAXPACKETS_AVI)
    , m_timeformat(TIME_FORMAT_MEDIA_TIME)
    , m_maxTimeStamp(Packet::INVALID_TIME)
    , m_bNonInterleavedFilesSupport(true)
{
#ifdef STANDALONE_FILTER
    /*CRegKey key;

    if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\AVI Splitter"), KEY_READ)) {
        DWORD dw;

        if (ERROR_SUCCESS == key.QueryDWORDValue(_T("NonInterleavedFilesSupport"), dw)) {
            m_bNonInterleavedFilesSupport = !!dw;
        }
    }*/
#else
    //m_bNonInterleavedFilesSupport = !!AfxGetApp()->GetProfileInt(_T("Filters\\AVI Splitter"), _T("NonInterleavedFilesSupport"), m_bNonInterleavedFilesSupport);
#endif
}

STDMETHODIMP CAviSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    *ppv = nullptr;

    return
        QI(IAviSplitterFilter)
        QI(ISpecifyPropertyPages)
        QI(ISpecifyPropertyPages2)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CAviSplitterFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

    if (m_pName && m_pName[0] == L'M' && m_pName[1] == L'P' && m_pName[2] == L'C') {
        (void)StringCchCopyW(pInfo->achName, NUMELMS(pInfo->achName), m_pName);
    } else {
        wcscpy_s(pInfo->achName, AviSourceName);
    }
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

HRESULT CAviSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
    CheckPointer(pAsyncReader, E_POINTER);

    HRESULT hr = E_FAIL;

    m_pFile.Free();
    m_tFrame.Free();

    m_pFile.Attach(DEBUG_NEW CAviFile(pAsyncReader, hr));
    if (!m_pFile) {
        return E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr) && !m_bNonInterleavedFilesSupport && !m_pFile->IsInterleaved()) {
        hr = E_FAIL;
    }

    if (FAILED(hr)) {
        m_pFile.Free();
        return hr;
    }

    m_rtNewStart = m_rtCurrent = 0;
    m_rtNewStop = m_rtStop = m_rtDuration = m_pFile->GetTotalTime();

    bool fHasIndex = false;

    for (unsigned int i = 0; !fHasIndex && i < m_pFile->m_strms.GetCount(); ++i) {
        if (!m_pFile->m_strms[i]->cs.IsEmpty()) {
            fHasIndex = true;
        }
    }

    for (unsigned int i = 0; i < m_pFile->m_strms.GetCount(); ++i) {
        CAviFile::strm_t* s = m_pFile->m_strms[i];

        if (fHasIndex && s->cs.IsEmpty()) {
            continue;
        }

        CMediaType mt;
        CAtlArray<CMediaType> mts;

        CStringW name, label;

        if (s->strh.fccType == FCC('vids')) {
            label = L"Video";

            ASSERT(s->strf.GetCount() >= sizeof(BITMAPINFOHEADER));

            BITMAPINFOHEADER* pbmi = &((BITMAPINFO*)s->strf.GetData())->bmiHeader;

            mt.majortype = MEDIATYPE_Video;
            switch (pbmi->biCompression) {
                case BI_RGB:
                case BI_BITFIELDS:
                    mt.subtype =
                        pbmi->biBitCount == 1 ? MEDIASUBTYPE_RGB1 :
                        pbmi->biBitCount == 4 ? MEDIASUBTYPE_RGB4 :
                        pbmi->biBitCount == 8 ? MEDIASUBTYPE_RGB8 :
                        pbmi->biBitCount == 16 ? MEDIASUBTYPE_RGB565 :
                        pbmi->biBitCount == 24 ? MEDIASUBTYPE_RGB24 :
                        pbmi->biBitCount == 32 ? MEDIASUBTYPE_ARGB32 :
                        MEDIASUBTYPE_NULL;
                    break;
                    //case BI_RLE8: mt.subtype = MEDIASUBTYPE_RGB8; break;
                    //case BI_RLE4: mt.subtype = MEDIASUBTYPE_RGB4; break;
                case FCC('AVRn')://uncommon fourcc
                case FCC('JPGL')://uncommon fourcc
                    mt.subtype = MEDIASUBTYPE_MJPG;
                    break;
                case FCC('MPG2'):
                    mt.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
                    break;
                default:
                    mt.subtype = FOURCCMap(pbmi->biCompression);
            }
            mt.formattype = FORMAT_VideoInfo;
            VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + (ULONG)s->strf.GetCount() - sizeof(BITMAPINFOHEADER));
            memset(mt.Format(), 0, mt.FormatLength());
            memcpy(&pvih->bmiHeader, s->strf.GetData(), s->strf.GetCount());
            if (s->strh.dwRate > 0) {
                pvih->AvgTimePerFrame = 10000000ui64 * s->strh.dwScale / s->strh.dwRate;
            }

            if (s->cs.GetCount() && pvih->AvgTimePerFrame > 0) {
                UINT64 size = 0;
                for (unsigned int j = 0; j < s->cs.GetCount(); ++j) {
                    size += s->cs[j].orgsize;
                }
                pvih->dwBitRate = (DWORD)(10000000.0 * size * 8 / (s->cs.GetCount() * pvih->AvgTimePerFrame) + 0.5);
                // need calculate in double, because the (10000000ui64 * size * 8) can give overflow
            }

            mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
                             ? s->strh.dwSuggestedBufferSize * 3 / 2
                             : (pvih->bmiHeader.biWidth * pvih->bmiHeader.biHeight * 4));
            mts.Add(mt);
        } else if (s->strh.fccType == FCC('auds') || s->strh.fccType == FCC('amva')) {
            label = L"Audio";

            ASSERT(s->strf.GetCount() >= sizeof(WAVEFORMATEX)
                   || s->strf.GetCount() == sizeof(PCMWAVEFORMAT));

            WAVEFORMATEX* pwfe = (WAVEFORMATEX*)s->strf.GetData();

            if (pwfe->nBlockAlign == 0) {
                continue;
            }

            mt.majortype = MEDIATYPE_Audio;
            if (m_pFile->m_isamv) {
                mt.subtype = FOURCCMap(MAKEFOURCC('A', 'M', 'V', 'A'));
            } else {
                mt.subtype = FOURCCMap(pwfe->wFormatTag);
            }
            mt.formattype = FORMAT_WaveFormatEx;
            if (nullptr == mt.AllocFormatBuffer(max((ULONG)s->strf.GetCount(), sizeof(WAVEFORMATEX)))) {
                continue;
            }
            memcpy(mt.Format(), s->strf.GetData(), s->strf.GetCount());
            pwfe = (WAVEFORMATEX*)mt.Format();
            if (s->strf.GetCount() == sizeof(PCMWAVEFORMAT)) {
                pwfe->cbSize = 0;
            }
            if (pwfe->wFormatTag == WAVE_FORMAT_PCM) {
                pwfe->nBlockAlign = pwfe->nChannels * pwfe->wBitsPerSample >> 3;
            }
            if (pwfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
                mt.subtype = ((WAVEFORMATEXTENSIBLE*)pwfe)->SubFormat;
            }
            mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
                             ? s->strh.dwSuggestedBufferSize * 3 / 2
                             : (pwfe->nChannels * pwfe->nSamplesPerSec * 32 >> 3));
            mts.Add(mt);
        } else if (s->strh.fccType == FCC('mids')) {
            label = L"Midi";

            mt.majortype = MEDIATYPE_Midi;
            mt.subtype = MEDIASUBTYPE_NULL;
            mt.formattype = FORMAT_None;
            mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
                             ? s->strh.dwSuggestedBufferSize * 3 / 2
                             : (1024 * 1024));
            mts.Add(mt);
        } else if (s->strh.fccType == FCC('txts')) {
            label = L"Text";

            mt.majortype = MEDIATYPE_Text;
            mt.subtype = MEDIASUBTYPE_NULL;
            mt.formattype = FORMAT_None;
            mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
                             ? s->strh.dwSuggestedBufferSize * 3 / 2
                             : (1024 * 1024));
            mts.Add(mt);
        } else if (s->strh.fccType == FCC('iavs')) {
            label = L"Interleaved";

            ASSERT(s->strh.fccHandler == FCC('dvsd'));

            mt.majortype = MEDIATYPE_Interleaved;
            mt.subtype = FOURCCMap(s->strh.fccHandler);
            mt.formattype = FORMAT_DvInfo;
            mt.SetFormat(s->strf.GetData(), max((ULONG)s->strf.GetCount(), sizeof(DVINFO)));
            mt.SetSampleSize(s->strh.dwSuggestedBufferSize > 0
                             ? s->strh.dwSuggestedBufferSize * 3 / 2
                             : (1024 * 1024));
            mts.Add(mt);
        }

        if (mts.IsEmpty()) {
            TRACE(_T("CAviSourceFilter: Unsupported stream (%d)\n"), i);
            continue;
        }

        //Put filename at front sometime(eg. ~temp.avi) will cause filter graph
        //stop check this pin. Not sure the reason exactly. but it happens.
        //If you know why, please emailto: tomasen@gmail.com
        if (s->strn.IsEmpty()) {
            name.Format(L"%s %u", label, i);
        } else {
            name.Format(L"%s (%s %u)", CStringW(s->strn), label, i);
        }

        HRESULT hr2;

        CAutoPtr<CBaseSplitterOutputPin> pPinOut(DEBUG_NEW CAviSplitterOutputPin(mts, name, this, this, &hr2));
        AddOutputPin(i, pPinOut);
    }

    POSITION pos = m_pFile->m_info.GetStartPosition();
    while (pos) {
        DWORD fcc;
        CStringA value;
        m_pFile->m_info.GetNextAssoc(pos, fcc, value);

        switch (fcc) {
            case FCC('INAM'):
                SetProperty(L"TITL", CStringW(value));
                break;
            case FCC('IART'):
                SetProperty(L"AUTH", CStringW(value));
                break;
            case FCC('ICOP'):
                SetProperty(L"CPYR", CStringW(value));
                break;
            case FCC('ISBJ'):
                SetProperty(L"DESC", CStringW(value));
                break;
        }
    }

    m_tFrame.Attach(DEBUG_NEW DWORD[m_pFile->m_avih.dwStreams]);

    return !m_pOutputs.IsEmpty() ? S_OK : E_FAIL;
}

bool CAviSplitterFilter::DemuxInit()
{
    SetThreadName((DWORD) - 1, "CAviSplitterFilter");

    if (!m_pFile) {
        return false;
    }

    // reindex if needed

    bool fReIndex = false;

    for (DWORD i = 0; i < m_pFile->m_avih.dwStreams && !fReIndex; ++i) {
        if (m_pFile->m_strms[i]->cs.IsEmpty() && GetOutputPin(i)) {
            fReIndex = true;
        }
    }

    if (fReIndex) {
        m_pFile->EmptyIndex();

        m_fAbort = false;
        m_nOpenProgress = 0;

        m_rtDuration = 0;

        CAutoVectorPtr<UINT64> pSize;
        pSize.Allocate(m_pFile->m_avih.dwStreams);
        memset((UINT64*)pSize, 0, sizeof(UINT64)*m_pFile->m_avih.dwStreams);
        m_pFile->Seek(0);
        ReIndex(m_pFile->GetLength(), pSize);

        if (m_fAbort) {
            m_pFile->EmptyIndex();
        }

        m_fAbort = false;
        m_nOpenProgress = 100;
    }

    return true;
}

HRESULT CAviSplitterFilter::ReIndex(__int64 end, UINT64* pSize)
{
    HRESULT hr = S_OK;

    while (S_OK == hr && m_pFile->GetPos() < end && SUCCEEDED(hr) && !m_fAbort) {
        __int64 pos = m_pFile->GetPos();

        DWORD id = 0, size;
        if (S_OK != m_pFile->Read(id) || id == 0) {
            return E_FAIL;
        }

        if (id == FCC('RIFF') || id == FCC('LIST')) {
            if (S_OK != m_pFile->Read(size) || S_OK != m_pFile->Read(id)) {
                return E_FAIL;
            }

            size += (size & 1) + 8;

            if (id == FCC('AVI ') || id == FCC('AVIX') || id == FCC('movi') || id == FCC('rec ')) {
                hr = ReIndex(pos + size, pSize);
            }
        } else {
            if (S_OK != m_pFile->Read(size)) {
                return E_FAIL;
            }

            DWORD TrackNumber = TRACKNUM(id);

            if (TrackNumber < m_pFile->m_strms.GetCount()) {
                CAviFile::strm_t* s = m_pFile->m_strms[TrackNumber];

                WORD type = TRACKTYPE(id);

                if (type == 'db' || type == 'dc' || /*type == 'pc' ||*/ type == 'wb'
                        || type == 'iv' || type == '__' || type == 'xx') {
                    CAviFile::strm_t::chunk c;
                    c.filepos = pos;
                    c.size = pSize[TrackNumber];
                    c.orgsize = size;
                    c.fKeyFrame = size > 0; // TODO: find a better way...
                    c.fChunkHdr = true;
                    s->cs.Add(c);

                    pSize[TrackNumber] += s->GetChunkSize(size);

                    REFERENCE_TIME rt = s->GetRefTime((DWORD)s->cs.GetCount() - 1, pSize[TrackNumber]);
                    m_rtDuration = max(rt, m_rtDuration);
                }
            }

            size += (size & 1) + 8;
        }

        m_pFile->Seek(pos + size);

        m_nOpenProgress = m_pFile->GetPos() * 100 / m_pFile->GetLength();

        DWORD cmd;
        if (CheckRequest(&cmd)) {
            if (cmd == CMD_EXIT) {
                m_fAbort = true;
            } else {
                Reply(S_OK);
            }
        }
    }

    return hr;
}

void CAviSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
    size_t nTracks =  m_pFile->m_strms.GetCount();

    memset((DWORD*)m_tFrame, 0, nTracks * sizeof(DWORD));
    m_pFile->Seek(0);

    DbgLog((LOG_TRACE, 0, _T("Seek: %I64d"), rt / 10000));

    if (rt > 0) {
        for (size_t j = 0; j < nTracks; ++j) {
            CAviFile::strm_t* s = m_pFile->m_strms[j];

            if (s->IsRawSubtitleStream() || s->cs.IsEmpty()) {
                continue;
            }

            //ASSERT(s->GetFrame(rt) == s->GetKeyFrame(rt)); // fast seek test
            m_tFrame[j] = s->GetKeyFrame(rt);
        }
    }
}

bool CAviSplitterFilter::DemuxLoop()
{
    HRESULT hr = S_OK;

    size_t nTracks = m_pFile->m_strms.GetCount();

    CAtlArray<BOOL> fDiscontinuity;
    fDiscontinuity.SetCount(nTracks);
    memset(fDiscontinuity.GetData(), 0, nTracks * sizeof(BOOL));

    while (SUCCEEDED(hr) && !CheckRequest(nullptr)) {
        DWORD curTrack = DWORD_MAX;

        REFERENCE_TIME minTime = INT64_MAX;
        for (size_t j = 0; j < nTracks; ++j) {
            CAviFile::strm_t* s = m_pFile->m_strms[j];
            DWORD f = m_tFrame[j];

            if (f >= (DWORD)s->cs.GetCount()) {
                continue;
            }

            if (s->IsRawSubtitleStream()) {
                // TODO: get subtitle time from index
                minTime = 0;
                curTrack = (DWORD)j;
                break; // read all subtitles at once
            }

            REFERENCE_TIME start = s->GetRefTime(f, s->cs[f].size);
            if (start < minTime) {
                minTime = start;
                curTrack = (DWORD)j;
            }
        }

        if (minTime == INT64_MAX) {
            return true;
        }

        do {
            CAviFile::strm_t* s = m_pFile->m_strms[curTrack];
            DWORD f = m_tFrame[curTrack];

            m_pFile->Seek(s->cs[f].filepos);
            DWORD size = 0;

            if (s->cs[f].fChunkHdr) {
                DWORD id = 0;
                if (S_OK != m_pFile->Read(id) || id == 0 || curTrack != TRACKNUM(id) || S_OK != m_pFile->Read(size)) {
                    fDiscontinuity[curTrack] = true;
                    break;
                }

                if (size != s->cs[f].orgsize) {
                    TRACE(_T("WARNING: CAviFile::DemuxLoop() incorrect chunk size. By index: %d, by header: %d\n"), s->cs[f].orgsize, size);
                    fDiscontinuity[curTrack] = true;
                    //ASSERT(0);
                    break;
                }
            } else {
                size = s->cs[f].orgsize;
            }

            CAutoPtr<Packet> p(DEBUG_NEW Packet());

            p->TrackNumber    = (DWORD)curTrack;
            p->bSyncPoint     = (BOOL)s->cs[f].fKeyFrame;
            p->bDiscontinuity = fDiscontinuity[curTrack];
            p->rtStart        = s->GetRefTime(f, s->cs[f].size);
            p->rtStop         = s->GetRefTime(f + 1, f + 1 < (DWORD)s->cs.GetCount() ? s->cs[f + 1].size : s->totalsize);
            p->SetCount(size);
            if (S_OK != (hr = m_pFile->ByteRead(p->GetData(), p->GetCount()))) {
                return true;    // break;
            }
#if defined(_DEBUG) && 0
            DbgLog((LOG_TRACE, 0,
                    _T("%d (%d): %I64d - %I64d, %I64d - %I64d (size = %d)"),
                    minTrack, (int)p->bSyncPoint,
                    (p->rtStart) / 10000, (p->rtStop) / 10000,
                    (p->rtStart - m_rtStart) / 10000, (p->rtStop - m_rtStart) / 10000,
                    size));
#endif
            m_maxTimeStamp = max(m_maxTimeStamp, p->rtStart);

            hr = DeliverPacket(p);

            fDiscontinuity[curTrack] = false;
        } while (0);

        m_tFrame[curTrack]++;
    }

    if (m_maxTimeStamp != Packet::INVALID_TIME) {
        m_rtCurrent = m_maxTimeStamp;
    }
    return true;
}

// IMediaSeeking

STDMETHODIMP CAviSplitterFilter::GetDuration(LONGLONG* pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    CheckPointer(m_pFile, VFW_E_NOT_CONNECTED);

    if (m_timeformat == TIME_FORMAT_FRAME) {
        for (unsigned int i = 0; i < m_pFile->m_strms.GetCount(); ++i) {
            CAviFile::strm_t* s = m_pFile->m_strms[i];
            if (s->strh.fccType == FCC('vids')) {
                *pDuration = s->cs.GetCount();
                return S_OK;
            }
        }

        return E_UNEXPECTED;
    }

    return __super::GetDuration(pDuration);
}

//

STDMETHODIMP CAviSplitterFilter::IsFormatSupported(const GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    HRESULT hr = __super::IsFormatSupported(pFormat);
    if (S_OK == hr) {
        return hr;
    }
    return *pFormat == TIME_FORMAT_FRAME ? S_OK : S_FALSE;
}

STDMETHODIMP CAviSplitterFilter::GetTimeFormat(GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = m_timeformat;
    return S_OK;
}

STDMETHODIMP CAviSplitterFilter::IsUsingTimeFormat(const GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    return *pFormat == m_timeformat ? S_OK : S_FALSE;
}

STDMETHODIMP CAviSplitterFilter::SetTimeFormat(const GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    if (S_OK != IsFormatSupported(pFormat)) {
        return E_FAIL;
    }
    m_timeformat = *pFormat;
    return S_OK;
}

STDMETHODIMP CAviSplitterFilter::GetStopPosition(LONGLONG* pStop)
{
    CheckPointer(pStop, E_POINTER);
    if (FAILED(__super::GetStopPosition(pStop))) {
        return E_FAIL;
    }
    if (m_timeformat == TIME_FORMAT_MEDIA_TIME) {
        return S_OK;
    }
    LONGLONG rt = *pStop;
    if (FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_FRAME, rt, &TIME_FORMAT_MEDIA_TIME))) {
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP CAviSplitterFilter::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
    CheckPointer(pTarget, E_POINTER);

    const GUID& SourceFormat = pSourceFormat ? *pSourceFormat : m_timeformat;
    const GUID& TargetFormat = pTargetFormat ? *pTargetFormat : m_timeformat;

    if (TargetFormat == SourceFormat) {
        *pTarget = Source;
        return S_OK;
    } else if (TargetFormat == TIME_FORMAT_FRAME && SourceFormat == TIME_FORMAT_MEDIA_TIME) {
        for (unsigned int i = 0; i < m_pFile->m_strms.GetCount(); ++i) {
            CAviFile::strm_t* s = m_pFile->m_strms[i];
            if (s->strh.fccType == FCC('vids')) {
                *pTarget = s->GetFrame(Source);
                return S_OK;
            }
        }
    } else if (TargetFormat == TIME_FORMAT_MEDIA_TIME && SourceFormat == TIME_FORMAT_FRAME) {
        for (unsigned int i = 0; i < m_pFile->m_strms.GetCount(); ++i) {
            CAviFile::strm_t* s = m_pFile->m_strms[i];
            if (s->strh.fccType == FCC('vids')) {
                if (Source < 0 || Source >= (LONGLONG)s->cs.GetCount()) {
                    return E_FAIL;
                }
                CAviFile::strm_t::chunk& c = s->cs[(size_t)Source];
                *pTarget = s->GetRefTime((DWORD)Source, c.size);
                return S_OK;
            }
        }
    }

    return E_FAIL;
}

STDMETHODIMP CAviSplitterFilter::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
    HRESULT hr;
    if (FAILED(hr = __super::GetPositions(pCurrent, pStop)) || m_timeformat != TIME_FORMAT_FRAME) {
        return hr;
    }

    if (pCurrent)
        if (FAILED(ConvertTimeFormat(pCurrent, &TIME_FORMAT_FRAME, *pCurrent, &TIME_FORMAT_MEDIA_TIME))) {
            return E_FAIL;
        }
    if (pStop)
        if (FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_FRAME, *pStop, &TIME_FORMAT_MEDIA_TIME))) {
            return E_FAIL;
        }

    return S_OK;
}

HRESULT CAviSplitterFilter::SetPositionsInternal(void* id, LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    if (m_timeformat != TIME_FORMAT_FRAME) {
        return __super::SetPositionsInternal(id, pCurrent, dwCurrentFlags, pStop, dwStopFlags);
    }

    if (!pCurrent && !pStop
            || (dwCurrentFlags & AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning
            && (dwStopFlags & AM_SEEKING_PositioningBitsMask) == AM_SEEKING_NoPositioning) {
        return S_OK;
    }

    REFERENCE_TIME
    rtCurrent = m_rtCurrent,
    rtStop = m_rtStop;

    if ((dwCurrentFlags & AM_SEEKING_PositioningBitsMask)
            && FAILED(ConvertTimeFormat(&rtCurrent, &TIME_FORMAT_FRAME, rtCurrent, &TIME_FORMAT_MEDIA_TIME))) {
        return E_FAIL;
    }
    if ((dwStopFlags & AM_SEEKING_PositioningBitsMask)
            && FAILED(ConvertTimeFormat(&rtStop, &TIME_FORMAT_FRAME, rtStop, &TIME_FORMAT_MEDIA_TIME))) {
        return E_FAIL;
    }

    if (pCurrent)
        switch (dwCurrentFlags & AM_SEEKING_PositioningBitsMask) {
            case AM_SEEKING_NoPositioning:
                break;
            case AM_SEEKING_AbsolutePositioning:
                rtCurrent = *pCurrent;
                break;
            case AM_SEEKING_RelativePositioning:
                rtCurrent = rtCurrent + *pCurrent;
                break;
            case AM_SEEKING_IncrementalPositioning:
                rtCurrent = rtCurrent + *pCurrent;
                break;
        }

    if (pStop)
        switch (dwStopFlags & AM_SEEKING_PositioningBitsMask) {
            case AM_SEEKING_NoPositioning:
                break;
            case AM_SEEKING_AbsolutePositioning:
                rtStop = *pStop;
                break;
            case AM_SEEKING_RelativePositioning:
                rtStop += *pStop;
                break;
            case AM_SEEKING_IncrementalPositioning:
                rtStop = rtCurrent + *pStop;
                break;
        }

    if ((dwCurrentFlags & AM_SEEKING_PositioningBitsMask)
            && pCurrent)
        if (FAILED(ConvertTimeFormat(pCurrent, &TIME_FORMAT_MEDIA_TIME, rtCurrent, &TIME_FORMAT_FRAME))) {
            return E_FAIL;
        }
    if ((dwStopFlags & AM_SEEKING_PositioningBitsMask)
            && pStop)
        if (FAILED(ConvertTimeFormat(pStop, &TIME_FORMAT_MEDIA_TIME, rtStop, &TIME_FORMAT_FRAME))) {
            return E_FAIL;
        }

    return __super::SetPositionsInternal(id, pCurrent, dwCurrentFlags, pStop, dwStopFlags);
}

// IKeyFrameInfo

STDMETHODIMP CAviSplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
    if (!m_pFile) {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

    nKFs = 0;

    for (unsigned int i = 0; i < m_pFile->m_strms.GetCount(); ++i) {
        CAviFile::strm_t* s = m_pFile->m_strms[i];
        if (s->strh.fccType != FCC('vids')) {
            continue;
        }

        for (unsigned int j = 0; j < s->cs.GetCount(); ++j) {
            CAviFile::strm_t::chunk& c = s->cs[j];
            if (c.fKeyFrame) {
                ++nKFs;
            }
        }

        if (nKFs == s->cs.GetCount()) {
            hr = S_FALSE;
        }

        break;
    }

    return hr;
}

STDMETHODIMP CAviSplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
    CheckPointer(pFormat, E_POINTER);
    CheckPointer(pKFs, E_POINTER);

    if (!m_pFile) {
        return E_UNEXPECTED;
    }
    if (*pFormat != TIME_FORMAT_MEDIA_TIME && *pFormat != TIME_FORMAT_FRAME) {
        return E_INVALIDARG;
    }

    for (unsigned int i = 0; i < m_pFile->m_strms.GetCount(); ++i) {
        CAviFile::strm_t* s = m_pFile->m_strms[i];
        if (s->strh.fccType != FCC('vids')) {
            continue;
        }
        bool fConvertToRefTime = !!(*pFormat == TIME_FORMAT_MEDIA_TIME);

        UINT nKFsTmp = 0;

        for (unsigned int j = 0; j < s->cs.GetCount() && nKFsTmp < nKFs; ++j) {
            if (s->cs[j].fKeyFrame) {
                pKFs[nKFsTmp++] = fConvertToRefTime ? s->GetRefTime(j, s->cs[j].size) : j;
            }
        }
        nKFs = nKFsTmp;

        return S_OK;
    }

    return E_FAIL;
}

// ISpecifyPropertyPages2

STDMETHODIMP CAviSplitterFilter::GetPages(CAUUID* pPages)
{
    CheckPointer(pPages, E_POINTER);

    HRESULT hr = S_OK;

    pPages->cElems = 1;
    pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
    if (pPages->pElems != nullptr) {
        pPages->pElems[0] = __uuidof(CAviSplitterSettingsWnd);
    } else {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

STDMETHODIMP CAviSplitterFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
    CheckPointer(ppPage, E_POINTER);

    if (*ppPage != nullptr) {
        return E_INVALIDARG;
    }

    HRESULT hr;

    if (guid == __uuidof(CAviSplitterSettingsWnd)) {
        (*ppPage = DEBUG_NEW CInternalPropertyPageTempl<CAviSplitterSettingsWnd>(nullptr, &hr))->AddRef();
    }

    return *ppPage ? S_OK : E_FAIL;
}

// IAviSplitterFilter
STDMETHODIMP CAviSplitterFilter::Apply()
{
#ifdef STANDALONE_FILTER
    /*CRegKey key;
    if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\AVI Splitter"))) {
        key.SetDWORDValue(_T("NonInterleavedFilesSupport"), m_bNonInterleavedFilesSupport);
    }*/
#else
    //AfxGetApp()->WriteProfileInt(_T("Filters\\AVI Splitter"), _T("NonInterleavedFilesSupport"), m_bNonInterleavedFilesSupport);
#endif

    return S_OK;
}

STDMETHODIMP CAviSplitterFilter::SetNonInterleavedFilesSupport(BOOL nValue)
{
    CAutoLock cAutoLock(&m_csProps);
    m_bNonInterleavedFilesSupport = !!nValue;
    return S_OK;
}

STDMETHODIMP_(BOOL) CAviSplitterFilter::GetNonInterleavedFilesSupport()
{
    CAutoLock cAutoLock(&m_csProps);
    return m_bNonInterleavedFilesSupport;
}

//
// CAviSourceFilter
//

CAviSourceFilter::CAviSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CAviSplitterFilter(pUnk, phr)
{
    m_clsid = __uuidof(this);
    m_pInput.Free();
}

//
// CAviSplitterOutputPin
//

CAviSplitterOutputPin::CAviSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
    : CBaseSplitterOutputPin(mts, pName, pFilter, pLock, phr, 0, MAXPACKETS_AVI)
{
}

HRESULT CAviSplitterOutputPin::CheckConnect(IPin* pPin)
{
    int iPosition = 0;
    CMediaType mt;
    while (S_OK == GetMediaType(iPosition++, &mt)) {
        if (mt.majortype == MEDIATYPE_Video
                && (mt.subtype == FOURCCMap(FCC('IV32'))
                    || mt.subtype == FOURCCMap(FCC('IV31'))
                    || mt.subtype == FOURCCMap(FCC('IF09')))) {
            CLSID clsid = GetCLSID(GetFilterFromPin(pPin));
            if (clsid == CLSID_VideoMixingRenderer || clsid == CLSID_OverlayMixer) {
                return E_FAIL;
            }
        }

        mt.InitMediaType();
    }

    return __super::CheckConnect(pPin);
}
