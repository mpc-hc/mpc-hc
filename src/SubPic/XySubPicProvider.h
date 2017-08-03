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

#include "ISubPic.h"
#include "SubRenderIntf.h"


interface __declspec(uuid("63679E0A-93AB-4656-AF40-589E4E985991"))
    IXyCompatProvider :
    public IUnknown
{
    STDMETHOD(RequestFrame)(REFERENCE_TIME start, REFERENCE_TIME stop, DWORD timeout) PURE;
    STDMETHOD(DeliverFrame)(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame) PURE;
    STDMETHOD(GetID)(ULONGLONG* id) PURE;
};

class CXySubPicProvider
    : public CUnknown
    , public ISubPicProvider
    , public IXyCompatProvider
{
    CComPtr<ISubRenderProvider> m_pSubRenderProvider;
    CComPtr<ISubRenderFrame> m_pSubFrame;
    CCritSec m_csSubRenderProvider;

    REFERENCE_TIME  m_rtStart;
    REFERENCE_TIME  m_rtStop;

    HANDLE m_hEvtDelivered;
public:
    CXySubPicProvider(ISubRenderProvider* provider);
    virtual ~CXySubPicProvider();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // IXyCompatProvider

    STDMETHODIMP DeliverFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame);
    STDMETHODIMP RequestFrame(REFERENCE_TIME start, REFERENCE_TIME stop, DWORD timeout);
    STDMETHODIMP GetID(ULONGLONG* id);

    // ISubPicProvider

    STDMETHODIMP Lock();
    STDMETHODIMP Unlock();

    STDMETHODIMP_(POSITION) GetStartPosition(REFERENCE_TIME rt, double fps) { return nullptr; }
    STDMETHODIMP_(POSITION) GetNext(POSITION pos) { return nullptr; }

    STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps) { return 0; }
    STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps) { return 0; }

    STDMETHODIMP_(bool) IsAnimated(POSITION pos) { return true; }

    STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);
    STDMETHODIMP GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft);
    STDMETHODIMP GetRelativeTo(POSITION pos, RelativeTo& relativeTo);
};

