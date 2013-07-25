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

#pragma once

#include "ISubPic.h"
#include "SubRenderIntf.h"

class CXySubPicQueueImpl
    : public CUnknown
    , public ISubPicQueue
    , public ISubRenderConsumer
{
protected:
    CCritSec m_csSubRenderProvider;
    CComPtr<ISubRenderProvider> m_pSubRenderProvider;

    double m_fps;
    REFERENCE_TIME m_rtNow;
    REFERENCE_TIME m_rtNowLast;

    CComPtr<ISubPicAllocator> m_pAllocator;

    HRESULT RenderTo(ISubPic* pSubPic, ISubRenderFrame* pSubFrame, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop);
public:
    CXySubPicQueueImpl(ISubPicAllocator* pAllocator, HRESULT* phr);
    virtual ~CXySubPicQueueImpl();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicQueue

    STDMETHODIMP SetSubPicProvider(ISubPicProvider* pSubPicProvider) { return E_NOTIMPL; }
    STDMETHODIMP GetSubPicProvider(ISubPicProvider** pSubPicProvider) { return E_NOTIMPL; }

    STDMETHODIMP SetFPS(double fps);
    STDMETHODIMP SetTime(REFERENCE_TIME rtNow);
    /*
    STDMETHODIMP Invalidate(REFERENCE_TIME rtInvalidate = -1) PURE;
    STDMETHODIMP_(bool) LookupSubPic(REFERENCE_TIME rtNow, ISubPic** ppSubPic) PURE;

    STDMETHODIMP GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop) PURE;
    STDMETHODIMP GetStats(int nSubPics, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop) PURE;
    */

    // ISubRenderOptions

    STDMETHODIMP GetBool(LPCSTR field, bool* value) { return E_NOTIMPL; }
    STDMETHODIMP GetInt(LPCSTR field, int* value) { return E_NOTIMPL; }
    STDMETHODIMP GetSize(LPCSTR field, SIZE* value) { return E_NOTIMPL; }
    STDMETHODIMP GetRect(LPCSTR field, RECT* value) { return E_NOTIMPL; }
    STDMETHODIMP GetUlonglong(LPCSTR field, ULONGLONG* value) { return E_NOTIMPL; }
    STDMETHODIMP GetDouble(LPCSTR field, double* value) { return E_NOTIMPL; }
    STDMETHODIMP GetString(LPCSTR field, LPWSTR* value, int* chars) { return E_NOTIMPL; }
    STDMETHODIMP GetBin(LPCSTR field, LPVOID* value, int* size) { return E_NOTIMPL; }
    STDMETHODIMP SetBool(LPCSTR field, bool value) { return E_NOTIMPL; }
    STDMETHODIMP SetInt(LPCSTR field, int value) { return E_NOTIMPL; }
    STDMETHODIMP SetSize(LPCSTR field, SIZE value) { return E_NOTIMPL; }
    STDMETHODIMP SetRect(LPCSTR field, RECT value) { return E_NOTIMPL; }
    STDMETHODIMP SetUlonglong(LPCSTR field, ULONGLONG value) { return E_NOTIMPL; }
    STDMETHODIMP SetDouble(LPCSTR field, double value) { return E_NOTIMPL; }
    STDMETHODIMP SetString(LPCSTR field, LPWSTR value, int chars) { return E_NOTIMPL; }
    STDMETHODIMP SetBin(LPCSTR field, LPVOID value, int size) { return E_NOTIMPL; }

    // ISubRenderConsumer

    STDMETHODIMP GetMerit(ULONG* plMerit) { return E_NOTIMPL; }
    STDMETHODIMP Connect(ISubRenderProvider* subtitleRenderer);
    STDMETHODIMP Disconnect();
    //STDMETHODIMP DeliverFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame) PURE;
};

class CXySubPicQueueNoThread : public CXySubPicQueueImpl
{
    CCritSec m_csLock;
    CComPtr<ISubRenderFrame> m_pSubFrame;
    CComPtr<ISubPic> m_pSubPic;

    ULONGLONG m_llSubId;
    REFERENCE_TIME m_rtStart;
    REFERENCE_TIME m_rtStop;
public:
    CXySubPicQueueNoThread(ISubPicAllocator* pAllocator, HRESULT* phr);
    virtual ~CXySubPicQueueNoThread();

    // ISubPicQueue

    STDMETHODIMP Invalidate(REFERENCE_TIME rtInvalidate = -1);
    STDMETHODIMP_(bool) LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& pSubPic);

    STDMETHODIMP GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
    STDMETHODIMP GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);

    // CXySubPicQueueImpl

    STDMETHODIMP DeliverFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame);
};
