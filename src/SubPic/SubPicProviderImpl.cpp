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

#include "stdafx.h"
#include "SubPicProviderImpl.h"
#include "../DSUtil/DSUtil.h"

CSubPicProviderImpl::CSubPicProviderImpl(CCritSec* pLock)
    : CUnknown(NAME("CSubPicProviderImpl"), nullptr)
    , m_pLock(pLock)
{
}

CSubPicProviderImpl::~CSubPicProviderImpl()
{
}

STDMETHODIMP CSubPicProviderImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(ISubPicProvider)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// CSubPicProviderImpl

STDMETHODIMP CSubPicProviderImpl::Lock()
{
    CheckPointer(m_pLock, E_FAIL);

    m_pLock->Lock();

    return S_OK;
}

STDMETHODIMP CSubPicProviderImpl::Unlock()
{
    CheckPointer(m_pLock, E_FAIL);

    m_pLock->Unlock();

    return S_OK;
}
