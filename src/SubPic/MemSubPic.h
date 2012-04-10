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

enum {MSP_RGB32,MSP_RGB24,MSP_RGB16,MSP_RGB15,MSP_YUY2,MSP_YV12,MSP_IYUV,MSP_AYUV,MSP_RGBA};

// CMemSubPic

class CMemSubPic : public CSubPicImpl
{
#pragma warning(disable: 4799)
	SubPicDesc m_spd;

protected:
	STDMETHODIMP_(void*) GetObject(); // returns SubPicDesc*

public:
	CMemSubPic(SubPicDesc& spd);
	virtual ~CMemSubPic();

	// ISubPic
	STDMETHODIMP GetDesc(SubPicDesc& spd);
	STDMETHODIMP CopyTo(ISubPic* pSubPic);
	STDMETHODIMP ClearDirtyRect(DWORD color);
	STDMETHODIMP Lock(SubPicDesc& spd);
	STDMETHODIMP Unlock(RECT* pDirtyRect);
	STDMETHODIMP AlphaBlt(RECT* pSrc, RECT* pDst, SubPicDesc* pTarget);
};

// CMemSubPicAllocator

class CMemSubPicAllocator : public CSubPicAllocatorImpl
{
	int m_type;
	CSize m_maxsize;

	bool Alloc(bool fStatic, ISubPic** ppSubPic);

public:
	CMemSubPicAllocator(int type, SIZE maxsize);
};
