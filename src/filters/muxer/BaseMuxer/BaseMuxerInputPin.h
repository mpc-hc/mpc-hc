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

#include "BaseMuxerRelatedPin.h"
#include "../../../DSUtil/DSMPropertyBag.h"

class CBaseMuxerInputPin;

struct MuxerPacket {
    CBaseMuxerInputPin* pPin;
    REFERENCE_TIME rtStart, rtStop;
    CAtlArray<BYTE> pData;
    enum flag_t {
        empty = 0,
        timevalid = 1,
        syncpoint = 2,
        discontinuity = 4,
        eos = 8,
        bogus = 16
    };
    DWORD flags;
    int index;
    struct MuxerPacket(CBaseMuxerInputPin* pPin) {
        this->pPin = pPin;
        rtStart = rtStop = _I64_MIN;
        flags = empty;
        index = -1;
    }

    bool IsTimeValid() const { return !!(flags & timevalid); }
    bool IsSyncPoint() const { return !!(flags & syncpoint); }
    bool IsDiscontinuity() const { return !!(flags & discontinuity); }
    bool IsEOS() const { return !!(flags & eos); }
    bool IsBogus() const { return !!(flags & bogus); }
};

class CBaseMuxerInputPin : public CBaseInputPin, public CBaseMuxerRelatedPin, public IDSMPropertyBagImpl
{
private:
    int m_iID;

    CCritSec m_csReceive;
    REFERENCE_TIME m_rtMaxStart, m_rtDuration;
    bool m_fEOS;
    int m_iPacketIndex;

    CCritSec m_csQueue;
    CAutoPtrList<MuxerPacket> m_queue;
    void PushPacket(CAutoPtr<MuxerPacket> pPacket);
    CAutoPtr<MuxerPacket> PopPacket();
    CAMEvent m_evAcceptPacket;

    friend class CBaseMuxerFilter;

public:
    CBaseMuxerInputPin(LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr);
    virtual ~CBaseMuxerInputPin();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    REFERENCE_TIME GetDuration() { return m_rtDuration; }
    int GetID() { return m_iID; }
    CMediaType& CurrentMediaType() { return m_mt; }
    bool IsSubtitleStream();

    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin* pReceivePin);

    HRESULT Active();
    HRESULT Inactive();

    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    STDMETHODIMP Receive(IMediaSample* pSample);
    STDMETHODIMP EndOfStream();
};
