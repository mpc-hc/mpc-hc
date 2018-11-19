/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2015-2017 see Authors.txt
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
#include "StreamDriveThru.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] = {
    {const_cast<LPWSTR>(L"Input"), FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesIn), sudPinTypesIn},
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CStreamDriveThruFilter), StreamDriveThruName, MERIT_DO_NOT_USE, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CStreamDriveThruFilter>, nullptr, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CStreamDriveThruFilter
//

CStreamDriveThruFilter::CStreamDriveThruFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CBaseFilter(NAME("CStreamDriveThruFilter"), pUnk, &m_csLock, __uuidof(this))
    , m_position(0)
{
    if (phr) {
        *phr = S_OK;
    }

    m_pInput = DEBUG_NEW CStreamDriveThruInputPin(NAME("CStreamDriveThruInputPin"), this, &m_csLock, phr);
    m_pOutput = DEBUG_NEW CStreamDriveThruOutputPin(NAME("CStreamDriveThruOutputPin"), this, &m_csLock, phr);

    CAMThread::Create();
}

CStreamDriveThruFilter::~CStreamDriveThruFilter()
{
    CAutoLock csAutoLock(&m_csLock);

    CAMThread::CallWorker(CMD_EXIT);
    CAMThread::Close();

    delete m_pInput;
    delete m_pOutput;
}

STDMETHODIMP CStreamDriveThruFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IMediaSeeking)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

#define PACKETSIZE 65536

#pragma warning(push)
#pragma warning(disable: 4702)
DWORD CStreamDriveThruFilter::ThreadProc()
{
    for (;;) {
        DWORD cmd = GetRequest();

        switch (cmd) {
            default:
            case CMD_EXIT:
                Reply(S_OK);
                return 0;
            case CMD_STOP:
                Reply(S_OK);
                break;
            case CMD_PAUSE:
                Reply(S_OK);
                break;
            case CMD_RUN:
                Reply(S_OK);

                do {
                    CComPtr<IAsyncReader> pAsyncReader;
                    CComPtr<IStream> pStream;

                    if (!m_pInput || !m_pInput->IsConnected() || FAILED(m_pInput->GetAsyncReader(&pAsyncReader))
                            || !m_pOutput || !m_pOutput->IsConnected() || FAILED(m_pOutput->GetStream(&pStream))) {
                        break;
                    }

                    LARGE_INTEGER li = {0};
                    ULARGE_INTEGER uli = {0};

                    if (FAILED(pStream->Seek(li, STREAM_SEEK_SET, nullptr))
                            || FAILED(pStream->SetSize(uli))) {
                        break;
                    }

                    if (CComQIPtr<IFileSinkFilter2> pFSF = GetFilterFromPin(m_pOutput->GetConnected())) {
                        pFSF->SetMode(AM_FILE_OVERWRITE);

                        CComHeapPtr<OLECHAR> pfn;
                        if (SUCCEEDED(pFSF->GetCurFile(&pfn, nullptr))) {
                            pFSF->SetFileName(pfn, nullptr);
                        }
                    }

                    m_position = 0;
                    BYTE buff[PACKETSIZE];

                    do {
                        while (!CheckRequest(&cmd)) {
                            CAutoLock csAutoLock(&m_csLock);

                            LONGLONG total = 0, available = 0;
                            if (FAILED(pAsyncReader->Length(&total, &available)) || m_position >= total) {
                                cmd = CMD_STOP;
                                break;
                            }

                            LONG size = std::min<LONG>(PACKETSIZE, LONG(total - m_position));
                            if (FAILED(pAsyncReader->SyncRead(m_position, size, buff))) {
                                cmd = CMD_STOP;
                                break;
                            }

                            ULONG written = 0;
                            if (FAILED(pStream->Write(buff, (ULONG)size, &written)) || (ULONG)size != written) {
                                cmd = CMD_STOP;
                                break;
                            }

                            m_position += size;
                        }

                        if (cmd == CMD_PAUSE) {
                            Reply(S_OK); // reply to CMD_PAUSE

                            while (!CheckRequest(&cmd)) {
                                Sleep(50);
                            }

                            Reply(S_OK); // reply to something
                        }
                    } while (cmd == CMD_RUN);

                    uli.QuadPart = m_position;
                    pStream->SetSize(uli);

                    if (CComPtr<IPin> pPin = m_pOutput->GetConnected()) {
                        pPin->EndOfStream();
                    }
                } while (false);

                break;
        }
    }
    UNREACHABLE_CODE(); // we should only exit via CMD_EXIT
#pragma warning(pop)
}

