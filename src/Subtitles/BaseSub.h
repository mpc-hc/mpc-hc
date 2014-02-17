/*
 * (C) 2009-2014 see Authors.txt
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

enum SOURCE_MATRIX {
    NONE,
    BT_709,
    BT_601
};

class CBaseSub
{
public:
    static const REFERENCE_TIME INVALID_TIME = _I64_MIN;

    CBaseSub() = delete;
    explicit CBaseSub(SUBTITLE_TYPE nType)
        : m_nType(nType) {
    }
    virtual ~CBaseSub() = default;

    virtual HRESULT         ParseSample(IMediaSample* pSample) PURE;
    virtual void            EndOfStream() PURE;
    virtual void            Reset() PURE;
    virtual POSITION        GetStartPosition(REFERENCE_TIME rt, double fps) PURE;
    virtual POSITION        GetNext(POSITION pos) PURE;
    virtual REFERENCE_TIME  GetStart(POSITION nPos) PURE;
    virtual REFERENCE_TIME  GetStop(POSITION nPos) PURE;
    virtual void            Render(SubPicDesc& spd, REFERENCE_TIME rt, RECT& bbox) PURE;
    virtual HRESULT         GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VideoSize, POINT& VideoTopLeft) PURE;

    void SetSourceTargetInfo(int sourceBlackLevel, int sourceWhiteLevel,
                             int targetBlackLevel, int targetWhiteLevel, SOURCE_MATRIX sourceMatrix) {
        m_infoSourceTarget.sourceBlackLevel = sourceBlackLevel;
        m_infoSourceTarget.sourceWhiteLevel = sourceWhiteLevel;
        m_infoSourceTarget.targetBlackLevel = targetBlackLevel;
        m_infoSourceTarget.targetWhiteLevel = targetWhiteLevel;
        m_infoSourceTarget.sourceMatrix = sourceMatrix;
    }

protected:
    SUBTITLE_TYPE m_nType;

    struct SourceTarget {
        int sourceBlackLevel = 16;
        int sourceWhiteLevel = 235;
        int targetBlackLevel = 0;
        int targetWhiteLevel = 255;

        SOURCE_MATRIX sourceMatrix = NONE;
    } m_infoSourceTarget;
};
