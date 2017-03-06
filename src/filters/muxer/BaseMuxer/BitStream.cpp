/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "BitStream.h"

//
// CBitStream
//

CBitStream::CBitStream(IStream* pStream, bool fThrowError)
    : CUnknown(_T("CBitStream"), nullptr)
    , m_pStream(pStream)
    , m_fThrowError(fThrowError)
    , m_bitbuff(0)
    , m_bitlen(0)
{
    ASSERT(m_pStream);

    LARGE_INTEGER li = {0};
    m_pStream->Seek(li, STREAM_SEEK_SET, nullptr);

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

    *ppv = nullptr;

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
    linew.QuadPart = (ULONGLONG) - 1;
    m_pStream->Seek(li, STREAM_SEEK_SET, &linew);
    ASSERT(li.QuadPart == (LONGLONG)linew.QuadPart);
    return linew.QuadPart;
}

STDMETHODIMP CBitStream::ByteWrite(const void* pData, int len)
{
    HRESULT hr = S_OK;

    BitFlush();

    if (len > 0) {
        ULONG cbWritten = 0;
        hr = m_pStream->Write(pData, len, &cbWritten);

        ASSERT(SUCCEEDED(hr));
        if (m_fThrowError && FAILED(hr)) {
            throw hr;
        }
    }

    return hr;
}

STDMETHODIMP CBitStream::BitWrite(UINT64 data, int len)
{
    HRESULT hr = S_OK;

    ASSERT(len >= 0 && len <= 64);

    if (len > 56) {
        BitWrite(data >> 56, len - 56);
        len = 56;
    }

    m_bitbuff <<= len;
    m_bitbuff |= data & ((1ui64 << len) - 1);
    m_bitlen += len;

    while (m_bitlen >= 8) {
        BYTE b = (BYTE)(m_bitbuff >> (m_bitlen - 8));
        hr = m_pStream->Write(&b, 1, nullptr);
        m_bitlen -= 8;

        ASSERT(SUCCEEDED(hr));
        if (m_fThrowError && FAILED(hr)) {
            throw E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CBitStream::BitFlush()
{
    HRESULT hr = S_OK;

    if (m_bitlen > 0) {
        ASSERT(m_bitlen < 8);
        BYTE b = (BYTE)(m_bitbuff << (8 - m_bitlen));
        hr = m_pStream->Write(&b, 1, nullptr);
        m_bitlen = 0;

        ASSERT(SUCCEEDED(hr));
        if (m_fThrowError && FAILED(hr)) {
            throw E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CBitStream::StrWrite(LPCSTR pData, BOOL bFixNewLine)
{
    CStringA str = pData;

    if (bFixNewLine) {
        str.Replace("\r", "");
        str.Replace("\n", "\r\n");
    }

    return ByteWrite((LPCSTR)str, str.GetLength());
}
