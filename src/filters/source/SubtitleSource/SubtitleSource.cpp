/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2017 see Authors.txt
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
#include <algorithm>
#include "SubtitleSource.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER
#include <InitGuid.h>
#endif
#include "moreuuids.h"

static int _WIDTH = 640;
static int _HEIGHT = 480;
static int _ATPF = 400000;

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Subtitle, &MEDIASUBTYPE_NULL},
    {&MEDIATYPE_Text, &MEDIASUBTYPE_NULL},
    {&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32},
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut},
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CSubtitleSourceASCII), L"MPC-HC SubtitleSource (S_TEXT/ASCII)", MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CSubtitleSourceUTF8), L"MPC-HC SubtitleSource (S_TEXT/UTF8)", MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CSubtitleSourceSSA), L"MPC-HC SubtitleSource (S_TEXT/SSA)", MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CSubtitleSourceASS), L"MPC-HC SubtitleSource (S_TEXT/ASS)", MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CSubtitleSourceUSF), L"MPC-HC SubtitleSource (S_TEXT/USF)", MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CSubtitleSourcePreview), L"MPC-HC SubtitleSource (Preview)", MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CSubtitleSourceARGB), L"MPC-HC SubtitleSource (ARGB)", MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CSubtitleSourceASCII>, nullptr, &sudFilter[0]},
    {sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CSubtitleSourceUTF8>, nullptr, &sudFilter[1]},
    {sudFilter[2].strName, sudFilter[2].clsID, CreateInstance<CSubtitleSourceSSA>, nullptr, &sudFilter[2]},
    {sudFilter[3].strName, sudFilter[3].clsID, CreateInstance<CSubtitleSourceASS>, nullptr, &sudFilter[3]},
    //  {sudFilter[4].strName, sudFilter[4].clsID, CreateInstance<CSubtitleSourceUSF>, nullptr, &sudFilter[4]},
    {sudFilter[5].strName, sudFilter[5].clsID, CreateInstance<CSubtitleSourcePreview>, nullptr, &sudFilter[5]},
    {sudFilter[6].strName, sudFilter[6].clsID, CreateInstance<CSubtitleSourceARGB>, nullptr, &sudFilter[6]},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    /*CString clsid = CStringFromGUID(__uuidof(CSubtitleSourcePreview));

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".sub"),
        _T("Source Filter"), clsid);

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".srt"),
        _T("Source Filter"), clsid);

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".smi"),
        _T("Source Filter"), clsid);

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".ssa"),
        _T("Source Filter"), clsid);

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".ass"),
        _T("Source Filter"), clsid);

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".xss"),
        _T("Source Filter"), clsid);

    SetRegKeyValue(
        _T("Media Type\\Extensions"), _T(".usf"),
        _T("Source Filter"), clsid);*/
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".sub"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".srt"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".smi"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".ssa"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".ass"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".xss"));
    DeleteRegKey(_T("Media Type\\Extensions"), _T(".usf"));
    /**/
    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

class CSubtitleSourceApp : public CFilterApp
{
public:
    BOOL InitInstance() {
        if (!__super::InitInstance()) {
            return FALSE;
        }

        _WIDTH = GetProfileInt(_T("SubtitleSource"), _T("w"), 640);
        _HEIGHT = GetProfileInt(_T("SubtitleSource"), _T("h"), 480);
        _ATPF = GetProfileInt(_T("SubtitleSource"), _T("atpf"), 400000);
        if (_ATPF <= 0) {
            _ATPF = 400000;
        }
        WriteProfileInt(_T("SubtitleSource"), _T("w"), _WIDTH);
        WriteProfileInt(_T("SubtitleSource"), _T("h"), _HEIGHT);
        WriteProfileInt(_T("SubtitleSource"), _T("atpf"), _ATPF);

        return TRUE;
    }
};

CSubtitleSourceApp theApp;

#endif

//
// CSubtitleSource
//

CSubtitleSource::CSubtitleSource(LPUNKNOWN lpunk, HRESULT* phr, const CLSID& clsid)
    : CSource(NAME("CSubtitleSource"), lpunk, clsid)
{
}

