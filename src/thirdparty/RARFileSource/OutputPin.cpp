/*
 * Copyright (C) 2008-2012, OctaneSnail <os@v12pwr.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <windows.h>
#include <streams.h>

#include "OutputPin.h"
#include "RFS.h"
#include "Utils.h"
#include "Anchor.h"
#include "File.h"


CRFSOutputPin::CRFSOutputPin (CRARFileSource *pFilter, CCritSec *pLock, HRESULT *phr) :
	CBasePin (L"RAR File Source Output Pin", pFilter, pLock, phr, L"Output", PINDIR_OUTPUT)
{
	m_align = 1;
	m_asked_for_reader = FALSE;
	m_file = NULL;
	m_flush = FALSE;

	if (!(m_event = CreateEvent (NULL, FALSE, FALSE, NULL)))
	{
		ErrorMsg (GetLastError (), L"CRFSOutputPin::CRFSOutputPin - CreateEvent");

		m_event = INVALID_HANDLE_VALUE;

		if (phr)
			*phr = S_FALSE;
	}
}

CRFSOutputPin::~CRFSOutputPin ()
{
	CloseHandle (m_event);
}

STDMETHODIMP CRFSOutputPin::NonDelegatingQueryInterface (REFIID riid, void **ppv)
{
	if (riid == IID_IAsyncReader)
	{
		m_asked_for_reader = TRUE;
		return GetInterface ((IAsyncReader*) this, ppv);
	}
	else
		return CBasePin::NonDelegatingQueryInterface (riid, ppv);
}

STDMETHODIMP CRFSOutputPin::Connect (IPin * pReceivePin, const AM_MEDIA_TYPE *pmt)
{
	return CBasePin::Connect (pReceivePin, pmt);
}

HRESULT CRFSOutputPin::GetMediaType (int iPosition, CMediaType *pMediaType)
{
	if (!pMediaType)
		return E_POINTER;

	if (!m_file)
		return E_UNEXPECTED;

	if (iPosition < 0)
		return E_INVALIDARG;

	if (iPosition > 1)
		return VFW_S_NO_MORE_ITEMS;

	if (iPosition == 0)
		*pMediaType = m_file->media_type;
	else
		*pMediaType = &MEDIASUBTYPE_NULL;

	return S_OK;
}


HRESULT CRFSOutputPin::CheckMediaType (const CMediaType* pType)
{
	if (!m_file)
		return E_UNEXPECTED;

	// Treat MEDIASUBTYPE_NULL subtype as a wild card.
	if ((m_file->media_type.majortype == pType->majortype) &&
		(m_file->media_type.subtype == MEDIASUBTYPE_NULL || m_file->media_type.subtype == pType->subtype))
	{
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CRFSOutputPin::CheckConnect (IPin *pPin)
{
	m_asked_for_reader = FALSE;
	return CBasePin::CheckConnect (pPin);
}

HRESULT CRFSOutputPin::CompleteConnect (IPin *pReceivePin)
{
	if (m_asked_for_reader)
		return CBasePin::CompleteConnect (pReceivePin);

	return VFW_E_NO_TRANSPORT;
}

HRESULT CRFSOutputPin::BreakConnect ()
{
	m_asked_for_reader = FALSE;
	return CBasePin::BreakConnect ();
}

STDMETHODIMP CRFSOutputPin::RequestAllocator (IMemAllocator *pPreferred, ALLOCATOR_PROPERTIES *pProps, IMemAllocator **ppActual)
{
	if (!(pPreferred && pProps && ppActual))
		return E_POINTER;

	ALLOCATOR_PROPERTIES actual;
	HRESULT hr;

	DbgLog((LOG_TRACE, 2, L"Requested alignment = %ld", pProps->cbAlign));
	if (pProps->cbAlign)
		m_align = pProps->cbAlign;
	else
		pProps->cbAlign = m_align;

	if (pPreferred)
	{
		hr = pPreferred->SetProperties (pProps, &actual);

		if (SUCCEEDED (hr) && IsAligned (actual.cbAlign))
		{
			DbgLog((LOG_TRACE, 2, L"Using preferred allocator."));
			pPreferred->AddRef ();
			*ppActual = pPreferred;
			return S_OK;
		}
	}

	CMemAllocator *pMemObject = new CMemAllocator (L"RFS memory allocator", NULL, &hr);

	if (!pMemObject)
		return E_OUTOFMEMORY;

	if (FAILED (hr))
	{
		delete pMemObject;
		return hr;
	}

	IMemAllocator* pAlloc;

	hr = pMemObject->QueryInterface (IID_IMemAllocator, (void **) &pAlloc);

	if (FAILED (hr))
	{
		delete pMemObject;
		return E_NOINTERFACE;
	}

	hr = pAlloc->SetProperties (pProps, &actual);

	if (SUCCEEDED (hr) && IsAligned (actual.cbAlign))
	{
		DbgLog((LOG_TRACE, 2, L"Using our allocator."));
		*ppActual = pAlloc;
		return S_OK;
	}

	pAlloc->Release ();

	if (SUCCEEDED (hr))
		hr = VFW_E_BADALIGN;

	DbgLog((LOG_TRACE, 2, L"RequestAllocator failed."));
	return hr;
}

HRESULT CRFSOutputPin::ConvertSample (IMediaSample* sample, LONGLONG *pos, DWORD *length, BYTE **buffer)
{
	if (!(sample && pos && length && buffer))
		return E_POINTER;

	REFERENCE_TIME start, stop;

	HRESULT hr = sample->GetTime (&start, &stop);
	if (FAILED (hr))
		return hr;

	if (start < 0)
		return E_UNEXPECTED;

	LONGLONG llPos = start / UNITS;
	LONGLONG llLength = (stop - start) / UNITS;

	if (llLength < 0 || llLength > LONG_MAX)
		return E_UNEXPECTED;

	DWORD lLength = (LONG) llLength;
	LONGLONG llTotal = m_file->size;

	if (llPos > llTotal)
	{
		DbgLog((LOG_TRACE, 2, L"ConvertSample EOF pos = %lld total = %lld", llPos, llTotal));
		return ERROR_HANDLE_EOF;
	}

	if (llPos + lLength > llTotal)
	{
		llTotal = (llTotal + m_align - 1) & ~((LONGLONG) (m_align - 1));

		if (llPos + lLength > llTotal)
		{
			lLength = (LONG) (llTotal - llPos);

			stop = llTotal * UNITS;
			sample->SetTime (&start, &stop);
		}
	}

	BYTE* b;
	hr = sample->GetPointer (&b);
	if (FAILED (hr))
	{
		DbgLog((LOG_TRACE, 2, L"ConvertSample pSample->GetPointer failed"));
		return hr;
	}

	*pos = llPos;
	*length = lLength;
	*buffer = b;

	return S_OK;
}

STDMETHODIMP CRFSOutputPin::Request (IMediaSample* pSample, DWORD_PTR dwUser)
{
	LONGLONG llPosition;
	DWORD lLength;
	BYTE* pBuffer;

	if (m_flush)
	{
		DbgLog((LOG_TRACE, 2, L"Request called during flush."));
		return VFW_E_WRONG_STATE;
	}

	if (!m_file)
	{
		DbgLog((LOG_TRACE, 2, L"Request called with no file loaded."));
		return E_UNEXPECTED;
	}

	HRESULT hr = ConvertSample (pSample, &llPosition, &lLength, &pBuffer);

	if (FAILED (hr))
		return hr;

	if (!(IsAligned ((INT_PTR) llPosition) && IsAligned ((INT_PTR) lLength) && IsAligned ((INT_PTR) pBuffer)))
	{
		DbgLog((LOG_TRACE, 2, L"SyncReadAligned bad alignment. align = %lu, pos = %lld, len = %lu, buf = %p",
			m_align, llPosition, lLength, pBuffer));
        return VFW_E_BADALIGN;
	}

	LARGE_INTEGER offset;
	DWORD to_read, acc = 0;
	LONGLONG offset2;

	ReadRequest *request = new ReadRequest ();

	if (!request)
		return E_OUTOFMEMORY;

	Anchor<ReadRequest> arr (&request);

	request->dwUser = dwUser;
	request->pSample = pSample;

    CRFSFile::ReadThread *thread = new CRFSFile::ReadThread(m_file, llPosition, lLength, pBuffer);

    request->threadHandle = CreateThread(NULL, 0, CRFSFile::ReadThread::ThreadStartStatic, (void*)this, 0, &request->threadID);
	if (request->threadHandle != S_OK)
	{
		DWORD err = GetLastError ();
		ErrorMsg (err, L"CRFSOutputPin::Request - ReadFile");
		return S_FALSE;
	}

	m_lock.Lock ();

	arr.Release ();
	m_requests.InsertFirst (request);

	if (!SetEvent (m_event))
		ErrorMsg (GetLastError (), L"CRFSOutputPin::Request - SetEvent");

	m_lock.Unlock ();

	return S_OK;
}

HRESULT CRFSOutputPin::DoFlush (DWORD dwTimeout, IMediaSample **ppSample, DWORD_PTR *pdwUser)
{
	ReadRequest *rr;

	DbgLog((LOG_TRACE, 2, L"WaitForNext is flushing..."));

	m_lock.Lock ();
	rr = m_requests.UnlinkLast ();
	m_lock.Unlock ();

	if (!rr)
	{
		*ppSample = NULL;
		return VFW_E_TIMEOUT;
	}

	*pdwUser = rr->dwUser;
	*ppSample = rr->pSample;
    
    DWORD r;
    r = WaitForSingleObject(rr->threadHandle, dwTimeout);
    if (r == WAIT_TIMEOUT) {
        // Put it back into the list.
        m_lock.Lock();
        m_requests.InsertLast(rr);
        m_lock.Unlock();
        return VFW_E_TIMEOUT;
    }

	delete rr;

	return VFW_E_TIMEOUT;
}

STDMETHODIMP CRFSOutputPin::WaitForNext (DWORD dwTimeout, IMediaSample **ppSample, DWORD_PTR *pdwUser)
{
	HRESULT ret = S_OK;
	DWORD r;
	ReadRequest *rr;
    DWORD sTime = GetTickCount64(), curTime;

	if (!(ppSample && pdwUser))
		return E_POINTER;

	if (m_flush)
		return DoFlush (dwTimeout, ppSample, pdwUser);

	m_lock.Lock ();
	rr = m_requests.UnlinkLast ();
	m_lock.Unlock ();

	Anchor<ReadRequest> arr (&rr);

	while (!rr)
	{
		r = WaitForSingleObject (m_event, dwTimeout);
        curTime = GetTickCount64();
        dwTimeout -= (curTime - sTime);
        if (dwTimeout < 0) dwTimeout = 0;
        sTime = curTime;

		if (m_flush)
			return DoFlush (dwTimeout, ppSample, pdwUser);

		if (r == WAIT_TIMEOUT)
			return VFW_E_TIMEOUT;

		if (r == WAIT_FAILED)
		{
			ErrorMsg (GetLastError (), L"CRFSOutputPin::WaitForNext - WaitForSingleObject");
			return E_FAIL;
		}

		m_lock.Lock ();
		rr = m_requests.UnlinkLast ();
		m_lock.Unlock ();

		if (!rr)
			DbgLog((LOG_TRACE, 2, L"Got nothing?!?!"));
	}

	DWORD count, read, acc = 0;

	r = WaitForSingleObject (rr->threadHandle, dwTimeout);

	if (r == WAIT_TIMEOUT)
	{
		// Put it back into the list.
		m_lock.Lock ();
		arr.Release ();
		m_requests.InsertLast (rr);
		m_lock.Unlock ();
		return VFW_E_TIMEOUT;
	}

	*pdwUser = rr->dwUser;
	*ppSample = rr->pSample;

	if (r == WAIT_FAILED)
	{
		ErrorMsg (GetLastError (), L"CRFSOutputPin::WaitForNext - WaitForMultipleObjects");
		return E_FAIL;
	}

	read = 0;

	acc += rr->threadObj->read;

	// TODO: Try to recover if read != lLength
	if (read != rr->threadObj->lLength)
	{
		DbgLog((LOG_TRACE, 2, L"CRFSOutputPin::WaitForNext Got %lu expected %lu!", read, rr->threadObj->lLength));
		ret = S_FALSE;
	}

	rr->pSample->SetActualDataLength (acc);

	return ret;
}


STDMETHODIMP CRFSOutputPin::SyncReadAligned (IMediaSample* pSample)
{
	LONGLONG llPosition;
	DWORD lLength;
	BYTE* pBuffer;

	if (!m_file)
	{
		DbgLog((LOG_TRACE, 2, L"SyncReadAligned called with no file loaded."));
		return E_UNEXPECTED;
	}

	HRESULT hr = ConvertSample (pSample, &llPosition, &lLength, &pBuffer);

	if (FAILED (hr))
		return hr;

	if (!(IsAligned ((INT_PTR) llPosition) && IsAligned ((INT_PTR) lLength) && IsAligned ((INT_PTR) pBuffer)))
	{
		DbgLog((LOG_TRACE, 2, L"SyncReadAligned bad alignment. align = %lu, pos = %lld, len = %lu, buf = %p",
			m_align, llPosition, lLength, pBuffer));
        return VFW_E_BADALIGN;
	}

	LONG cbActual = 0;

	hr = m_file->SyncRead (llPosition, lLength, pBuffer, &cbActual);

	pSample->SetActualDataLength (cbActual);

	return hr;
}

STDMETHODIMP CRFSOutputPin::SyncRead (LONGLONG llPosition, LONG lLength, BYTE* pBuffer)
{
	if (!m_file)
	{
		DbgLog((LOG_TRACE, 2, L"SyncRead called with no file loaded."));
		return E_UNEXPECTED;
	}

	if (lLength < 0)
		return E_UNEXPECTED;

	return m_file->SyncRead (llPosition, lLength, pBuffer, NULL);
}

STDMETHODIMP CRFSOutputPin::Length (LONGLONG *pTotal, LONGLONG *pAvailable)
{
	if (!m_file)
		return E_UNEXPECTED;

	if (pTotal)
		*pTotal = m_file->size;

	if (pAvailable)
		*pAvailable = m_file->size;

	return S_OK;
}

STDMETHODIMP CRFSOutputPin::BeginFlush (void)
{
	DbgLog((LOG_TRACE, 2, L"CRFSOutputPin::BeginFlush"));
	m_flush = TRUE;
	SetEvent (m_event);
	return S_OK;
}

STDMETHODIMP CRFSOutputPin::EndFlush (void)
{
	DbgLog((LOG_TRACE, 2, L"CRFSOutputPin::EndFlush"));
	m_flush = FALSE;
	return S_OK;
}
