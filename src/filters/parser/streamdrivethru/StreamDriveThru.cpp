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

#include "StdAfx.h"
#include "streamdrivethru.h"
#include "..\..\..\DSUtil\DSUtil.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CStreamDriveThruFilter), L"StreamDriveThru", MERIT_DO_NOT_USE, countof(sudpPins), sudpPins}
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CStreamDriveThruFilter>, NULL, &sudFilter[0]}
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif

//
// CStreamDriveThruFilter
//

CStreamDriveThruFilter::CStreamDriveThruFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseFilter(NAME("CStreamDriveThruFilter"), pUnk, &m_csLock, __uuidof(this))
	, m_position(0)
{
	if(phr) *phr = S_OK;

	m_pInput = new CStreamDriveThruInputPin(NAME("CStreamDriveThruInputPin"), this, &m_csLock, phr);
	m_pOutput = new CStreamDriveThruOutputPin(NAME("CStreamDriveThruOutputPin"), this, &m_csLock, phr);

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

DWORD CStreamDriveThruFilter::ThreadProc()
{
	while(1)
	{
		DWORD cmd = GetRequest();

		switch(cmd)
		{
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

			do
			{
				CComPtr<IAsyncReader> pAsyncReader;
				CComPtr<IStream> pStream;

				if(!m_pInput || !m_pInput->IsConnected() || FAILED(m_pInput->GetAsyncReader(&pAsyncReader))
				|| !m_pOutput || !m_pOutput->IsConnected() || FAILED(m_pOutput->GetStream(&pStream)))
					break;

				LARGE_INTEGER li = {0};
				ULARGE_INTEGER uli = {0};

				if(FAILED(pStream->Seek(li, STREAM_SEEK_SET, NULL))
				|| FAILED(pStream->SetSize(uli)))
					break;

				if(CComQIPtr<IFileSinkFilter2> pFSF = GetFilterFromPin(m_pOutput->GetConnected()))
				{
					pFSF->SetMode(AM_FILE_OVERWRITE);

					LPOLESTR pfn;
					if(SUCCEEDED(pFSF->GetCurFile(&pfn, NULL)))
					{
						pFSF->SetFileName(pfn, NULL);
						CoTaskMemFree(pfn);
					}
				}

				m_position = 0;
				BYTE buff[PACKETSIZE];

				do
				{
					while(!CheckRequest(&cmd))
					{
						CAutoLock csAutoLock(&m_csLock);

						LONGLONG total = 0, available = 0;
						if(FAILED(pAsyncReader->Length(&total, &available)) || m_position >= total)
						{
							cmd = CMD_STOP;
							break;
						}

						LONG size = (LONG)min(PACKETSIZE, total - m_position);
						if(FAILED(pAsyncReader->SyncRead(m_position, size, buff)))
						{
							cmd = CMD_STOP;
							break;
						}

						ULONG written = 0;
						if(FAILED(pStream->Write(buff, (ULONG)size, &written)) || (ULONG)size != written)
						{
							cmd = CMD_STOP;
							break;
						}

						m_position += size;
					}

					if(cmd == CMD_PAUSE)
					{
						Reply(S_OK); // reply to CMD_PAUSE

						while(!CheckRequest(&cmd))
							Sleep(50);

						Reply(S_OK); // reply to something
					}
				}
				while(cmd == CMD_RUN);

				uli.QuadPart = m_position;
				pStream->SetSize(uli);

				if(CComPtr<IPin> pPin = m_pOutput->GetConnected())
					pPin->EndOfStream();
			}
			while(false);

			break;
		}
	}

	return 0;
}

int CStreamDriveThruFilter::GetPinCount()
{
	return 2;
}

CBasePin* CStreamDriveThruFilter::GetPin(int n)
{
    CAutoLock csAutoLock(&m_csLock);

	if(n == 0) return m_pInput;
	else if(n == 1) return m_pOutput;

    return NULL;
}

STDMETHODIMP CStreamDriveThruFilter::Stop()
{
	HRESULT hr;
	
	if(FAILED(hr = __super::Stop()))
		return hr;

	CallWorker(CMD_STOP);

	return S_OK;
}

STDMETHODIMP CStreamDriveThruFilter::Pause()
{
	HRESULT hr;
	
	if(FAILED(hr = __super::Pause()))
		return hr;

	CallWorker(CMD_PAUSE);

	return S_OK;
}

STDMETHODIMP CStreamDriveThruFilter::Run(REFERENCE_TIME tStart)
{
	HRESULT hr;
	
	if(FAILED(hr = __super::Run(tStart)))
		return hr;

	CallWorker(CMD_RUN);

	return S_OK;
}

// IMediaSeeking

STDMETHODIMP CStreamDriveThruFilter::GetCapabilities(DWORD* pCapabilities)
{
	return pCapabilities ? *pCapabilities = AM_SEEKING_CanGetCurrentPos|AM_SEEKING_CanGetStopPos|AM_SEEKING_CanGetDuration, S_OK : E_POINTER;
}
STDMETHODIMP CStreamDriveThruFilter::CheckCapabilities(DWORD* pCapabilities)
{
	CheckPointer(pCapabilities, E_POINTER);

	if(*pCapabilities == 0) return S_OK;

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
	return pFormat ? *pFormat = TIME_FORMAT_MEDIA_TIME, S_OK : E_POINTER;
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

	if(CComQIPtr<IAsyncReader> pAsyncReader = m_pInput->GetConnected())
	{
		LONGLONG total, available;
		if(SUCCEEDED(pAsyncReader->Length(&total, &available)))
		{
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
	return pCurrent ? *pCurrent = m_position, S_OK : E_POINTER;
}
STDMETHODIMP CStreamDriveThruFilter::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat) {return E_NOTIMPL;}
STDMETHODIMP CStreamDriveThruFilter::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags) {return E_NOTIMPL;}
STDMETHODIMP CStreamDriveThruFilter::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop) {return E_NOTIMPL;}
STDMETHODIMP CStreamDriveThruFilter::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest) {return E_NOTIMPL;}
STDMETHODIMP CStreamDriveThruFilter::SetRate(double dRate) {return E_NOTIMPL;}
STDMETHODIMP CStreamDriveThruFilter::GetRate(double* pdRate) {return E_NOTIMPL;}
STDMETHODIMP CStreamDriveThruFilter::GetPreroll(LONGLONG* pllPreroll)
{
	return pllPreroll ? *pllPreroll = 0, S_OK : E_POINTER;
}

