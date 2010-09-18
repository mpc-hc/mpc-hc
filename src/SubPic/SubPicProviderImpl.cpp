/*
 *  $Id$
 *
 *  (C) 2003-2006 Gabest
 *  (C) 2006-2010 see AUTHORS
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "SubPicProviderImpl.h"
#include "../DSUtil/DSUtil.h"

CSubPicProviderImpl::CSubPicProviderImpl(CCritSec* pLock)
	: CUnknown(NAME("CSubPicProviderImpl"), NULL)
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
	return m_pLock ? m_pLock->Lock(), S_OK : E_FAIL;
}

STDMETHODIMP CSubPicProviderImpl::Unlock()
{
	return m_pLock ? m_pLock->Unlock(), S_OK : E_FAIL;
}
