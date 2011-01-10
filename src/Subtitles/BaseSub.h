/*
 * $Id$
 *
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#pragma once

#include "CompositionObject.h"

enum SUBTITLE_TYPE {
	ST_DVB,
	ST_HDMV
};

class CBaseSub
{
public:

	static const REFERENCE_TIME INVALID_TIME = _I64_MIN;

	CBaseSub(SUBTITLE_TYPE nType);
	virtual ~CBaseSub();

	virtual HRESULT			ParseSample (IMediaSample* pSample) = NULL;
	virtual void			Reset() = NULL;
	virtual POSITION		GetStartPosition(REFERENCE_TIME rt, double fps) = NULL;
	virtual POSITION		GetNext(POSITION pos) = NULL;
	virtual REFERENCE_TIME	GetStart(POSITION nPos) = NULL;
	virtual REFERENCE_TIME	GetStop(POSITION nPos)  = NULL;
	virtual void			Render(SubPicDesc& spd, REFERENCE_TIME rt, RECT& bbox) = NULL;
	virtual HRESULT			GetTextureSize (POSITION pos, SIZE& MaxTextureSize, SIZE& VideoSize, POINT& VideoTopLeft) = NULL;

protected :
	SUBTITLE_TYPE			m_nType;
};
