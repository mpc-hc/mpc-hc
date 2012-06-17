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

#include "ISubPic.h"

class CSubPicQueueImpl : public CUnknown, public ISubPicQueue
{
    CCritSec m_csSubPicProvider;
    CComPtr<ISubPicProvider> m_pSubPicProvider;

protected:
    double m_fps;
    REFERENCE_TIME m_rtNow;
    REFERENCE_TIME m_rtNowLast;

    CComPtr<ISubPicAllocator> m_pAllocator;

    HRESULT RenderTo(ISubPic* pSubPic, REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, double fps, BOOL bIsAnimated);

public:
    CSubPicQueueImpl(ISubPicAllocator* pAllocator, HRESULT* phr);
    virtual ~CSubPicQueueImpl();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicQueue

    STDMETHODIMP SetSubPicProvider(ISubPicProvider* pSubPicProvider);
    STDMETHODIMP GetSubPicProvider(ISubPicProvider** pSubPicProvider);

    STDMETHODIMP SetFPS(double fps);
    STDMETHODIMP SetTime(REFERENCE_TIME rtNow);
    /*
    STDMETHODIMP Invalidate(REFERENCE_TIME rtInvalidate = -1) = 0;
    STDMETHODIMP_(bool) LookupSubPic(REFERENCE_TIME rtNow, ISubPic** ppSubPic) = 0;

    STDMETHODIMP GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop) = 0;
    STDMETHODIMP GetStats(int nSubPics, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop) = 0;
    */
};

class CSubPicQueue : public CSubPicQueueImpl, private CAMThread
{
    int m_nMaxSubPic;
    BOOL m_bDisableAnim;

    CInterfaceList<ISubPic> m_Queue;

    CCritSec m_csQueueLock; // for protecting CInterfaceList<ISubPic>
    REFERENCE_TIME UpdateQueue();
    void AppendQueue(ISubPic* pSubPic);
    int GetQueueCount();

    REFERENCE_TIME m_rtQueueMin;
    REFERENCE_TIME m_rtQueueMax;
    REFERENCE_TIME m_rtInvalidate;

    // CAMThread

    bool m_fBreakBuffering;
    enum {EVENT_EXIT, EVENT_TIME, EVENT_COUNT}; // IMPORTANT: _EXIT must come before _TIME if we want to exit fast from the destructor
    HANDLE m_ThreadEvents[EVENT_COUNT];
    DWORD ThreadProc();

public:
    CSubPicQueue(int nMaxSubPic, BOOL bDisableAnim, ISubPicAllocator* pAllocator, HRESULT* phr);
    virtual ~CSubPicQueue();

    // ISubPicQueue

    STDMETHODIMP SetFPS(double fps);
    STDMETHODIMP SetTime(REFERENCE_TIME rtNow);

    STDMETHODIMP Invalidate(REFERENCE_TIME rtInvalidate = -1);
    STDMETHODIMP_(bool) LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& pSubPic);

    STDMETHODIMP GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
    STDMETHODIMP GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
};

class CSubPicQueueNoThread : public CSubPicQueueImpl
{
    CCritSec m_csLock;
    CComPtr<ISubPic> m_pSubPic;

public:
    CSubPicQueueNoThread(ISubPicAllocator* pAllocator, HRESULT* phr);
    virtual ~CSubPicQueueNoThread();

    // ISubPicQueue

    STDMETHODIMP Invalidate(REFERENCE_TIME rtInvalidate = -1);
    STDMETHODIMP_(bool) LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& pSubPic);

    STDMETHODIMP GetStats(int& nSubPics, REFERENCE_TIME& rtNow, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
    STDMETHODIMP GetStats(int nSubPic, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop);
};
