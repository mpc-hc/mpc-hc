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

#pragma once

#include <atlbase.h>
#include "../asyncreader/asyncio.h"
#include "../asyncreader/asyncrdr.h"

class CUDPStream : public CAsyncStream, public CAMThread
{
private:
    CCritSec m_csLock;

	class packet_t
	{
	public:
		BYTE* m_buff; 
		__int64 m_start, m_end;
		packet_t(BYTE* p, __int64 start, __int64 end);
		virtual ~packet_t() {delete [] m_buff;}
	};

	int m_port;
	CString m_ip;
	SOCKET m_socket;
	GUID m_subtype;
	__int64 m_pos, m_len;
	bool m_drop;
	CAtlList<packet_t*> m_packets;

	void Clear();
	void Append(BYTE* buff, int len);

	enum {CMD_EXIT, CMD_RUN};
	DWORD ThreadProc();

public:
	CUDPStream();
	virtual ~CUDPStream();

	bool Load(const WCHAR* fnw);
	const GUID& GetSubType() {return m_subtype;}

    HRESULT SetPointer(LONGLONG llPos);
    HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead);
    LONGLONG Size(LONGLONG* pSizeAvailable);
    DWORD Alignment();
    void Lock();
	void Unlock();
};

[uuid("0E4221A9-9718-48D5-A5CF-4493DAD4A015")]
class CUDPReader 
	: public CAsyncReader
	, public IFileSourceFilter
{
	CUDPStream m_stream;
	CStringW m_fn;

public:
    CUDPReader(IUnknown* pUnk, HRESULT* phr);
	~CUDPReader();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IFileSourceFilter
	STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
	STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);
};
