/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#include "BitStream.h"
#include "BaseMuxerInputPin.h"
#include "BaseMuxerRelatedPin.h"

class CBaseMuxerOutputPin : public CBaseOutputPin
{
    CComPtr<IBitStream> m_pBitStream;

public:
    CBaseMuxerOutputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
    virtual ~CBaseMuxerOutputPin() {}

    IBitStream* GetBitStream();

    HRESULT BreakConnect();

    HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pProperties);

    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);

    HRESULT DeliverEndOfStream();

    STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
};

class CBaseMuxerRawOutputPin : public CBaseMuxerOutputPin, public CBaseMuxerRelatedPin
{
    struct idx_t {
        REFERENCE_TIME rt;
        __int64 fp;
    };
    CAtlList<idx_t> m_idx;

public:
    CBaseMuxerRawOutputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
    virtual ~CBaseMuxerRawOutputPin() {}

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    virtual void MuxHeader(const CMediaType& mt);
    virtual void MuxPacket(const CMediaType& mt, const MuxerPacket* pPacket);
    virtual void MuxFooter(const CMediaType& mt);
};
