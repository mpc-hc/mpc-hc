/* 
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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

	STDMETHODIMP_(POSITION) GetStartPosition(REFERENCE_TIME rt, double fps) = 0;
	STDMETHODIMP_(POSITION) GetNext(POSITION pos) = 0;

	STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps) = 0;
	STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps) = 0;

	STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox) = 0;
	STDMETHODIMP GetTextureSize (POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft) { return E_NOTIMPL; };
};

