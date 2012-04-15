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

#include "SubPicImpl.h"
#include "SubPicAllocatorPresenterImpl.h"
#include <dx/d3d.h>

// CDX7SubPic

class CDX7SubPic : public CSubPicImpl
{
	CComPtr<IDirect3DDevice7> m_pD3DDev;
	CComPtr<IDirectDrawSurface7> m_pSurface;

protected:
	STDMETHODIMP_(void*) GetObject(); // returns IDirectDrawSurface7*

public:
	CDX7SubPic(IDirect3DDevice7* pD3DDev, IDirectDrawSurface7* pSurface);

	// ISubPic
	STDMETHODIMP GetDesc(SubPicDesc& spd);
	STDMETHODIMP CopyTo(ISubPic* pSubPic);
	STDMETHODIMP ClearDirtyRect(DWORD color);
	STDMETHODIMP Lock(SubPicDesc& spd);
	STDMETHODIMP Unlock(RECT* pDirtyRect);
	STDMETHODIMP AlphaBlt(RECT* pSrc, RECT* pDst, SubPicDesc* pTarget);
};

// CDX7SubPicAllocator

class CDX7SubPicAllocator : public CSubPicAllocatorImpl, public CCritSec
{
	CComPtr<IDirect3DDevice7> m_pD3DDev;
	CSize m_maxsize;

	bool Alloc(bool fStatic, ISubPic** ppSubPic);

public:
	CDX7SubPicAllocator(IDirect3DDevice7* pD3DDev, SIZE maxsize, bool fPow2Textures);

	// ISubPicAllocator
	STDMETHODIMP ChangeDevice(IUnknown* pDev);
};