int CStreamDriveThruFilter::GetPinCount()
{
    return 2;
}

CBasePin* CStreamDriveThruFilter::GetPin(int n)
{
    CAutoLock csAutoLock(&m_csLock);

    if (n == 0) {
        return m_pInput;
    } else if (n == 1) {
        return m_pOutput;
    }

    return nullptr;
}

STDMETHODIMP CStreamDriveThruFilter::Stop()
{
    HRESULT hr;

    if (FAILED(hr = __super::Stop())) {
        return hr;
    }

    CallWorker(CMD_STOP);

    return S_OK;
}

STDMETHODIMP CStreamDriveThruFilter::Pause()
{
    HRESULT hr;

    if (FAILED(hr = __super::Pause())) {
        return hr;
    }

    CallWorker(CMD_PAUSE);

    return S_OK;
}

STDMETHODIMP CStreamDriveThruFilter::Run(REFERENCE_TIME tStart)
{
    HRESULT hr;

    if (FAILED(hr = __super::Run(tStart))) {
        return hr;
    }

    CallWorker(CMD_RUN);

    return S_OK;
}

// IMediaSeeking

STDMETHODIMP CStreamDriveThruFilter::GetCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);

    *pCapabilities = AM_SEEKING_CanGetCurrentPos | AM_SEEKING_CanGetStopPos | AM_SEEKING_CanGetDuration;

    return S_OK;
}

STDMETHODIMP CStreamDriveThruFilter::CheckCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);

    if (*pCapabilities == 0) {
        return S_OK;
    }

    DWORD caps;
    GetCapabilities(&caps);

    DWORD caps2 = caps & *pCapabilities;

    return caps2 == 0 ? E_FAIL : caps2 == *pCapabilities ? S_OK : S_FALSE;
}

STDMETHODIMP CStreamDriveThruFilter::IsFormatSupported(const GUID* pFormat)
{
    return !pFormat ? E_POINTER : *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

STDMETHODIMP CStreamDriveThruFilter::QueryPreferredFormat(GUID* pFormat)
{
    return GetTimeFormat(pFormat);
}

STDMETHODIMP CStreamDriveThruFilter::GetTimeFormat(GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);

    *pFormat = TIME_FORMAT_MEDIA_TIME;

    return S_OK;
}

STDMETHODIMP CStreamDriveThruFilter::IsUsingTimeFormat(const GUID* pFormat)
{
    return IsFormatSupported(pFormat);
}

STDMETHODIMP CStreamDriveThruFilter::SetTimeFormat(const GUID* pFormat)
{
    return S_OK == IsFormatSupported(pFormat) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CStreamDriveThruFilter::GetDuration(LONGLONG* pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    CheckPointer(m_pInput, VFW_E_NOT_CONNECTED);

    if (CComQIPtr<IAsyncReader> pAsyncReader = m_pInput->GetConnected()) {
        LONGLONG total, available;
        if (SUCCEEDED(pAsyncReader->Length(&total, &available))) {
            *pDuration = total;
            return S_OK;
        }
    }

    return E_NOINTERFACE;
}

STDMETHODIMP CStreamDriveThruFilter::GetStopPosition(LONGLONG* pStop)
{
    return GetDuration(pStop);
}

STDMETHODIMP CStreamDriveThruFilter::GetCurrentPosition(LONGLONG* pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);

    *pCurrent = m_position;

    return S_OK;
}