CSubtitleSource::~CSubtitleSource()
{
}

STDMETHODIMP CSubtitleSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFileSourceFilter)
        QI(IAMFilterMiscFlags)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// IFileSourceFilter

STDMETHODIMP CSubtitleSource::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    if (GetPinCount() > 0) {
        return VFW_E_ALREADY_CONNECTED;
    }

    HRESULT hr = S_OK;
    if (!(DEBUG_NEW CSubtitleStream(pszFileName, this, &hr))) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        return hr;
    }

    m_fn = pszFileName;

    return S_OK;
}

STDMETHODIMP CSubtitleSource::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    CheckPointer(ppszFileName, E_POINTER);

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!*ppszFileName) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}

// IAMFilterMiscFlags

ULONG CSubtitleSource::GetMiscFlags()
{
    return AM_FILTER_MISC_FLAGS_IS_SOURCE;
}

STDMETHODIMP CSubtitleSource::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));
    wcscpy_s(pInfo->achName, SubtitleSourceName);
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

//
// CSubtitleStream
//

CSubtitleStream::CSubtitleStream(const WCHAR* wfn, CSubtitleSource* pParent, HRESULT* phr)
    : CSourceStream(NAME("SubtitleStream"), phr, pParent, L"Output")
    , CSourceSeeking(NAME("SubtitleStream"), (IPin*)this, phr, &m_cSharedState)
    , m_nPosition(0)
    , m_bDiscontinuity(FALSE)
    , m_bFlushing(FALSE)
    , m_rts(nullptr)
{
    CAutoLock cAutoLock(&m_cSharedState);

    CString fn(wfn);

    if (!m_rts.Open(fn, DEFAULT_CHARSET)) {
        *phr = E_FAIL;
        return;
    }

    m_rts.CreateDefaultStyle(DEFAULT_CHARSET);
    m_rts.ConvertToTimeBased(25);
    m_rts.Sort();

    m_rtDuration = 0;
    for (size_t i = 0, cnt = m_rts.GetCount(); i < cnt; i++) {
        m_rtDuration = std::max(m_rtDuration, 10000i64 * m_rts[i].end);
    }

    m_rtStop = m_rtDuration;

    *phr = m_rtDuration > 0 ? S_OK : E_FAIL;
}

CSubtitleStream::~CSubtitleStream()
{
    CAutoLock cAutoLock(&m_cSharedState);
}

STDMETHODIMP CSubtitleStream::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return (riid == IID_IMediaSeeking) ? CSourceSeeking::NonDelegatingQueryInterface(riid, ppv) //GetInterface((IMediaSeeking*)this, ppv)
           : CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

void CSubtitleStream::UpdateFromSeek()
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

HRESULT CSubtitleStream::SetRate(double dRate)
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

HRESULT CSubtitleStream::OnThreadStartPlay()
{
    m_bDiscontinuity = TRUE;
    return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}

HRESULT CSubtitleStream::ChangeStart()
{
    {
        CAutoLock lock(CSourceSeeking::m_pLock);

        OnThreadCreate();
        /*if (m_mt.majortype == MEDIATYPE_Video && m_mt.subtype == MEDIASUBTYPE_ARGB32)
        {
            m_nPosition = (int)(m_rtStart/10000)*1/1000;
        }
        else if (m_mt.majortype == MEDIATYPE_Video && m_mt.subtype == MEDIASUBTYPE_RGB32)
        {
            int m_nSegments = 0;
            if (!m_rts.SearchSubs((int)(m_rtStart/10000), 25, &m_nPosition, &m_nSegments))
                m_nPosition = m_nSegments;
        }
        else
        {
            m_nPosition = m_rts.SearchSub((int)(m_rtStart/10000), 25);
            if (m_nPosition < 0) m_nPosition = 0;
            else if (m_rts[m_nPosition].end <= (int)(m_rtStart/10000)) m_nPosition++;
        }*/
    }

    UpdateFromSeek();

    return S_OK;
}

