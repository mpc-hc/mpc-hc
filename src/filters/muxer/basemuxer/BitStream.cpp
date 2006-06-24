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
#include "BitStream.h"

//
// CBitStream
//

CBitStream::CBitStream(IStream* pStream, bool fThrowError)
	: CUnknown(_T("CBitStream"), NULL)
	, m_pStream(pStream)
	, m_fThrowError(fThrowError)
	, m_bitlen(0)
{
	ASSERT(m_pStream);

	LARGE_INTEGER li = {0};
	m_pStream->Seek(li, STREAM_SEEK_SET, NULL);

	ULARGE_INTEGER uli = {0};
	m_pStream->SetSize(uli); // not that it worked...

	m_pStream->Commit(S_OK); // also seems to have no effect, but maybe in the future...
}

CBitStream::~CBitStream()
{
	BitFlush();
}

STDMETHODIMP CBitStream::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	*ppv = NULL;

	return 
		QI(IBitStream)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// IBitStream

STDMETHODIMP_(UINT64) CBitStream::GetPos()
{
	ULARGE_INTEGER pos = {0, 0};
	m_pStream->Seek(*(LARGE_INTEGER*)&pos, STREAM_SEEK_CUR, &pos);
	return pos.QuadPart;
}

STDMETHODIMP_(UINT64) CBitStream::Seek(UINT64 pos)
{
	BitFlush();

	LARGE_INTEGER li;
	li.QuadPart = pos;
	ULARGE_INTEGER linew;
	linew.QuadPart = -1;
	m_pStream->Seek(li, STREAM_SEEK_SET, &linew);
	ASSERT(li.QuadPart == linew.QuadPart);
	return linew.QuadPart;
}

STDMETHODIMP CBitStream::ByteWrite(const void* pData, int len)
{
	HRESULT hr = S_OK;

	BitFlush();

	if(len > 0)
	{
		ULONG cbWritten = 0;
		hr = m_pStream->Write(pData, len, &cbWritten);

		ASSERT(SUCCEEDED(hr));
		if(m_fThrowError && FAILED(hr)) throw hr;
	}

	return hr;
}

STDMETHODIMP CBitStream::BitWrite(UINT64 data, int len)
{
	HRESULT hr = S_OK;

	ASSERT(len >= 0 && len <= 64);

	if(len > 56) {BitWrite(data >> 56, len - 56); len = 56;}

	m_bitbuff <<= len;
	m_bitbuff |= data & ((1ui64 << len) - 1);
	m_bitlen += len;
	
	while(m_bitlen >= 8)
	{
		BYTE b = (BYTE)(m_bitbuff >> (m_bitlen - 8));
		hr = m_pStream->Write(&b, 1, NULL);
		m_bitlen -= 8;

		ASSERT(SUCCEEDED(hr));
		if(m_fThrowError && FAILED(hr)) throw E_FAIL;
	}

	return hr;
}

STDMETHODIMP CBitStream::BitFlush()
{
	HRESULT hr = S_OK;

	if(m_bitlen > 0)
	{
		ASSERT(m_bitlen < 8);
		BYTE b = (BYTE)(m_bitbuff << (8 - m_bitlen));
		hr = m_pStream->Write(&b, 1, NULL);
		m_bitlen = 0;

		ASSERT(SUCCEEDED(hr));
		if(m_fThrowError && FAILED(hr)) throw E_FAIL;
	}

	return hr;
}

STDMETHODIMP CBitStream::StrWrite(LPCSTR pData, BOOL bFixNewLine)
{
	CStringA str = pData;

	if(bFixNewLine)
	{
		str.Replace("\r", "");
		str.Replace("\n", "\r\n");
	}

	return ByteWrite((LPCSTR)str, str.GetLength());
}
