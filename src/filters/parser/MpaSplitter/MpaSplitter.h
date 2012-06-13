/*
 * $Id$
 *
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

#include "../BaseSplitter/BaseSplitter.h"
#include "MpaSplitterFile.h"

#define MpaSplitterName L"MPC Mpa Splitter"
#define MpaSourceName   L"MPC Mpa Source"

class __declspec(uuid("0E9D4BF7-CBCB-46C7-BD80-4EF223A3DC2B"))
    CMpaSplitterFilter : public CBaseSplitterFilter
{
    REFERENCE_TIME m_rtStart;

protected:
    CAutoPtr<CMpaSplitterFile> m_pFile;
    HRESULT CreateOutputs(IAsyncReader* pAsyncReader);

    STDMETHODIMP GetDuration(LONGLONG* pDuration);

    bool DemuxInit();
    void DemuxSeek(REFERENCE_TIME rt);
    bool DemuxLoop();

public:
    CMpaSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // CBaseFilter

    STDMETHODIMP QueryFilterInfo(FILTER_INFO* pInfo);

};

class __declspec(uuid("59A0DB73-0287-4C9A-9D3C-8CFF39F8E5DB"))
    CMpaSourceFilter : public CMpaSplitterFilter
{
public:
    CMpaSourceFilter(LPUNKNOWN pUnk, HRESULT* phr);
};