HRESULT CSubtitleStream::ChangeStop()
{
    /*{
        CAutoLock lock(CSourceSeeking::m_pLock);
        if (m_rtPosition < m_rtStop)
            return S_OK;
    }*/
    // We're already past the new stop time -- better flush the graph.
    UpdateFromSeek();

    return S_OK;
}

HRESULT CSubtitleStream::OnThreadCreate()
{
    CAutoLock cAutoLockShared(&m_cSharedState);

    if (m_mt.majortype == MEDIATYPE_Video && m_mt.subtype == MEDIASUBTYPE_ARGB32) {
        m_nPosition = (int)(m_rtStart / _ATPF);
    } else if (m_mt.majortype == MEDIATYPE_Video && m_mt.subtype == MEDIASUBTYPE_RGB32) {
        int m_nSegments = 0;
        if (!m_rts.SearchSubs((int)(m_rtStart / 10000), 10000000.0 / _ATPF, &m_nPosition, &m_nSegments)) {
            m_nPosition = m_nSegments;
        }
    } else {
        m_nPosition = m_rts.SearchSub((int)(m_rtStart / 10000), 25);
        if (m_nPosition < 0) {
            m_nPosition = 0;
        } else if (m_rts[m_nPosition].end <= (int)(m_rtStart / 10000)) {
            m_nPosition++;
        }
    }

    return CSourceStream::OnThreadCreate();
}

HRESULT CSubtitleStream::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    //CAutoLock cAutoLock(m_pFilter->pStateLock());

    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

    if (m_mt.majortype == MEDIATYPE_Video) {
        pProperties->cBuffers = 2;
        pProperties->cbBuffer = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader.biSizeImage;
    } else {
        pProperties->cBuffers = 1;
        pProperties->cbBuffer = 0x10000;
    }

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