STDMETHODIMP CStreamDriveThruFilter::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamDriveThruFilter::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamDriveThruFilter::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamDriveThruFilter::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamDriveThruFilter::SetRate(double dRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamDriveThruFilter::GetRate(double* pdRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CStreamDriveThruFilter::GetPreroll(LONGLONG* pllPreroll)
{
    CheckPointer(pllPreroll, E_POINTER);

    *pllPreroll = 0;

    return S_OK;
}

//
// CStreamDriveThruInputPin
//

CStreamDriveThruInputPin::CStreamDriveThruInputPin(LPCTSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
    : CBasePin(pName, pFilter, pLock, phr, L"Input", PINDIR_INPUT)
{
}

CStreamDriveThruInputPin::~CStreamDriveThruInputPin()
{
}

HRESULT CStreamDriveThruInputPin::GetAsyncReader(IAsyncReader** ppAsyncReader)
{
    CheckPointer(ppAsyncReader, E_POINTER);

    *ppAsyncReader = nullptr;

    CheckPointer(m_pAsyncReader, VFW_E_NOT_CONNECTED);

    (*ppAsyncReader = m_pAsyncReader)->AddRef();

    return S_OK;
}

STDMETHODIMP CStreamDriveThruInputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CStreamDriveThruInputPin::CheckMediaType(const CMediaType* pmt)
{
    return pmt->majortype == MEDIATYPE_Stream || pmt->majortype == MEDIATYPE_NULL
           ? S_OK
           : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CStreamDriveThruInputPin::CheckConnect(IPin* pPin)
{
    HRESULT hr;

    if (FAILED(hr = __super::CheckConnect(pPin))) {
        return hr;
    }

    if (!CComQIPtr<IAsyncReader>(pPin)) {
        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT CStreamDriveThruInputPin::BreakConnect()
{
    HRESULT hr;

    if (FAILED(hr = __super::BreakConnect())) {
        return hr;
    }

    m_pAsyncReader.Release();

    return S_OK;
}

HRESULT CStreamDriveThruInputPin::CompleteConnect(IPin* pPin)
{
    HRESULT hr;

    if (FAILED(hr = __super::CompleteConnect(pPin))) {
        return hr;
    }

    CheckPointer(pPin, E_POINTER);
    m_pAsyncReader = pPin;
    CheckPointer(m_pAsyncReader, E_NOINTERFACE);

    return S_OK;
}

STDMETHODIMP CStreamDriveThruInputPin::BeginFlush()
{
    return E_UNEXPECTED;
}

STDMETHODIMP CStreamDriveThruInputPin::EndFlush()
{
    return E_UNEXPECTED;
}

//
// CStreamDriveThruOutputPin
//

CStreamDriveThruOutputPin::CStreamDriveThruOutputPin(LPCTSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
    : CBaseOutputPin(pName, pFilter, pLock, phr, L"Output")
{
}

CStreamDriveThruOutputPin::~CStreamDriveThruOutputPin()
{
}

HRESULT CStreamDriveThruOutputPin::GetStream(IStream** ppStream)
{
    CheckPointer(ppStream, E_POINTER);

    *ppStream = nullptr;

    CheckPointer(m_pStream, VFW_E_NOT_CONNECTED);

    (*ppStream = m_pStream)->AddRef();

    return S_OK;
}

STDMETHODIMP CStreamDriveThruOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CStreamDriveThruOutputPin::DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);

    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = PACKETSIZE;

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

HRESULT CStreamDriveThruOutputPin::CheckMediaType(const CMediaType* pmt)
{
    return pmt->majortype == MEDIATYPE_Stream
           ? S_OK
           : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CStreamDriveThruOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pLock);

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    pmt->majortype = MEDIATYPE_Stream;
    pmt->subtype = GUID_NULL;
    pmt->formattype = GUID_NULL;
    pmt->SetSampleSize(PACKETSIZE);

    return S_OK;
}

HRESULT CStreamDriveThruOutputPin::CheckConnect(IPin* pPin)
{
    HRESULT hr;

    if (FAILED(hr = __super::CheckConnect(pPin))) {
        return hr;
    }

    if (!CComQIPtr<IStream>(pPin)) {
        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT CStreamDriveThruOutputPin::BreakConnect()
{
    HRESULT hr;

    if (FAILED(hr = __super::BreakConnect())) {
        return hr;
    }

    m_pStream.Release();

    return S_OK;
}

HRESULT CStreamDriveThruOutputPin::CompleteConnect(IPin* pPin)
{
    HRESULT hr;

    if (FAILED(hr = __super::CompleteConnect(pPin))) {
        return hr;
    }

    CheckPointer(pPin, E_POINTER);
    m_pStream = pPin;
    CheckPointer(m_pStream, E_NOINTERFACE);

    return S_OK;
}

STDMETHODIMP CStreamDriveThruOutputPin::BeginFlush()
{
    return E_UNEXPECTED;
}

STDMETHODIMP CStreamDriveThruOutputPin::EndFlush()
{
    return E_UNEXPECTED;
}

STDMETHODIMP CStreamDriveThruOutputPin::Notify(IBaseFilter* pSender, Quality q)
{
    return E_NOTIMPL;
}
