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

class CSubPicProviderImpl : public CUnknown, public ISubPicProvider
{
protected:
    CCritSec* m_pLock;

public:
    CSubPicProviderImpl(CCritSec* pLock);
    virtual ~CSubPicProviderImpl();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicProvider

    STDMETHODIMP Lock();
    STDMETHODIMP Unlock();

    STDMETHODIMP_(POSITION) GetStartPosition(REFERENCE_TIME rt, double fps) PURE;
    STDMETHODIMP_(POSITION) GetNext(POSITION pos) PURE;

    STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps) PURE;
    STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps) PURE;

    STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox) PURE;
    STDMETHODIMP GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft) { return E_NOTIMPL; };
    STDMETHODIMP GetRelativeTo(POSITION pos, RelativeTo& relativeTo) { relativeTo = WINDOW; return S_OK; };
};
