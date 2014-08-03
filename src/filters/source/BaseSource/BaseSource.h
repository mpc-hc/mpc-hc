/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2012, 2014 see Authors.txt
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

#include "../../../DSUtil/DSUtil.h"

template<class TStream>
class CBaseSource
    : public CSource
    , public IFileSourceFilter
    , public IAMFilterMiscFlags
{
protected:
    CStringW m_fn;

public:
    CBaseSource(TCHAR* name, LPUNKNOWN lpunk, HRESULT* phr, const CLSID& clsid)
        : CSource(name, lpunk, clsid) {
        if (phr) {
            *phr = S_OK;
        }
    }

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) {
        CheckPointer(ppv, E_POINTER);

        return
            QI(IFileSourceFilter)
            QI(IAMFilterMiscFlags)
            __super::NonDelegatingQueryInterface(riid, ppv);
    }

    // IFileSourceFilter

    STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt) {
        // TODO: destroy any already existing pins and create new, now we are just going die nicely instead of doing it :)
        if (GetPinCount() > 0) {
            return VFW_E_ALREADY_CONNECTED;
        }

        HRESULT hr = S_OK;
        if (!(DEBUG_NEW TStream(pszFileName, this, &hr))) {
            return E_OUTOFMEMORY;
        }

        if (FAILED(hr)) {
            return hr;
        }

        m_fn = pszFileName;

        return S_OK;
    }

    STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt) {
        CheckPointer(ppszFileName, E_POINTER);

        size_t nCount = m_fn.GetLength() + 1;
        *ppszFileName = (LPOLESTR)CoTaskMemAlloc(nCount * sizeof(WCHAR));
        if (!(*ppszFileName)) {
            return E_OUTOFMEMORY;
        }

        wcscpy_s(*ppszFileName, nCount, m_fn);

        return S_OK;
    }

    // IAMFilterMiscFlags

    STDMETHODIMP_(ULONG) GetMiscFlags() {
        return AM_FILTER_MISC_FLAGS_IS_SOURCE;
    }
};

class CBaseStream
    : public CSourceStream
    , public CSourceSeeking
{
protected:
    CCritSec m_cSharedState;

    REFERENCE_TIME m_AvgTimePerFrame;
    REFERENCE_TIME m_rtSampleTime, m_rtPosition;

    BOOL m_bDiscontinuity, m_bFlushing;

    HRESULT OnThreadStartPlay();
    HRESULT OnThreadCreate();

private:
    void UpdateFromSeek();
    STDMETHODIMP SetRate(double dRate);

    HRESULT ChangeStart();
    HRESULT ChangeStop();
    HRESULT ChangeRate() { return S_OK; }

public:
    CBaseStream(TCHAR* name, CSource* pParent, HRESULT* phr);
    virtual ~CBaseStream();

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    HRESULT FillBuffer(IMediaSample* pSample);

    virtual HRESULT FillBuffer(IMediaSample* pSample, int nFrame, BYTE* pOut, long& len /*in+out*/) = 0;

    STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
};
