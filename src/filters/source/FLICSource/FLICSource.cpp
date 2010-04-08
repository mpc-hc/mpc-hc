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

#include "stdafx.h"
#include "FLICSource.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
    {&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32},
};

const AMOVIESETUP_PIN sudOpPin[] =
{
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
    {&__uuidof(CFLICSource), L"MPC - FLICSource", MERIT_NORMAL, countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] =
{
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CFLICSource>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

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
    if(GetPinCount() > 0)
        return VFW_E_ALREADY_CONNECTED;

    HRESULT hr = S_OK;
    if(!(DNew CFLICStream(pszFileName, this, &hr)))
        return E_OUTOFMEMORY;

    if(FAILED(hr))
        return hr;

    m_fn = pszFileName;

    return S_OK;
}

STDMETHODIMP CFLICSource::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    if(!ppszFileName) return E_POINTER;

    if(!(*ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR))))
        return E_OUTOFMEMORY;

    wcscpy(*ppszFileName, m_fn);

    return S_OK;
}

// IAMFilterMiscFlags

ULONG CFLICSource::GetMiscFlags()
{
    return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}

// CFLICStream

CFLICStream::CFLICStream(const WCHAR* wfn, CFLICSource* pParent, HRESULT* phr)
    : CSourceStream(NAME("FLICStream"), phr, pParent, L"Output")
    , CSourceSeeking(NAME("FLICStream"), (IPin*)this, phr, &m_cSharedState)
    , m_bDiscontinuity(FALSE), m_bFlushing(FALSE)
{
    CAutoLock cAutoLock(&m_cSharedState);

    CString fn(wfn);

    if(!m_flic.Open(fn, CFile::modeRead | CFile::shareDenyNone))
    {
        if(phr) *phr = E_FAIL;
        return;
    }

    if(m_flic.Read(&m_hdr, sizeof(m_hdr)) != sizeof(m_hdr)
       || (m_hdr.id != 0xaf11 && m_hdr.id != 0xaf12)
       || m_hdr.bpp != 8)
    {
        if(phr) *phr = E_FAIL;
        return;
    }

    m_AvgTimePerFrame = (m_hdr.id == 0xaf11)
                        ? 10000000i64 * max(m_hdr.ticks, 1) / 70
                        : 10000000i64 * max(m_hdr.ticks, 1) / 1000;

    // not tested (lack of test files)
    {
        __int64 pos = m_flic.GetPosition();
        FLIC_PREFIX fp;
        if(m_flic.Read(&fp, sizeof(fp)) != sizeof(fp) || fp.id != 0xf100)
            m_flic.Seek(pos, CFile::begin);
        else
            m_flic.Seek(pos + fp.size, CFile::begin);
    }

    do
    {
        FLIC_FRAME_ENTRY ffe;
        if(m_flic.Read(&ffe.hdr, sizeof(ffe.hdr)) != sizeof(ffe.hdr) || ffe.hdr.id != 0xf1fa)
            break;
        ffe.pos = m_flic.GetPosition();
        ffe.fKeyframe = (m_frames.GetCount() == 0);

        int chunk = 0;
        while(chunk < ffe.hdr.chunks)
        {
            FLIC_CHUNK fc;
            if(m_flic.Read(&fc, sizeof(fc)) != sizeof(fc))
                break;
            /*
            			switch(fc.type)
            			{
            				case FLIC_COLOR: _colorchunk(); break;
            				case FLIC_256_COLOR: _color256chunk(); break;
            				case FLIC_BRUN: _brunchunk(); break;
            				case FLIC_LC: _lcchunk(); break;
            				case FLIC_DELTA: _deltachunk(); break;
            				case FLIC_BLACK: _blackchunk(); break;
            				case FLIC_COPY: _copychunk(); break;
            				case FLIC_MINI: break;
            				default: break;
            			}
            */
            ffe.fKeyframe =
                (/*fc.type == FLIC_256_COLOR
				|| fc.type == FLIC_64_COLOR
				||*/ fc.type == FLIC_BRUN
                    || fc.type == FLIC_BLACK
                    || fc.type == FLIC_COPY);

            __int64 pos = m_flic.GetPosition() + fc.size - sizeof(fc);
            if(m_flic.Seek(pos, CFile::begin) != pos)
                break;

            chunk++;
        }
        if(chunk < ffe.hdr.chunks)
            break;

        __int64 pos = ffe.pos + ffe.hdr.size - sizeof(ffe.hdr);
        if(m_flic.Seek(pos, CFile::begin) != pos)
            break;

        m_frames.Add(ffe);
    }
    while(1);

    m_nLastFrameNum = -1;
    memset(m_pPalette, 0, sizeof(m_pPalette));
    m_nBufferSize = m_hdr.x * m_hdr.y * 32 >> 3;
    if(!m_pFrameBuffer.Allocate(m_nBufferSize))
    {
        if(phr) *phr = E_OUTOFMEMORY;
        return;
    }

    m_rtDuration = m_rtStop = m_AvgTimePerFrame * m_frames.GetCount();

    if(phr) *phr = m_rtDuration > 0 ? S_OK : E_FAIL;
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
    if(ThreadExists())
    {
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
    if(dRate <= 0)
        return E_INVALIDARG;

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
        if(m_rtPosition < m_rtStop)
            return S_OK;
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
    if(FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) return hr;

    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CFLICStream::FillBuffer(IMediaSample* pSample)
{
    HRESULT hr;

    {
        CAutoLock cAutoLockShared(&m_cSharedState);

        if(m_rtPosition >= m_rtStop)
            return S_FALSE;

        BYTE* pDataIn = m_pFrameBuffer;
        BYTE* pDataOut = NULL;
        if(!pDataIn || FAILED(hr = pSample->GetPointer(&pDataOut)) || !pDataOut)
            return S_FALSE;

        AM_MEDIA_TYPE* pmt;
        if(SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt)
        {
            CMediaType mt(*pmt);
            SetMediaType(&mt);

            DeleteMediaType(pmt);
        }

        int w, h, bpp;
        if(m_mt.formattype == FORMAT_VideoInfo)
        {
            w = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biWidth;
            h = abs(((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biHeight);
            bpp = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biBitCount;
        }
        else if(m_mt.formattype == FORMAT_VideoInfo2)
        {
            w = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biWidth;
            h = abs(((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biHeight);
            bpp = ((VIDEOINFOHEADER2*)m_mt.pbFormat)->bmiHeader.biBitCount;
        }
        else
        {
            return S_FALSE;
        }

        int pitchIn = m_hdr.x;
        int pitchOut = w * bpp >> 3;

        int nFrame = m_rtPosition / m_AvgTimePerFrame; // (int)(1.0 * m_rtPosition / m_AvgTimePerFrame + 0.5);

        {
            SeekToNearestKeyFrame(nFrame);

            while(m_nLastFrameNum < nFrame && !m_bFlushing)
                ExtractFrame(++m_nLastFrameNum);

            for(int y = 0, p = min(pitchIn, pitchOut);
                y < h;
                y++, pDataIn += pitchIn, pDataOut += pitchOut)
            {
                BYTE* src = pDataIn;
                BYTE* end = src + p;
                DWORD* dst = (DWORD*)pDataOut;
                while(src < end) *dst++ = m_pPalette[*src++];
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

    if(m_bDiscontinuity)
    {
        pSample->SetDiscontinuity(TRUE);
        m_bDiscontinuity = FALSE;
    }

    return S_OK;
}

HRESULT CFLICStream::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if(iPosition < 0) return E_INVALIDARG;
    if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(TRUE);

    VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
    memset(vih, 0, sizeof(VIDEOINFOHEADER));
    vih->AvgTimePerFrame = m_AvgTimePerFrame;
    vih->bmiHeader.biSize = sizeof(vih->bmiHeader);
    vih->bmiHeader.biWidth = m_hdr.x;
    vih->bmiHeader.biHeight = -m_hdr.y;
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
    if(pmt->majortype == MEDIATYPE_Video
       && pmt->subtype == MEDIASUBTYPE_RGB32
       && pmt->formattype == FORMAT_VideoInfo) return S_OK;

    return E_INVALIDARG;
}

STDMETHODIMP CFLICStream::Notify(IBaseFilter* pSender, Quality q)
{
    return E_NOTIMPL;
}

////////

void CFLICStream::SeekToNearestKeyFrame(int nFrame)
{
    if(m_nLastFrameNum == nFrame)
        return;

    if(m_nLastFrameNum > nFrame)
        m_nLastFrameNum = -1;

    for(int i = m_nLastFrameNum + 1, j = min(m_frames.GetCount(), nFrame); i < j; i++)
    {
        FLIC_FRAME_ENTRY& ffe = m_frames[i];
        if(ffe.fKeyframe)
            m_nLastFrameNum = i - 1;
    }
}

void CFLICStream::ExtractFrame(int nFrame)
{
    FLIC_FRAME_ENTRY& ffe = m_frames[nFrame];

    bool fNewPalette = false;
    bool fNewFrame = false;

    m_flic.Seek(ffe.pos, CFile::begin);

    int chunk = 0;
    while(chunk < ffe.hdr.chunks)
    {
        FLIC_CHUNK fc;
        if(m_flic.Read(&fc, sizeof(fc)) != sizeof(fc))
            break;

        __int64 next = m_flic.GetPosition() + fc.size - sizeof(fc);

        switch(fc.type)
        {
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

        if(m_flic.Seek(next, CFile::begin) != next)
            break;

        chunk++;
    }

    if(chunk < ffe.hdr.chunks)
        ASSERT(0);

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

    while(packets--)
    {
        BYTE skip2;
        m_flic.Read(&skip2, sizeof(skip2));
        skip += skip2;

        BYTE count;
        m_flic.Read(&count, sizeof(count));

        int len = (count == 0 ? (256 - skip) : count);
        while(len-- > 0)
        {
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

    return(nColorsUpdated == 256);
}

void CFLICStream::_brunchunk()
{
    BYTE* tmp = m_pFrameBuffer;

    int lines = m_hdr.y;
    while(lines--)
    {
        BYTE packets;
        m_flic.Read(&packets, sizeof(packets));

        BYTE* ptr = tmp;

        while(ptr < tmp + m_hdr.x)
        {
            signed char count;
            m_flic.Read(&count, sizeof(count));

            if(count >= 0)
            {
                BYTE c;
                m_flic.Read(&c, sizeof(c));
                memset(ptr, c, count);
                ptr += count;
            }
            else
            {
                m_flic.Read(ptr, -count);
                ptr += -count;
            }
        }

        tmp += m_hdr.x;
    }
}

void CFLICStream::_lcchunk()
{
    WORD y;
    m_flic.Read(&y, sizeof(y));

    BYTE* tmp = &m_pFrameBuffer[y*m_hdr.x];

    WORD lines;
    m_flic.Read(&lines, sizeof(lines));

    while(lines--)
    {
        BYTE* ptr = tmp;

        BYTE packets;
        m_flic.Read(&packets, sizeof(packets));

        while(packets--)
        {
            BYTE skip;
            m_flic.Read(&skip, sizeof(skip));

            ptr += skip;

            signed char count;
            m_flic.Read(&count, sizeof(count));

            if(count >= 0)
            {
                m_flic.Read(ptr, count);
                ptr += count;
            }
            else
            {
                BYTE c;
                m_flic.Read(&c, sizeof(c));
                memset(ptr, c, -count);
                ptr += -count;
            }
        }

        tmp += m_hdr.x;
    }
}

void CFLICStream::_deltachunk()
{
    BYTE* tmp = m_pFrameBuffer;

    WORD lines;
    m_flic.Read(&lines, sizeof(lines));

    while(lines--)
    {
        signed short packets;
        m_flic.Read(&packets, sizeof(packets));

        if(packets < 0)
        {
            if(packets & 0x4000)
            {
                tmp += -packets * m_hdr.x;
                lines++;
            }
            else
            {
                signed char count;
                m_flic.Read(&count, sizeof(count));
                tmp[m_hdr.x-1] = (BYTE)packets;
            }
        }
        else
        {
            BYTE* ptr = tmp;

            while(packets--)
            {
                BYTE skip;
                m_flic.Read(&skip, sizeof(skip));

                ptr += skip;

                signed char count;
                m_flic.Read(&count, sizeof(count));

                if(count >= 0)
                {
                    // Fix vulnerability : http://www.team509.com/modules.php?name=News&file=article&sid=38
                    if((count << 1) + (long)(ptr - m_pFrameBuffer) < m_nBufferSize)
                        m_flic.Read(ptr, count << 1);
                    else
                        ASSERT(FALSE);
                    ptr += count << 1;
                }
                else
                {
                    WORD c;
                    m_flic.Read(&c, sizeof(c));
                    count = -count;
                    while(count-- > 0)
                    {
                        *ptr++ = c >> 8;
                        *ptr++ = c & 0xff;
                    }
                }
            }

            tmp += m_hdr.x;
        }
    }
}
