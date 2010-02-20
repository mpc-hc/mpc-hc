/* 
 *    Copyright (C) 2003-2004 Gabest
 *    http://www.gabest.org
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
#include <mmreg.h>
#include <initguid.h>
#include "DiracSplitter.h"
#include <moreuuids.h>

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{    
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_Dirac},
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn2[] =
{
    {&MEDIATYPE_Video, &MEDIASUBTYPE_DiracVideo},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut2[] =
{
    {&MEDIATYPE_Video, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins2[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn2), sudPinTypesIn2},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut2), sudPinTypesOut2}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
    {&__uuidof(CDiracSplitterFilter), L"MPC - Dirac Splitter", MERIT_NORMAL, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CDiracSourceFilter), L"MPC - Dirac Source", MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CDiracVideoDecoder), L"MPC - Dirac Video Decoder", MERIT_UNLIKELY, countof(sudpPins2), sudpPins2, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] =
{
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CDiracSplitterFilter>, NULL, &sudFilter[0]},
    {sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CDiracSourceFilter>, NULL, &sudFilter[1]},
    {sudFilter[2].strName, sudFilter[2].clsID, CreateInstance<CDiracVideoDecoder>, NULL, &sudFilter[2]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
    RegisterSourceFilter(
        CLSID_AsyncReader, 
        MEDIASUBTYPE_Dirac, 
        _T("0,8,,4B572D4449524143"), // KW-DIRAC
        _T(".drc"), NULL);

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    UnRegisterSourceFilter(MEDIASUBTYPE_Dirac);

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"
CFilterApp theApp;

#endif

//
// CDiracSplitterFilter
//

CDiracSplitterFilter::CDiracSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CBaseSplitterFilter(NAME("CDiracSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

STDMETHODIMP CDiracSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return 
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CDiracSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
    CheckPointer(pAsyncReader, E_POINTER);

    HRESULT hr = E_FAIL;

    m_pFile.Free();

    m_pFile.Attach(DNew CDiracSplitterFile(pAsyncReader, hr));
    if(!m_pFile) return E_OUTOFMEMORY;
    if(FAILED(hr)) {m_pFile.Free(); return hr;}

    CAtlArray<CMediaType> mts;
    mts.Add(m_pFile->GetMediaType());

    CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, L"Video", this, this, &hr));
    AddOutputPin(0, pPinOut);

    m_rtNewStart = m_rtCurrent = 0;
    m_rtNewStop = m_rtStop = m_rtDuration = m_pFile->GetDuration();

    return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CDiracSplitterFilter::DemuxInit()
{
    if(!m_pFile) return(false);

    // TODO

    return(true);
}

void CDiracSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
    REFERENCE_TIME rtPreroll = 0; //10000000;

    if(rt <= rtPreroll || m_rtDuration <= 0)
    {
        //m_pFile->Seek(8);
        m_pFile->Seek(0);
    }
    else
    {
        // TODO

        __int64 len = m_pFile->GetLength();
        __int64 seekpos = (__int64)(1.0*rt/m_rtDuration*len);

        m_pFile->Seek(seekpos);
        seekpos = 0;

        REFERENCE_TIME rtmax = rt - rtPreroll;
        REFERENCE_TIME rtmin = rtmax - 5000000;

        REFERENCE_TIME pdt = _I64_MIN;

        BYTE code;
        for(int j = 0; j < 10; j++)
        {
            while(m_pFile->Next(code) && code != AU_START_CODE);
            if(code != AU_START_CODE) {m_pFile->Seek(seekpos >>= 1); continue;}

            __int64 pos = m_pFile->GetPos() - 5;

            REFERENCE_TIME rt = ((DIRACINFOHEADER*)m_pFile->GetMediaType().Format())->hdr.AvgTimePerFrame * m_pFile->UnsignedGolombDecode();
            REFERENCE_TIME dt = rt - rtmax;
            if(dt > 0 && dt == pdt) dt = 10000000i64;

            if(rtmin <= rt && rt <= rtmax || pdt > 0 && dt < 0)
            {
                seekpos = pos;
                break;
            }

            m_pFile->Seek(pos - (__int64)(1.0*dt/m_rtDuration*len));

            pdt = dt;
        }

        m_pFile->Seek(seekpos);
    }
}

bool CDiracSplitterFilter::DemuxLoop()
{
    HRESULT hr = S_OK;
    REFERENCE_TIME rtAvgTimePerFrame = ((DIRACINFOHEADER*)m_pFile->GetMediaType().Format())->hdr.AvgTimePerFrame;

    BYTE code;
    while(SUCCEEDED(hr) && !CheckRequest(NULL))
    {
        int size = 0, fnum = 0;
        const BYTE* pBuff = m_pFile->NextBlock(code, size, fnum);
        if(!pBuff || size < 5) break;

        if(isFrameStartCode(code))
        {
            CAutoPtr<Packet> p(DNew Packet());
            p->SetCount(size);
            memcpy(p->GetData(), pBuff, size);

            p->TrackNumber = 0;
            p->rtStart = rtAvgTimePerFrame*fnum;
            p->rtStop = p->rtStart + rtAvgTimePerFrame;
            p->bSyncPoint = code == AU_START_CODE;

            hr = DeliverPacket(p);
        }

        if(code == SEQ_END_CODE)
            break;
    }

    return(true);
}

//
// CDiracSourceFilter
//

CDiracSourceFilter::CDiracSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CDiracSplitterFilter(pUnk, phr)
{
    m_clsid = __uuidof(this);
    m_pInput.Free();
}

//
// CDiracVideoDecoder
//

CDiracVideoDecoder::CDiracVideoDecoder(LPUNKNOWN lpunk, HRESULT* phr)
    : CTransformFilter(NAME("CDiracVideoDecoder"), lpunk, __uuidof(this))
{
    if(phr) *phr = S_OK;

    m_decoder = NULL;
    m_pYUV[0] = NULL;
}

CDiracVideoDecoder::~CDiracVideoDecoder()
{
    delete [] m_pYUV[0];
}

void CDiracVideoDecoder::InitDecoder()
{
    FreeDecoder();

    dirac_decoder_t* decoder = dirac_decoder_init(0);

    DIRACINFOHEADER* dvih = (DIRACINFOHEADER*)m_pInput->CurrentMediaType().Format();
    dirac_buffer(decoder, (BYTE*)&dvih->dwSequenceHeader[0], (BYTE*)&dvih->dwSequenceHeader[0] + dvih->cbSequenceHeader);

    m_decoder = decoder;
}

void CDiracVideoDecoder::FreeDecoder()
{
    if(m_decoder)
    {
        dirac_decoder_close((dirac_decoder_t*)m_decoder);
        m_decoder = NULL;
        delete [] m_pYUV[0]; m_pYUV[0] = NULL;
    }
}

HRESULT CDiracVideoDecoder::Receive(IMediaSample* pIn)
{
    CAutoLock cAutoLock(&m_csReceive);

    HRESULT hr;

    AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
    if(pProps->dwStreamId != AM_STREAM_MEDIA)
        return m_pOutput->Deliver(pIn);

    BYTE* pDataIn = NULL;
    if(FAILED(hr = pIn->GetPointer(&pDataIn))) return hr;

    long len = pIn->GetActualDataLength();
    if(len <= 0) return S_OK; // nothing to do

    if(pIn->IsDiscontinuity() == S_OK) 
        InitDecoder();

    dirac_decoder_t* decoder = (dirac_decoder_t*)m_decoder;

    hr = S_OK;
    while(SUCCEEDED(hr))
    {
        switch(dirac_parse(decoder))
        {
        case STATE_BUFFER:
            if(len == 0) return S_OK;
            dirac_buffer(decoder, pDataIn, pDataIn + len);
            len = 0;
            break;

        case STATE_SEQUENCE:
            TRACE(_T("STATE_SEQUENCE\n"));

            {
                DIRACINFOHEADER* dvih = (DIRACINFOHEADER*)m_pInput->CurrentMediaType().Format();
                if(dvih->hdr.bmiHeader.biWidth != decoder->src_params.width
                || dvih->hdr.bmiHeader.biHeight != decoder->src_params.height)
                    return E_FAIL; // hmm
            }

            if(!m_pYUV[0])
            {
                int w = decoder->src_params.width;
                int h = decoder->src_params.height;
                int wc = decoder->src_params.chroma_width;
                int hc = decoder->src_params.chroma_height; 
                delete [] m_pYUV[0]; m_pYUV[0] = NULL;
                m_pYUV[0] = DNew BYTE[w*h + wc*hc*2 + w/2*h/2];
                m_pYUV[1] = m_pYUV[0] + w*h;
                m_pYUV[2] = m_pYUV[1] + wc*hc;
                m_pYUV[3] = m_pYUV[2] + wc*hc;
                memset(m_pYUV[3], 0x80, w/2*h/2);
                m_rtAvgTimePerFrame = 10000000i64 * decoder->src_params.frame_rate.denominator / decoder->src_params.frame_rate.numerator;
                dirac_set_buf(decoder, m_pYUV, NULL);
            }

            break;

        case STATE_SEQUENCE_END:
            TRACE(_T("STATE_SEQUENCE_END\n"));
            break;

        case STATE_INVALID:
            TRACE(_T("STATE_INVALID\n"));
            return E_FAIL; // TODO: can we recover from this state?
            // break;

        default:
            TRACE(_T("unknown state\n"));
            continue;
        }
    }

    return hr;
}

HRESULT CDiracVideoDecoder::Deliver(IMediaSample* pIn, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop)
{
    CheckPointer(pIn, E_POINTER);

    HRESULT hr;

    if(pIn->IsPreroll() == S_OK || rtStart < 0)
        return S_OK;

    CComPtr<IMediaSample> pOut;
    BYTE* pDataOut = NULL;
    if(FAILED(hr = m_pOutput->GetDeliveryBuffer(&pOut, NULL, NULL, 0))
    || FAILED(hr = pOut->GetPointer(&pDataOut)))
        return hr;

    AM_MEDIA_TYPE* pmt;
    if(SUCCEEDED(pOut->GetMediaType(&pmt)) && pmt)
    {
        CMediaType mt(*pmt);
        m_pOutput->SetMediaType(&mt);
        DeleteMediaType(pmt);
    }

    TRACE(_T("CDiracVideoDecoder::Deliver(%I64d, %I64d)\n"), rtStart, rtStop);

    pOut->SetTime(&rtStart, &rtStop);
    pOut->SetMediaTime(NULL, NULL);

    pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);
    pOut->SetSyncPoint(TRUE);

    if(GetCLSID(m_pOutput->GetConnected()) == CLSID_OverlayMixer)
        pOut->SetDiscontinuity(TRUE);

    Copy(pDataOut);

    return m_pOutput->Deliver(pOut);
}

void CDiracVideoDecoder::Copy(BYTE* pOut)
{
    // FIXME: modes other than I420 and Y-only

    BITMAPINFOHEADER bihOut;
    ExtractBIH(&m_pOutput->CurrentMediaType(), &bihOut);

    dirac_decoder_t* decoder = (dirac_decoder_t*)m_decoder;

    int w = decoder->src_params.width;
    int h = decoder->src_params.height;
    int wc = decoder->src_params.chroma_width;
    int hc = decoder->src_params.chroma_height;

    int pitchIn = w;

    BYTE* pY = m_pYUV[0];
    BYTE* pU = w/2 == wc && h/2 == hc ? m_pYUV[1] : m_pYUV[3]; // FIXME
    BYTE* pV = w/2 == wc && h/2 == hc ? m_pYUV[2] : m_pYUV[3]; // FIXME

    if(bihOut.biCompression == '2YUY')
    {
        BitBltFromI420ToYUY2(w, h, pOut, bihOut.biWidth*2, pY, pU, pV, pitchIn);
    }
    else if(bihOut.biCompression == 'I420' || bihOut.biCompression == 'VUYI')
    {
        BitBltFromI420ToI420(w, h, pOut, pOut + bihOut.biWidth*h, pOut + bihOut.biWidth*h*5/4, bihOut.biWidth, pY, pU, pV, pitchIn);
    }
    else if(bihOut.biCompression == '21VY')
    {
        BitBltFromI420ToI420(w, h, pOut, pOut + bihOut.biWidth*h*5/4, pOut + bihOut.biWidth*h, bihOut.biWidth, pY, pU, pV, pitchIn);
    }
    else if(bihOut.biCompression == BI_RGB || bihOut.biCompression == BI_BITFIELDS)
    {
        int pitchOut = bihOut.biWidth*bihOut.biBitCount>>3;

        if(bihOut.biHeight > 0)
        {
            pOut += pitchOut*(h-1);
            pitchOut = -pitchOut;
        }

        if(!BitBltFromI420ToRGB(w, h, pOut, pitchOut, bihOut.biBitCount, pY, pU, pV, pitchIn))
        {
            for(DWORD y = 0; y < h; y++, pOut += pitchOut)
                memset(pOut, 0, pitchOut);
        }
    }
}

HRESULT CDiracVideoDecoder::CheckInputType(const CMediaType* mtIn)
{
    DIRACINFOHEADER* dvih = (DIRACINFOHEADER*)mtIn->Format();

    if(mtIn->majortype != MEDIATYPE_Video 
    || mtIn->subtype != MEDIASUBTYPE_DiracVideo
    || mtIn->formattype != FORMAT_DiracVideoInfo
    || (dvih->hdr.bmiHeader.biWidth&1) || (dvih->hdr.bmiHeader.biHeight&1))
        return VFW_E_TYPE_NOT_ACCEPTED;

    return S_OK;
}

HRESULT CDiracVideoDecoder::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    if(m_pOutput && m_pOutput->IsConnected())
    {
        BITMAPINFOHEADER bih1, bih2;
        if(ExtractBIH(mtOut, &bih1) && ExtractBIH(&m_pOutput->CurrentMediaType(), &bih2)
        && abs(bih1.biHeight) != abs(bih2.biHeight))
            return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return mtIn->majortype == MEDIATYPE_Video && mtIn->subtype == MEDIASUBTYPE_DiracVideo
        && mtOut->majortype == MEDIATYPE_Video && (mtOut->subtype == MEDIASUBTYPE_YUY2
                                                || mtOut->subtype == MEDIASUBTYPE_YV12
                                                || mtOut->subtype == MEDIASUBTYPE_I420
                                                || mtOut->subtype == MEDIASUBTYPE_IYUV
                                                || mtOut->subtype == MEDIASUBTYPE_ARGB32
                                                || mtOut->subtype == MEDIASUBTYPE_RGB32
                                                || mtOut->subtype == MEDIASUBTYPE_RGB24
                                                || mtOut->subtype == MEDIASUBTYPE_RGB565
                                                || mtOut->subtype == MEDIASUBTYPE_RGB555)
        ? S_OK
        : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CDiracVideoDecoder::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
    if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

    BITMAPINFOHEADER bih;
    ExtractBIH(&m_pOutput->CurrentMediaType(), &bih);

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = bih.biSizeImage;
    pProperties->cbAlign = 1;
    pProperties->cbPrefix = 0;

    HRESULT hr;
    ALLOCATOR_PROPERTIES Actual;
    if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) 
        return hr;

    return(pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
        ? E_FAIL
        : NOERROR);
}

HRESULT CDiracVideoDecoder::GetMediaType(int iPosition, CMediaType* pmt)
{
    if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

    struct {const GUID* subtype; WORD biPlanes, biBitCount; DWORD biCompression;} fmts[] =
    {
        {&MEDIASUBTYPE_YV12, 3, 12, '21VY'},
        {&MEDIASUBTYPE_I420, 3, 12, '024I'},
        {&MEDIASUBTYPE_IYUV, 3, 12, 'VUYI'},
        {&MEDIASUBTYPE_YUY2, 1, 16, '2YUY'},
        {&MEDIASUBTYPE_ARGB32, 1, 32, BI_RGB},
        {&MEDIASUBTYPE_RGB32, 1, 32, BI_RGB},
        {&MEDIASUBTYPE_RGB24, 1, 24, BI_RGB},
        {&MEDIASUBTYPE_RGB565, 1, 16, BI_RGB},
        {&MEDIASUBTYPE_RGB555, 1, 16, BI_RGB},
        {&MEDIASUBTYPE_ARGB32, 1, 32, BI_BITFIELDS},
        {&MEDIASUBTYPE_RGB32, 1, 32, BI_BITFIELDS},
        {&MEDIASUBTYPE_RGB24, 1, 24, BI_BITFIELDS},
        {&MEDIASUBTYPE_RGB565, 1, 16, BI_BITFIELDS},
        {&MEDIASUBTYPE_RGB555, 1, 16, BI_BITFIELDS},
    };

    if(m_pInput->CurrentMediaType().formattype == FORMAT_VideoInfo)
        iPosition = iPosition*2 + 1;

    if(iPosition < 0) return E_INVALIDARG;
    if(iPosition >= 2*countof(fmts)) return VFW_S_NO_MORE_ITEMS;

    BITMAPINFOHEADER bih;
    ExtractBIH(&m_pInput->CurrentMediaType(), &bih);

    pmt->majortype = MEDIATYPE_Video;
    pmt->subtype = *fmts[iPosition/2].subtype;

    BITMAPINFOHEADER bihOut;
    memset(&bihOut, 0, sizeof(bihOut));
    bihOut.biSize = sizeof(bihOut);
    bihOut.biWidth = bih.biWidth;
    bihOut.biHeight = bih.biHeight;
    bihOut.biPlanes = fmts[iPosition/2].biPlanes;
    bihOut.biBitCount = fmts[iPosition/2].biBitCount;
    bihOut.biCompression = fmts[iPosition/2].biCompression;
    bihOut.biSizeImage = bih.biWidth*bih.biHeight*bihOut.biBitCount>>3;

    if(iPosition&1)
    {
        pmt->formattype = FORMAT_VideoInfo;
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
        memset(vih, 0, sizeof(VIDEOINFOHEADER));
        vih->bmiHeader = bihOut;

        if(m_pInput->CurrentMediaType().formattype == FORMAT_VideoInfo2)
        {
            vih->bmiHeader.biWidth = ((VIDEOINFOHEADER2*)m_pInput->CurrentMediaType().Format())->dwPictAspectRatioX;
            vih->bmiHeader.biHeight = ((VIDEOINFOHEADER2*)m_pInput->CurrentMediaType().Format())->dwPictAspectRatioY;
            vih->bmiHeader.biSizeImage = vih->bmiHeader.biWidth*vih->bmiHeader.biHeight*vih->bmiHeader.biBitCount>>3;
        }
    }
    else
    {
        pmt->formattype = FORMAT_VideoInfo2;
        VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
        memset(vih2, 0, sizeof(VIDEOINFOHEADER2));
        vih2->bmiHeader = bihOut;
        vih2->dwPictAspectRatioX = ((VIDEOINFOHEADER2*)m_pInput->CurrentMediaType().Format())->dwPictAspectRatioX;
        vih2->dwPictAspectRatioY = ((VIDEOINFOHEADER2*)m_pInput->CurrentMediaType().Format())->dwPictAspectRatioY;
    }

    CorrectMediaType(pmt);

    return S_OK;
}

HRESULT CDiracVideoDecoder::StartStreaming()
{
    InitDecoder();
    return __super::StartStreaming();
}

HRESULT CDiracVideoDecoder::StopStreaming()
{
    FreeDecoder();
    return __super::StopStreaming();
}

HRESULT CDiracVideoDecoder::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    CAutoLock cAutoLock(&m_csReceive);

    m_fDropFrames = false;
    m_tStart = tStart;

    return __super::NewSegment(tStart, tStop, dRate);
}

HRESULT CDiracVideoDecoder::AlterQuality(Quality q)
{
    if(q.Late > 500*10000i64) m_fDropFrames = true;
    if(q.Late <= 0) m_fDropFrames = false;
    return E_NOTIMPL;
}
