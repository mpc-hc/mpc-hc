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

#ifndef OUTPUT_PIN_H
#define OUTPUT_PIN_H

#include "List.h"
#include "File.h"

class CRARFileSource;
class CRFSFile;
class CRFSFilePart;

class ReadRequest : public CRFSNode<ReadRequest>
{
public:
	~ReadRequest (void) {  }

	DWORD_PTR dwUser;
	IMediaSample *pSample;
    CRFSFile::ReadThread *threadObj;
    DWORD threadID;
    HANDLE threadHandle;
};

class CRFSOutputPin :
	public CBasePin,
	public IAsyncReader
{
public:
	CRFSOutputPin (CRARFileSource *pFilter, CCritSec *pLock, HRESULT *phr);
	~CRFSOutputPin (void);

	DECLARE_IUNKNOWN;

	// Reveals IAsyncReader
	STDMETHODIMP NonDelegatingQueryInterface (REFIID riid, void **ppv);

	// IPin interface
	STDMETHODIMP Connect (IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);

	// CBasePin
	HRESULT GetMediaType (int iPosition, CMediaType *pMediaType);
	HRESULT CheckMediaType (const CMediaType* pType);
	HRESULT CheckConnect (IPin *pPin);
	HRESULT CompleteConnect (IPin *pReceivePin);
	HRESULT BreakConnect ();

	// IAsyncReader interface
	STDMETHODIMP RequestAllocator (IMemAllocator *pPreferred, ALLOCATOR_PROPERTIES *pProps, IMemAllocator **ppActual);

	STDMETHODIMP Request (IMediaSample* pSample, DWORD_PTR dwUser);
	STDMETHODIMP WaitForNext (DWORD dwTimeout, IMediaSample **ppSample, DWORD_PTR *pdwUser);

	STDMETHODIMP SyncReadAligned (IMediaSample *pSample);
	STDMETHODIMP SyncRead (LONGLONG llPosition, LONG lLength, BYTE *pBuffer);

	STDMETHODIMP Length (LONGLONG *pTotal, LONGLONG *pAvailable);

	STDMETHODIMP BeginFlush (void);
	STDMETHODIMP EndFlush (void);

	void SetFile (CRFSFile *file) { m_file = file; }

private:
	DWORD m_align;
	BOOL m_asked_for_reader;
	CRFSFile *m_file;
	BOOL m_flush;
	HANDLE m_event;

	CRFSList<ReadRequest> m_requests;
	CCritSec m_lock;

	HRESULT ConvertSample (IMediaSample *sample, LONGLONG *pos, DWORD *length, BYTE **buffer);
    HRESULT DoFlush(DWORD dwTimeout, IMediaSample** ppSample, DWORD_PTR* pdwUser);

	BOOL IsAligned (INT_PTR l) { return !(l & (m_align - 1)); }
};

#endif // OUTPUT_PIN_H
