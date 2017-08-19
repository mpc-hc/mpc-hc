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
#include "../../../DeCSS/VobFile.h"
#include "VTSReader.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG2_PROGRAM},
};

const AMOVIESETUP_PIN sudOpPin[] = {
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CVTSReader), VTSReaderName, MERIT_NORMAL, _countof(sudOpPin), sudOpPin, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CVTSReader>, nullptr, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{773EAEDE-D5EE-4fce-9C8F-C4F53D0A2F73}"),
        _T("0"), _T("0,12,,445644564944454F2D565453")); // "DVDVIDEO-VTS"

    SetRegKeyValue(
        _T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{773EAEDE-D5EE-4fce-9C8F-C4F53D0A2F73}"),
        _T("Source Filter"), _T("{773EAEDE-D5EE-4fce-9C8F-C4F53D0A2F73}"));

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    DeleteRegKey(_T("Media Type\\{e436eb83-524f-11ce-9f53-0020af0ba770}"), _T("{773EAEDE-D5EE-4fce-9C8F-C4F53D0A2F73}"));

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CVTSReader
//

CVTSReader::CVTSReader(IUnknown* pUnk, HRESULT* phr)
    : CAsyncReader(NAME("CVTSReader"), pUnk, &m_stream, phr, __uuidof(this))
{
    if (phr) {
        *phr = S_OK;
    }
}

CVTSReader::~CVTSReader()
{
}

STDMETHODIMP CVTSReader::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFileSourceFilter)
        QI(ITrackInfo)
        QI(IDSMChapterBag)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CVTSReader::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

    wcscpy_s(pInfo->achName, VTSReaderName);
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

// IFileSourceFilter

STDMETHODIMP CVTSReader::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
    if (!m_stream.Load(pszFileName)) {
        return E_FAIL;
    }

    ChapRemoveAll();
    for (int i = 0; i < m_stream.GetChaptersCount(); i++) {
        CString chap;
        chap.Format(_T("Chapter %d"), i + 1);
        ChapAppend(m_stream.GetChapterOffset(i), chap);
    }


    m_fn = pszFileName;

    CMediaType mt;
    mt.majortype = MEDIATYPE_Stream;
    mt.subtype = MEDIASUBTYPE_MPEG2_PROGRAM;
    m_mt = mt;

    return S_OK;
}

STDMETHODIMP CVTSReader::GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt)
{
    CheckPointer(ppszFileName, E_POINTER);

    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_fn.GetLength() + 1) * sizeof(WCHAR));
    if (!(*ppszFileName)) {
        return E_OUTOFMEMORY;
    }

    wcscpy_s(*ppszFileName, m_fn.GetLength() + 1, m_fn);

    return S_OK;
}

// ITrackInfo

STDMETHODIMP_(UINT) CVTSReader::GetTrackCount()
{
    return 0; // Not implemented yet
}

STDMETHODIMP_(BOOL) CVTSReader::GetTrackInfo(UINT aTrackIdx, struct TrackElement* pStructureToFill)
{
    return FALSE; // Not implemented yet
}

STDMETHODIMP_(BOOL) CVTSReader::GetTrackExtendedInfo(UINT aTrackIdx, void* pStructureToFill)
{
    return FALSE; // Not implemented yet
}

STDMETHODIMP_(BSTR) CVTSReader::GetTrackName(UINT aTrackIdx)
{
    return m_stream.GetTrackName(aTrackIdx); // return stream's language
}

STDMETHODIMP_(BSTR) CVTSReader::GetTrackCodecID(UINT aTrackIdx)
{
    return nullptr; // Not implemented yet
}

STDMETHODIMP_(BSTR) CVTSReader::GetTrackCodecName(UINT aTrackIdx)
{
    return nullptr; // Not implemented yet
}

STDMETHODIMP_(BSTR) CVTSReader::GetTrackCodecInfoURL(UINT aTrackIdx)
{
    return nullptr; // Not implemented yet
}

STDMETHODIMP_(BSTR) CVTSReader::GetTrackCodecDownloadURL(UINT aTrackIdx)
{
    return nullptr; // Not implemented yet
}

// CVTSStream

CVTSStream::CVTSStream() : m_off(0)
{
    m_vob.Attach(DEBUG_NEW CVobFile());
}

CVTSStream::~CVTSStream()
{
}

bool CVTSStream::Load(const WCHAR* fnw)
{
    CAtlList<CString> sl;
    return (m_vob && m_vob->Open(CString(fnw), sl) /*&& m_vob->IsDVD()*/);
}

HRESULT CVTSStream::SetPointer(LONGLONG llPos)
{
    m_off = (int)(llPos & 2047);
    int lba = (int)(llPos / 2048);

    return lba == m_vob->Seek(lba) ? S_OK : S_FALSE;
}

HRESULT CVTSStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
    CAutoLock lck(&m_csLock);

    DWORD len = dwBytesToRead;
    BYTE* ptr = pbBuffer;

    while (len > 0) {
        BYTE buff[2048];
        if (!m_vob->Read(buff)) {
            break;
        }

        int size = std::min(2048 - m_off, (int)std::min(len, 2048ul));

        memcpy(ptr, &buff[m_off], size);

        m_off = (m_off + size) & 2047;

        if (m_off > 0) {
            m_vob->Seek(m_vob->GetPosition() - 1);
        }

        ptr += size;
        len -= size;
    }

    if (pdwBytesRead) {
        *pdwBytesRead = DWORD(ptr - pbBuffer);
    }

    return S_OK;
}

LONGLONG CVTSStream::Size(LONGLONG* pSizeAvailable)
{
    LONGLONG len = 2048i64 * m_vob->GetLength();
    if (pSizeAvailable) {
        *pSizeAvailable = len;
    }
    return len;
}

DWORD CVTSStream::Alignment()
{
    return 1;
}

void CVTSStream::Lock()
{
    m_csLock.Lock();
}

void CVTSStream::Unlock()
{
    m_csLock.Unlock();
}

BSTR CVTSStream::GetTrackName(UINT aTrackIdx)
{
    return m_vob->GetTrackName(aTrackIdx);
}

int CVTSStream::GetChaptersCount()
{
    return m_vob->GetChaptersCount();
}

REFERENCE_TIME CVTSStream::GetChapterOffset(UINT ChapterNumber)
{
    return m_vob->GetChapterOffset(ChapterNumber);
}