//
// CStreamDriveThruInputPin
//

CStreamDriveThruInputPin::CStreamDriveThruInputPin(TCHAR* pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
	: CBasePin(pName, pFilter, pLock, phr, L"Input", PINDIR_INPUT)
{
}

CStreamDriveThruInputPin::~CStreamDriveThruInputPin()
{
}

HRESULT CStreamDriveThruInputPin::GetAsyncReader(IAsyncReader** ppAsyncReader)
{
	CheckPointer(ppAsyncReader, E_POINTER);

	*ppAsyncReader = NULL;

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
	return pmt->majortype == MEDIATYPE_Stream
		? S_OK
		: E_INVALIDARG;
}

HRESULT CStreamDriveThruInputPin::CheckConnect(IPin* pPin)
{
	HRESULT hr;

	if(FAILED(hr = __super::CheckConnect(pPin)))
		return hr;

	if(!CComQIPtr<IAsyncReader>(pPin))
		return E_NOINTERFACE;

	return S_OK;
}

HRESULT CStreamDriveThruInputPin::BreakConnect()
{
	HRESULT hr;

	if(FAILED(hr = __super::BreakConnect()))
		return hr;

	m_pAsyncReader.Release();

	return S_OK;
}

HRESULT CStreamDriveThruInputPin::CompleteConnect(IPin* pPin)
{
	HRESULT hr;

	if(FAILED(hr = __super::CompleteConnect(pPin)))
		return hr;

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

CStreamDriveThruOutputPin::CStreamDriveThruOutputPin(TCHAR* pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
	: CBaseOutputPin(pName, pFilter, pLock, phr, L"Output")
{
}

CStreamDriveThruOutputPin::~CStreamDriveThruOutputPin()
{
}

HRESULT CStreamDriveThruOutputPin::GetStream(IStream** ppStream)
{
	CheckPointer(ppStream, E_POINTER);

	*ppStream = NULL;

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
    if(FAILED(hr = pAlloc->SetProperties(pProperties, &Actual))) return hr;

    if(Actual.cbBuffer < pProperties->cbBuffer) return E_FAIL;
    ASSERT(Actual.cBuffers == pProperties->cBuffers);

    return NOERROR;
}

HRESULT CStreamDriveThruOutputPin::CheckMediaType(const CMediaType* pmt)
{
	return pmt->majortype == MEDIATYPE_Stream
		? S_OK
		: E_INVALIDARG;
}

HRESULT CStreamDriveThruOutputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cAutoLock(m_pLock);

	if(iPosition < 0) return E_INVALIDARG;
	if(iPosition > 0) return VFW_S_NO_MORE_ITEMS;

	pmt->majortype = MEDIATYPE_Stream;
	pmt->subtype = GUID_NULL;
	pmt->formattype = GUID_NULL;
	pmt->SetSampleSize(PACKETSIZE);

	return S_OK;
}

HRESULT CStreamDriveThruOutputPin::CheckConnect(IPin* pPin)
{
	HRESULT hr;

	if(FAILED(hr = __super::CheckConnect(pPin)))
		return hr;

	if(!CComQIPtr<IStream>(pPin))
		return E_NOINTERFACE;

	return S_OK;
}

HRESULT CStreamDriveThruOutputPin::BreakConnect()
{
	HRESULT hr;

	if(FAILED(hr = __super::BreakConnect()))
		return hr;

	m_pStream.Release();

	return S_OK;
}

HRESULT CStreamDriveThruOutputPin::CompleteConnect(IPin* pPin)
{
	HRESULT hr;

	if(FAILED(hr = __super::CompleteConnect(pPin)))
		return hr;

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
