/*
 * (C) 2008-2014 see Authors.txt
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

#include "../SubPic/SubPicProviderImpl.h"


class __declspec(uuid("FCA68599-C83E-4ea5-94A3-C2E1B0E326B9"))
    CRLECodedSubtitle : public CSubPicProviderImpl, public ISubStream
{
public:
    static const REFERENCE_TIME INVALID_TIME = _I64_MIN;
    static const REFERENCE_TIME INFINITE_TIME = _I64_MAX;

    CRLECodedSubtitle(CCritSec* pLock, const CString& name, LCID lcid);
    virtual ~CRLECodedSubtitle() = default;

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicProvider
    STDMETHODIMP GetRelativeTo(POSITION pos, RelativeTo& relativeTo);

    // IPersist
    STDMETHODIMP GetClassID(CLSID* pClassID);

    // ISubStream
    STDMETHODIMP_(int) GetStreamCount();
    STDMETHODIMP GetStreamInfo(int i, WCHAR** ppName, LCID* pLCID);
    STDMETHODIMP_(int) GetStream();
    STDMETHODIMP SetStream(int iStream);
    STDMETHODIMP Reload();

    virtual HRESULT ParseSample(IMediaSample* pSample) PURE;
    HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    virtual void EndOfStream() PURE;
    virtual void Reset() PURE;
    STDMETHODIMP SetSourceTargetInfo(CString yuvMatrix, int targetBlackLevel, int targetWhiteLevel);

protected:
    CCritSec        m_csCritSec;

    CString         m_name;
    LCID            m_lcid;
    REFERENCE_TIME  m_rtCurrentSegmentStart;

    enum SOURCE_MATRIX {
        NONE,
        BT_709,
        BT_601
    };

    struct SourceTarget {
        int sourceBlackLevel = 16;
        int sourceWhiteLevel = 235;
        int targetBlackLevel = 0;
        int targetWhiteLevel = 255;

        SOURCE_MATRIX sourceMatrix = NONE;
    } m_infoSourceTarget;
};