HRESULT CSubtitleStream::FillBuffer(IMediaSample* pSample)
{
    {
        CAutoLock cAutoLockShared(&m_cSharedState);

        BYTE* pData = nullptr;
        if (FAILED(pSample->GetPointer(&pData)) || !pData) {
            return S_FALSE;
        }

        AM_MEDIA_TYPE* pmt;
        if (SUCCEEDED(pSample->GetMediaType(&pmt)) && pmt) {
            CMediaType mt(*pmt);
            SetMediaType(&mt);
            DeleteMediaType(pmt);
        }

        int len = 0;
        REFERENCE_TIME rtStart, rtStop;

        if (m_mt.majortype == MEDIATYPE_Video && m_mt.subtype == MEDIASUBTYPE_ARGB32) {
            rtStart = (REFERENCE_TIME)((m_nPosition * _ATPF - m_rtStart) / m_dRateSeeking);
            rtStop = (REFERENCE_TIME)(((m_nPosition + 1) * _ATPF - m_rtStart) / m_dRateSeeking);
            if (m_rtStart + rtStart >= m_rtDuration) {
                return S_FALSE;
            }

            BITMAPINFOHEADER& bmi = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader;

            SubPicDesc spd;
            spd.w = _WIDTH;
            spd.h = _HEIGHT;
            spd.bpp = 32;
            spd.pitch = bmi.biWidth * 4;
            spd.bits = pData;

            len = spd.h * spd.pitch;

            for (int y = 0; y < spd.h; y++) {
                memsetd((DWORD*)(pData + spd.pitch * y), 0xff000000, spd.w * 4);
            }

            RECT bbox;
            m_rts.Render(spd, m_nPosition * _ATPF, 10000000.0 / _ATPF, bbox);

            for (int y = 0; y < spd.h; y++) {
                DWORD* p = (DWORD*)(pData + spd.pitch * y);
                for (int x = 0; x < spd.w; x++, p++) {
                    *p = (0xff000000 - (*p & 0xff000000)) | (*p & 0xffffff);
                }
            }
        } else if (m_mt.majortype == MEDIATYPE_Video && m_mt.subtype == MEDIASUBTYPE_RGB32) {
            const STSSegment* stss = m_rts.GetSegment(m_nPosition);
            if (!stss) {
                return S_FALSE;
            }

            BITMAPINFOHEADER& bmi = ((VIDEOINFOHEADER*)m_mt.pbFormat)->bmiHeader;

            SubPicDesc spd;
            spd.w = _WIDTH;
            spd.h = _HEIGHT;
            spd.bpp = 32;
            spd.pitch = bmi.biWidth * 4;
            spd.bits = pData;

            len = spd.h * spd.pitch;

            for (int y = 0; y < spd.h; y++) {
                DWORD c1 = 0xff606060, c2 = 0xffa0a0a0;
                if (y & 32) {
                    c1 ^= c2, c2 ^= c1, c1 ^= c2;
                }
                DWORD* p = (DWORD*)(pData + spd.pitch * y);
                for (int x = 0; x < spd.w; x += 32, p += 32) {
                    memsetd(p, (x & 32) ? c1 : c2, std::min(spd.w - x, 32) * 4);
                }
            }

            RECT bbox;
            m_rts.Render(spd, 10000i64 * (stss->start + stss->end) / 2, 10000000.0 / _ATPF, bbox);

            rtStart = (REFERENCE_TIME)((10000i64 * stss->start - m_rtStart) / m_dRateSeeking);
            rtStop = (REFERENCE_TIME)((10000i64 * stss->end - m_rtStart) / m_dRateSeeking);
        } else {
            if ((size_t)m_nPosition >= m_rts.GetCount()) {
                return S_FALSE;
            }

            STSEntry& stse = m_rts[m_nPosition];

            if (stse.start >= m_rtStop / 10000) {
                return S_FALSE;
            }

            if (m_mt.majortype == MEDIATYPE_Subtitle && m_mt.subtype == MEDIASUBTYPE_UTF8) {
                CStringA str = UTF16To8(m_rts.GetStrW(m_nPosition, false));
                memcpy((char*)pData, str, len = str.GetLength());
            } else if (m_mt.majortype == MEDIATYPE_Subtitle && (m_mt.subtype == MEDIASUBTYPE_SSA || m_mt.subtype == MEDIASUBTYPE_ASS)) {
                CStringW line;
                line.Format(L"%d,%d,%s,%s,%d,%d,%d,%s,%s",
                            stse.readorder, stse.layer, stse.style.GetString(), stse.actor.GetString(),
                            stse.marginRect.left, stse.marginRect.right, (stse.marginRect.top + stse.marginRect.bottom) / 2,
                            stse.effect.GetString(), m_rts.GetStrW(m_nPosition, true).GetString());

                CStringA str = UTF16To8(line);
                memcpy((char*)pData, str, len = str.GetLength());
            } else if (m_mt.majortype == MEDIATYPE_Text && m_mt.subtype == MEDIASUBTYPE_NULL) {
                CStringA str = m_rts.GetStrA(m_nPosition, false);
                memcpy((char*)pData, str, len = str.GetLength());
            } else {
                return S_FALSE;
            }

            rtStart = (REFERENCE_TIME)((10000i64 * stse.start - m_rtStart) / m_dRateSeeking);
            rtStop = (REFERENCE_TIME)((10000i64 * stse.end - m_rtStart) / m_dRateSeeking);
        }

        pSample->SetTime(&rtStart, &rtStop);
        pSample->SetActualDataLength(len);

        m_nPosition++;
    }

    pSample->SetSyncPoint(TRUE);

    if (m_bDiscontinuity) {
        pSample->SetDiscontinuity(TRUE);
        m_bDiscontinuity = FALSE;
    }

    return S_OK;
}

HRESULT CSubtitleStream::GetMediaType(CMediaType* pmt)
{
    return (static_cast<CSubtitleSource*>(m_pFilter))->GetMediaType(pmt);
}

HRESULT CSubtitleStream::CheckMediaType(const CMediaType* pmt)
{
    CAutoLock lock(m_pFilter->pStateLock());

    CMediaType mt;
    GetMediaType(&mt);

    if (mt.majortype == pmt->majortype && mt.subtype == pmt->subtype) {
        return NOERROR;
    }

    return E_FAIL;
}

STDMETHODIMP CSubtitleStream::Notify(IBaseFilter* pSender, Quality q)
{
    return E_NOTIMPL;
}

