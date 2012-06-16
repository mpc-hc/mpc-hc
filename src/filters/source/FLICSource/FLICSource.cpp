/*
 * $Id$
 *
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

#include "stdafx.h"
#include "FLICSource.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32},
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CFLICSource), FlicSourceName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CFLICSource>, NULL, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{17DB5CF6-39BB-4d5b-B0AA-BEBA44673AD4}"),
        _T("0"), _T("4,2,,11AF"));

    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{17DB5CF6-39BB-4d5b-B0AA-BEBA44673AD4}"),
        _T("1"), _T("4,2,,12AF"));

    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{17DB5CF6-39BB-4d5b-B0AA-BEBA44673AD4}"),
        _T("Source Filter"), _T("{17DB5CF6-39BB-4d5b-B0AA-BEBA44673AD4}"));

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".fli"),
        _T("Source Filter"), _T("{17DB5CF6-39BB-4d5b-B0AA-BEBA44673AD4}"));

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".flc"),
        _T("Source Filter"), _T("{17DB5CF6-39BB-4d5b-B0AA-BEBA44673AD4}"));

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{17DB5CF6-39BB-4d5b-B0AA-BEBA44673AD4}"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".fli"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".flc"));

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CFLICSource
//

CFLICSource::CFLICSource(LPUNKNOWN lpunk, HRESULT* phr)
    : CSource(NAME("CFLICSource"), lpunk, __uuidof(this))
{
}

CFLICSource::~CFLICSource()
{
}

STDMETHODIMP CFLICSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFileSourceFilter)
        QI(IAMFilterMiscFlags)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IFileSourceFilter

STDMETHODIMP CFLICSource::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    if (GetPinCount() > 0) {
        return VFW_E_ALREADY_CONNECTED;
    }

    HRESULT hr = S_OK;
    if (!(DNew CFLICStream(pszFileName, this, &hr))) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        return hr;
    }

    m_fn = pszFileName;

    return S_OK;
}

STDMETHODIMP CFLICSource::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    if (!ppszFileName) {
        return E_POINTER;
    }

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}

// IAMFilterMiscFlags

ULONG CFLICSource::GetMiscFlags()
{
    return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}

STDMETHODIMP CFLICSource::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));
    wcscpy_s(pInfo->achName, FlicSourceName);
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

// CFLICStream

CFLICStream::CFLICStream(const WCHAR* wfn, CFLICSource* pParent, HRESULT* phr)
    : CSourceStream(NAME("FLICStream"), phr, pParent, L"Output")
    , CSourceSeeking(NAME("FLICStream"), (IPin*)this, phr, &m_cSharedState)
    , m_bDiscontinuity(FALSE), m_bFlushing(FALSE)
{
    CAutoLock cAutoLock(&m_cSharedState);

    CString fn(wfn);

    if (!m_flic.Open(fn, CFile::modeRead | CFile::shareDenyNone)) {
        if (phr) { *phr = E_FAIL; }
        return;
    }

    if (m_flic.Read(&m_hdr, sizeof(m_hdr)) != sizeof(m_hdr)
            || (m_hdr.type != 0xAF11 && m_hdr.type != 0xAF12)
            || m_hdr.depth != 8) {
        if (phr) { *phr = E_FAIL; }
        return;
    }

    if (m_hdr.speed == 0) {
        m_hdr.speed = (m_hdr.type == 0xAF11) ? 5 : 67;
    }
    m_AvgTimePerFrame = (m_hdr.type == 0xAF11)
                        ? 10000000i64 * m_hdr.speed / 70
                        : 10000000i64 * m_hdr.speed / 1000;

    // not tested (lack of test files)
    {
        __int64 pos = m_flic.GetPosition();
        FLIC_PREFIX fp;
        if (m_flic.Read(&fp, sizeof(fp)) != sizeof(fp) || fp.type != 0xF100) {
            m_flic.Seek(pos, CFile::begin);
        } else {
            m_flic.Seek(pos + fp.size, CFile::begin);
        }
    }

    FLIC_FRAME_ENTRY ffe;
    unsigned int nFrames = 0;
    m_frames.SetCount(m_hdr.frames);
    while (nFrames < m_hdr.frames && m_flic.Read(&ffe.hdr, sizeof(ffe.hdr)) == sizeof(ffe.hdr) && ffe.hdr.type == 0xF1FA) {

        ffe.pos = m_flic.GetPosition();
        ffe.fKeyframe = false;

        int chunk = 0;
        while (chunk < ffe.hdr.chunks) {
            FLIC_CHUNK fc;
            if (m_flic.Read(&fc, sizeof(fc)) != sizeof(fc)) {
                break;
            }
            ffe.fKeyframe = ffe.fKeyframe
                            || fc.type == FLIC_BRUN
                            || fc.type == FLIC_BLACK
                            || fc.type == FLIC_COPY
                            || fc.type == DTA_BRUN
                            || fc.type == DTA_COPY
                            || fc.type == KEY_IMAGE;

            ULONGLONG pos = m_flic.GetPosition() + fc.size - sizeof(fc);
            if (pos < m_flic.GetLength()) {
                m_flic.Seek(pos, CFile::begin);
            } else { break; }

            chunk++;
        }
        if (chunk < ffe.hdr.chunks) {
            break;
        }

        ULONGLONG pos = ffe.pos + ffe.hdr.size - sizeof(ffe.hdr);
        if (pos < m_flic.GetLength()) {
            m_flic.Seek(pos, CFile::begin);
        } else { break; }

        m_frames[nFrames] = ffe;
        nFrames++;
    }
    if (nFrames > 0) {
        m_frames.SetCount(nFrames); // if the file is incomplete, then truncate the index
        m_frames[0].fKeyframe = true;
    } else {
        if (phr) { *phr = E_FAIL; }
        return;
    }

    m_nLastFrameNum = -1;
    memset(m_pPalette, 0, sizeof(m_pPalette));
    m_nBufferSize = m_hdr.width * m_hdr.height * 32 >> 3;
    if (!m_pFrameBuffer.Allocate(m_nBufferSize)) {
        if (phr) { *phr = E_OUTOFMEMORY; }
        return;
    }

    m_rtDuration = m_rtStop = m_AvgTimePerFrame * nFrames;

    if (phr) {
        *phr = m_rtDuration > 0 ? S_OK : E_FAIL;
    }
}

CFLICStream::~CFLICStream()
{
    CAutoLock cAutoLock(&m_cSharedState);
}

STDMETHODIMP CFLICStream::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return (riid == IID_IMediaSeeking) ? CSourceSeeking::NonDelegatingQueryInterface(riid, ppv) //GetInterface((IMediaSeeking*)this, ppv)
           : CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

void CFLICStream::UpdateFromSeek()
{
    if (ThreadExists()) {
        // next time around the loop, the worker thread will
        // pick up the position change.
        // We need to flush all the existing data - we must do that here
        // as our thread will probably be blocked in GetBuffer otherwise

        m_bFlushing = TRUE;

        DeliverBeginFlush();
        // make sure we have stopped pushing
        Stop();
        // complete the flush
        DeliverEndFlush();

        m_bFlushing = FALSE;

        // restart
        Run();
    }
}

HRESULT CFLICStream::SetRate(double dRate)
{
    if (dRate <= 0) {
        return E_INVALIDARG;
    }

    {
        CAutoLock lock(CSourceSeeking::m_pLock);
        m_dRateSeeking = dRate;
    }

    UpdateFromSeek();

    return S_OK;
}

HRESULT CFLICStream::OnThreadStartPlay()
{
    m_bDiscontinuity = TRUE;
    return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}

HRESULT CFLICStream::ChangeStart()
{
    {
        CAutoLock lock(CSourceSeeking::m_pLock);
        m_rtSampleTime = 0;
        m_rtPosition = m_rtStart;
    }

    UpdateFromSeek();

    return S_OK;
}

HRESULT CFLICStream::ChangeStop()
{
    {
        CAutoLock lock(CSourceSeeking::m_pLock);
        if (m_rtPosition < m_rtStop) {
            return S_OK;
        }
    }

    // We're already past the new stop time -- better flush the graph.
    UpdateFromSeek();

    return S_OK;
}

HRESULT CFLICStream::OnThreadCreate()
{
    CAutoLock cAutoLockShared(&m_cSharedState);
    m_rtSampleTime = 0;
    m_rtPosition = m_rtStart;

    return CSourceStream::OnThreadCreate();
}

HRESULT CFLICStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    //    CAutoLock cAutoLock(m_pFilter->pStateLock());

    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_nBufferSize;

    ALLOCATOR_PROPERTIES Actual;
    if (FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) {
        return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CFLICStream::FillBuffer(IMediaSample* pSample)
{
    HRESULT hr;

    {
        CAutoLock cAutoLockShared(&m_cSharedState);

        if (m_rtPosition >= m_rtStop) {
            return S_FALSE;
        }

        BYTE* pDataIn = m_pFrameBuffer;
        BYTE* pDataOut = NULL;
        if (!pDataIn || FAILED(hr = pSample->GetPointer(&pDataOut)) || !pDataOut) {
            return S_FALSE;
        }

        AM_MEDIA_TYPE* pmt;
        if (SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt) {
            CMediaType mt(*pmt);
            SetMediaType(&mt);

            DeleteMediaType(pmt);
        }

        int w, h, bpp;
        if (m_mt.formattype == FORMAT_VideoInfo) {
            w = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biWidth;
            h = abs(((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biHeight);
            bpp = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biBitCount;
        } else if (m_mt.formattype == FORMAT_VideoInfo2) {
            w = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biWidth;
            h = abs(((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biHeight);
            bpp = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biBitCount;
        } else {
            return S_FALSE;
        }

        int pitchIn = m_hdr.width;
        int pitchOut = w * bpp >> 3;

        int nFrame = (int)(m_rtPosition / m_AvgTimePerFrame);

        {
            SeekToNearestKeyFrame(nFrame);

            while (m_nLastFrameNum < nFrame && !m_bFlushing) {
                ExtractFrame(++m_nLastFrameNum);
            }

            for (int y = 0, p = min(pitchIn, pitchOut);
                    y < h;
                    y++, pDataIn += pitchIn, pDataOut += pitchOut) {
                BYTE* src = pDataIn;
                BYTE* end = src + p;
                DWORD* dst = (DWORD*)pDataOut;
                while (src < end) {
                    *dst++ = m_pPalette[*src++];
                }
            }
        }

        pSample->SetActualDataLength(pitchOut * h);

        REFERENCE_TIME rtStart, rtStop;
        // The sample times are modified by the current rate.
        rtStart = static_cast<REFERENCE_TIME>(m_rtSampleTime / m_dRateSeeking);
        rtStop  = rtStart + static_cast<int>(m_AvgTimePerFrame / m_dRateSeeking);
        pSample->SetTime(&rtStart, &rtStop);

        m_rtSampleTime += m_AvgTimePerFrame;
        m_rtPosition += m_AvgTimePerFrame;
    }

    pSample->SetSyncPoint(TRUE);

    if (m_bDiscontinuity) {
        pSample->SetDiscontinuity(TRUE);
        m_bDiscontinuity = FALSE;
    }

    return S_OK;
}

HRESULT CFLICStream::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(TRUE);

    VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
    memset(vih, 0, sizeof(VIDEOINFOHEADER));
    vih->AvgTimePerFrame = m_AvgTimePerFrame;
    vih->bmiHeader.biSize = sizeof(vih->bmiHeader);
    vih->bmiHeader.biWidth = m_hdr.width;
    vih->bmiHeader.biHeight = -m_hdr.height;
    vih->bmiHeader.biPlanes = 1;
    vih->bmiHeader.biBitCount = 32;
    vih->bmiHeader.biCompression = BI_RGB;
    vih->bmiHeader.biSizeImage = m_nBufferSize;

    pmt->SetSampleSize(vih->bmiHeader.biSizeImage);

    return NOERROR;
}

HRESULT CFLICStream::CheckConnect(IPin* pPin)
{
    return CSourceStream::CheckConnect(pPin);
}

HRESULT CFLICStream::CheckMediaType(const CMediaType* pmt)
{
    if (pmt->majortype == MEDIATYPE_Video
            && pmt->subtype == MEDIASUBTYPE_RGB32
            && pmt->formattype == FORMAT_VideoInfo) {
        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CFLICStream::Notify(IBaseFilter* pSender, Quality q)
{
    return E_NOTIMPL;
}

////////

void CFLICStream::SeekToNearestKeyFrame(int nFrame)
{
    if (m_nLastFrameNum == nFrame) {
        return;
    }

    if (m_nLastFrameNum > nFrame) {
        m_nLastFrameNum = -1;
    }

    for (int i = m_nLastFrameNum + 1, j = min((int)m_frames.GetCount(), nFrame); i < j; i++) {
        FLIC_FRAME_ENTRY& ffe = m_frames[i];
        if (ffe.fKeyframe) {
            m_nLastFrameNum = i - 1;
        }
    }
}

void CFLICStream::ExtractFrame(int nFrame)
{
    FLIC_FRAME_ENTRY& ffe = m_frames[nFrame];

    bool fNewPalette = false;
    bool fNewFrame = false;

    m_flic.Seek(ffe.pos, CFile::begin);

    int chunk = 0;
    while (chunk < ffe.hdr.chunks) {
        FLIC_CHUNK fc;
        if (m_flic.Read(&fc, sizeof(fc)) != sizeof(fc)) {
            break;
        }

        ULONGLONG next = m_flic.GetPosition() + fc.size - sizeof(fc);
        if (next >= m_flic.GetLength()) {
            break;
        }

        switch (fc.type) {
            case FLIC_64_COLOR:
                fNewPalette = _colorchunk(true);
                break;
            case FLIC_256_COLOR:
                fNewPalette = _colorchunk(false);
                break;
            case FLIC_BRUN:
                _brunchunk();
                fNewFrame = true;
                break;
            case FLIC_LC:
                _lcchunk();
                break;
            case FLIC_DELTA:
                _deltachunk();
                break;
            case FLIC_BLACK:
                _blackchunk();
                fNewFrame = true;
                break;
            case FLIC_COPY:
                _copychunk();
                fNewFrame = true;
                break;
            case FLIC_MINI:
                break;
            default:
                break;
        }

        m_flic.Seek(next, CFile::begin);

        chunk++;
    }

    if (chunk < ffe.hdr.chunks) {
        ASSERT(0);
    }

    ffe.fKeyframe = (fNewPalette && fNewFrame);
}

void CFLICStream::_blackchunk()
{
    memset(m_pFrameBuffer, 0, m_nBufferSize);
}

void CFLICStream::_copychunk()
{
    m_flic.Read(m_pFrameBuffer, m_nBufferSize);
}

bool CFLICStream::_colorchunk(bool f64)
{
    int nColorsUpdated = 0;

    BYTE skip = 0;

    WORD packets;
    m_flic.Read(&packets, sizeof(packets));

    while (packets--) {
        BYTE skip2;
        m_flic.Read(&skip2, sizeof(skip2));
        skip += skip2;

        BYTE count;
        m_flic.Read(&count, sizeof(count));

        int len = (count == 0 ? (256 - skip) : count);
        while (len-- > 0) {
            BYTE r, g, b;
            m_flic.Read(&r, sizeof(r));
            m_flic.Read(&g, sizeof(g));
            m_flic.Read(&b, sizeof(b));
            m_pPalette[skip++] = f64
                                 ? ((r << 18) & 0xff0000) | ((g << 10) & 0xff00) | ((b << 2) & 0xff)
                                 : ((r << 16) & 0xff0000) | ((g << 8) & 0xff00) | ((b << 0) & 0xff);
            nColorsUpdated++;
        }
    }

    return (nColorsUpdated == 256);
}

void CFLICStream::_brunchunk()
{
    BYTE* tmp = m_pFrameBuffer;

    int lines = m_hdr.height;
    while (lines--) {
        BYTE packets;
        m_flic.Read(&packets, sizeof(packets));

        BYTE* ptr = tmp;

        while (ptr < tmp + m_hdr.width) {
            signed char count;
            m_flic.Read(&count, sizeof(count));

            if (count >= 0) {
                BYTE c;
                m_flic.Read(&c, sizeof(c));
                memset(ptr, c, count);
                ptr += count;
            } else {
                m_flic.Read(ptr, -count);
                ptr += -count;
            }
        }

        tmp += m_hdr.width;
    }
}

void CFLICStream::_lcchunk()
{
    WORD y;
    m_flic.Read(&y, sizeof(y));

    BYTE* tmp = &m_pFrameBuffer[y * m_hdr.width];

    WORD lines;
    m_flic.Read(&lines, sizeof(lines));

    while (lines--) {
        BYTE* ptr = tmp;

        BYTE packets;
        m_flic.Read(&packets, sizeof(packets));

        while (packets--) {
            BYTE skip;
            m_flic.Read(&skip, sizeof(skip));

            ptr += skip;

            signed char count;
            m_flic.Read(&count, sizeof(count));

            if (count >= 0) {
                m_flic.Read(ptr, count);
                ptr += count;
            } else {
                BYTE c;
                m_flic.Read(&c, sizeof(c));
                memset(ptr, c, -count);
                ptr += -count;
            }
        }

        tmp += m_hdr.width;
    }
}

void CFLICStream::_deltachunk()
{
    BYTE* tmp = m_pFrameBuffer;

    WORD lines;
    m_flic.Read(&lines, sizeof(lines));

    while (lines--) {
        signed short packets;
        m_flic.Read(&packets, sizeof(packets));

        if (packets < 0) {
            if (packets & 0x4000) {
                tmp += -packets * m_hdr.width;
                lines++;
            } else {
                signed char count;
                m_flic.Read(&count, sizeof(count));
                tmp[m_hdr.width - 1] = (BYTE)packets;
            }
        } else {
            BYTE* ptr = tmp;

            while (packets--) {
                BYTE skip;
                m_flic.Read(&skip, sizeof(skip));

                ptr += skip;

                signed char count;
                m_flic.Read(&count, sizeof(count));

                if (count >= 0) {
                    // Fix vulnerability : http://www.team509.com/modules.php?name=News&file=article&sid=38
                    if ((count << 1) + (long)(ptr - m_pFrameBuffer) < m_nBufferSize) {
                        m_flic.Read(ptr, count << 1);
                    } else {
                        ASSERT(FALSE);
                    }
                    ptr += count << 1;
                } else {
                    WORD c;
                    m_flic.Read(&c, sizeof(c));
                    count = -count;
                    while (count-- > 0) {
                        *ptr++ = c >> 8;
                        *ptr++ = c & 0xff;
                    }
                }
            }

            tmp += m_hdr.width;
        }
    }
}
