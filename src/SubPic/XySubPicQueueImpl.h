/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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

#include "SubPicQueueImpl.h"
#include "XySubPicProvider.h"

#if 0
class CXySubPicQueue : public CSubPicQueue
{
protected:
    ULONGLONG m_llSubId;
    virtual DWORD ThreadProc();

public:
    CXySubPicQueue(int nMaxSubPic, ISubPicAllocator* pAllocator, HRESULT* phr);
    virtual ~CXySubPicQueue();

    // ISubPicQueue

    STDMETHODIMP Invalidate(REFERENCE_TIME rtInvalidate = -1);
};
#endif

class CXySubPicQueueNoThread : public CSubPicQueueNoThread
{
    ULONGLONG m_llSubId;

public:
    CXySubPicQueueNoThread(ISubPicAllocator* pAllocator, HRESULT* phr);
    virtual ~CXySubPicQueueNoThread();

    // ISubPicQueue

    STDMETHODIMP Invalidate(REFERENCE_TIME rtInvalidate = -1);
    STDMETHODIMP_(bool) LookupSubPic(REFERENCE_TIME rtNow, CComPtr<ISubPic>& pSubPic);
    STDMETHODIMP_(bool) LookupSubPic(REFERENCE_TIME rtNow, bool, CComPtr<ISubPic>& pSubPic);
};