//
// CSubtitleSourceASCII
//

CSubtitleSourceASCII::CSubtitleSourceASCII(LPUNKNOWN lpunk, HRESULT* phr)
    : CSubtitleSource(lpunk, phr, __uuidof(this))
{
}

HRESULT CSubtitleSourceASCII::GetMediaType(CMediaType* pmt)
{
    CAutoLock cAutoLock(pStateLock());

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Text);
    pmt->SetSubtype(&MEDIASUBTYPE_NULL);
    pmt->SetFormatType(&FORMAT_None);
    pmt->ResetFormatBuffer();

    return NOERROR;
}

//
// CSubtitleSourceUTF8
//

CSubtitleSourceUTF8::CSubtitleSourceUTF8(LPUNKNOWN lpunk, HRESULT* phr)
    : CSubtitleSource(lpunk, phr, __uuidof(this))
{
}

HRESULT CSubtitleSourceUTF8::GetMediaType(CMediaType* pmt)
{
    CAutoLock cAutoLock(pStateLock());

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Subtitle);
    pmt->SetSubtype(&MEDIASUBTYPE_UTF8);
    pmt->SetFormatType(&FORMAT_SubtitleInfo);
    SUBTITLEINFO* psi = (SUBTITLEINFO*)pmt->AllocFormatBuffer(sizeof(SUBTITLEINFO));
    ZeroMemory(psi, pmt->FormatLength());
    strcpy_s(psi->IsoLang, "eng");

    return NOERROR;
}

//
// CSubtitleSourceSSA
//

CSubtitleSourceSSA::CSubtitleSourceSSA(LPUNKNOWN lpunk, HRESULT* phr)
    : CSubtitleSource(lpunk, phr, __uuidof(this))
{
}

HRESULT CSubtitleSourceSSA::GetMediaType(CMediaType* pmt)
{
    CAutoLock cAutoLock(pStateLock());

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Subtitle);
    pmt->SetSubtype(&MEDIASUBTYPE_SSA);
    pmt->SetFormatType(&FORMAT_SubtitleInfo);

    CSimpleTextSubtitle sts;
    sts.Open(CString(m_fn), DEFAULT_CHARSET);
    sts.RemoveAll();

    CFile f;
    TCHAR path[MAX_PATH], fn[MAX_PATH];
    if (!GetTempPath(MAX_PATH, path) || !GetTempFileName(path, _T("mpc_sts"), 0, fn)) {
        return E_FAIL;
    }

    _tremove(fn);

    _tcscat_s(fn, _T(".ssa"));

    if (!sts.SaveAs(fn, Subtitle::SSA, -1, CTextFile::UTF8) || !f.Open(fn, CFile::modeRead)) {
        return E_FAIL;
    }

    int len = (int)f.GetLength() - 3;
    f.Seek(3, CFile::begin);

    SUBTITLEINFO* psi = (SUBTITLEINFO*)pmt->AllocFormatBuffer(sizeof(SUBTITLEINFO) + len);
    ZeroMemory(psi, pmt->FormatLength());
    psi->dwOffset = sizeof(SUBTITLEINFO);
    strcpy_s(psi->IsoLang, "eng");
    f.Read(pmt->pbFormat + psi->dwOffset, len);
    f.Close();

    _tremove(fn);

    return NOERROR;
}

//
// CSubtitleSourceASS
//

CSubtitleSourceASS::CSubtitleSourceASS(LPUNKNOWN lpunk, HRESULT* phr)
    : CSubtitleSource(lpunk, phr, __uuidof(this))
{
}

