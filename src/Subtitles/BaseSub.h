/*
 * (C) 2009-2013 see Authors.txt
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

    virtual HRESULT         ParseSample(IMediaSample* pSample) = 0;
    virtual void            EndOfStream() { /* Nothing to do */ };
    virtual void            Reset() = 0;
    virtual POSITION        GetStartPosition(REFERENCE_TIME rt, double fps) = 0;
    virtual POSITION        GetNext(POSITION pos) = 0;
    virtual REFERENCE_TIME  GetStart(POSITION nPos) = 0;
    virtual REFERENCE_TIME  GetStop(POSITION nPos) = 0;
    virtual void            Render(SubPicDesc& spd, REFERENCE_TIME rt, RECT& bbox) = 0;
    virtual HRESULT         GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VideoSize, POINT& VideoTopLeft) = 0;

protected:
    SUBTITLE_TYPE           m_nType;
};
