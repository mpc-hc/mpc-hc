/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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

#pragma once

#include <atlbase.h>
#include "../AsyncReader/asyncio.h"
#include "../AsyncReader/asyncrdr.h"

class CVobFile;

class CVTSStream : public CAsyncStream
{
private:
	CCritSec m_csLock;

	CAutoPtr<CVobFile> m_vob;
	int m_off;

public:
	CVTSStream();
	virtual ~CVTSStream();

	bool Load(const WCHAR* fnw);

	HRESULT SetPointer(LONGLONG llPos);
	HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead);
	LONGLONG Size(LONGLONG* pSizeAvailable);
	DWORD Alignment();
	void Lock();
	void Unlock();
};

class __declspec(uuid("773EAEDE-D5EE-4fce-9C8F-C4F53D0A2F73"))
	CVTSReader
	: public CAsyncReader
	, public IFileSourceFilter
{
	CVTSStream m_stream;
	CStringW m_fn;

public:
	CVTSReader(IUnknown* pUnk, HRESULT* phr);
	~CVTSReader();

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IFileSourceFilter
	STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
	STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);
};