HRESULT CSubtitleSourceASS::GetMediaType(CMediaType* pmt)
{
    CAutoLock cAutoLock(pStateLock());

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Subtitle);
    pmt->SetSubtype(&MEDIASUBTYPE_ASS);
    pmt->SetFormatType(&FORMAT_SubtitleInfo);

    CSimpleTextSubtitle sts;
    sts.Open(CString(m_fn), DEFAULT_CHARSET);
    sts.RemoveAll();

    CFile f;
    TCHAR path[MAX_PATH], fn[MAX_PATH];
    if (!GetTempPath(MAX_PATH, path) || !GetTempFileName(path, _T("mpc_sts"), 0, fn)) {
        return E_FAIL;
    }

    _tremove(fn);

    _tcscat_s(fn, _T(".ass"));

    if (!sts.SaveAs(fn, Subtitle::ASS, -1, CTextFile::UTF8) || !f.Open(fn, CFile::modeRead)) {
        return E_FAIL;
    }

    int len = (int)f.GetLength();

    SUBTITLEINFO* psi = (SUBTITLEINFO*)pmt->AllocFormatBuffer(sizeof(SUBTITLEINFO) + len);
    ZeroMemory(psi, pmt->FormatLength());
    psi->dwOffset = sizeof(SUBTITLEINFO);
    strcpy_s(psi->IsoLang, "eng");
    f.Read(pmt->pbFormat + psi->dwOffset, len);
    f.Close();

    _tremove(fn);

    return NOERROR;
}

//
// CSubtitleSourceUSF
//

CSubtitleSourceUSF::CSubtitleSourceUSF(LPUNKNOWN lpunk, HRESULT* phr)
    : CSubtitleSource(lpunk, phr, __uuidof(this))
{
}

HRESULT CSubtitleSourceUSF::GetMediaType(CMediaType* pmt)
{
    CAutoLock cAutoLock(pStateLock());

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Subtitle);
    pmt->SetSubtype(&MEDIASUBTYPE_USF);
    pmt->SetFormatType(&FORMAT_SubtitleInfo);
    SUBTITLEINFO* psi = (SUBTITLEINFO*)pmt->AllocFormatBuffer(sizeof(SUBTITLEINFO));
    ZeroMemory(psi, pmt->FormatLength());
    strcpy_s(psi->IsoLang, "eng");
    // TODO: ...

    return NOERROR;
}

//
// CSubtitleSourcePreview
//

CSubtitleSourcePreview::CSubtitleSourcePreview(LPUNKNOWN lpunk, HRESULT* phr)
    : CSubtitleSource(lpunk, phr, __uuidof(this))
{
}

HRESULT CSubtitleSourcePreview::GetMediaType(CMediaType* pmt)
{
    CAutoLock cAutoLock(pStateLock());

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
    ZeroMemory(pvih, pmt->FormatLength());
    pvih->bmiHeader.biSize = sizeof(pvih->bmiHeader);
    pvih->bmiHeader.biWidth = _WIDTH;
    pvih->bmiHeader.biHeight = _HEIGHT;
    pvih->bmiHeader.biBitCount = 32;
    pvih->bmiHeader.biCompression = BI_RGB;
    pvih->bmiHeader.biPlanes = 1;
    pvih->bmiHeader.biSizeImage = pvih->bmiHeader.biWidth * abs(pvih->bmiHeader.biHeight) * pvih->bmiHeader.biBitCount >> 3;

    return NOERROR;
}

//
// CSubtitleSourceARGB
//

CSubtitleSourceARGB::CSubtitleSourceARGB(LPUNKNOWN lpunk, HRESULT* phr)
    : CSubtitleSource(lpunk, phr, __uuidof(this))
{
}

HRESULT CSubtitleSourceARGB::GetMediaType(CMediaType* pmt)
{
    CAutoLock cAutoLock(pStateLock());

    pmt->InitMediaType();
    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetSubtype(&MEDIASUBTYPE_ARGB32);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
    ZeroMemory(pvih, pmt->FormatLength());
    pvih->bmiHeader.biSize = sizeof(pvih->bmiHeader);
    // TODO: read w,h,fps from a config file or registry
    pvih->bmiHeader.biWidth = _WIDTH;
    pvih->bmiHeader.biHeight = _HEIGHT;
    pvih->bmiHeader.biBitCount = 32;
    pvih->bmiHeader.biCompression = BI_RGB;
    pvih->bmiHeader.biPlanes = 1;
    pvih->bmiHeader.biSizeImage = pvih->bmiHeader.biWidth * abs(pvih->bmiHeader.biHeight) * pvih->bmiHeader.biBitCount >> 3;

    return NOERROR;
}
