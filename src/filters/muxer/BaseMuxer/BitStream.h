/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

#pragma once

interface __declspec(uuid("30AB78C7-5259-4594-AEFE-9C0FC2F08A5E"))
    IBitStream :
    public IUnknown
{
    STDMETHOD_(UINT64, GetPos)() PURE;
    STDMETHOD_(UINT64, Seek)(UINT64 pos) PURE;  // it's a _stream_, please don't seek if you don't have to
    STDMETHOD(ByteWrite)(const void* pData, int len) PURE;
    STDMETHOD(BitWrite)(UINT64 data, int len) PURE;
    STDMETHOD(BitFlush)() PURE;
    STDMETHOD(StrWrite)(LPCSTR pData, BOOL bFixNewLine) PURE;
};

class CBitStream : public CUnknown, public IBitStream
{
    CComPtr<IStream> m_pStream;
    bool m_fThrowError;
    UINT64 m_bitbuff;
    int m_bitlen;

public:
    CBitStream(IStream* pStream, bool m_fThrowError = false);
    virtual ~CBitStream();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IBitStream

    STDMETHODIMP_(UINT64) GetPos();
    STDMETHODIMP_(UINT64) Seek(UINT64 pos);
    STDMETHODIMP ByteWrite(const void* pData, int len);
    STDMETHODIMP BitWrite(UINT64 data, int len);
    STDMETHODIMP BitFlush();
    STDMETHODIMP StrWrite(LPCSTR pData, BOOL bFixNewLine);
};
