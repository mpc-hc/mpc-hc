/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015, 2017 see Authors.txt
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
#include <memory>
#include <vector>

enum {
    MSP_RGB32,
    MSP_RGB24,
    MSP_RGB16,
    MSP_RGB15,
    MSP_YUY2,
    MSP_YV12,
    MSP_IYUV,
    MSP_AYUV,
    MSP_RGBA
};

// CMemSubPic
class CMemSubPicAllocator;
class CMemSubPic : public CSubPicImpl
{
    CComPtr<CMemSubPicAllocator> m_pAllocator;

    SubPicDesc m_spd;
    std::unique_ptr<SubPicDesc> m_resizedSpd;

protected:
    STDMETHODIMP_(void*) GetObject(); // returns SubPicDesc*

public:
    CMemSubPic(const SubPicDesc& spd, CMemSubPicAllocator* pAllocator);
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

class CMemSubPicAllocator : public CSubPicAllocatorImpl, public CCritSec
{
    int m_type;
    CSize m_maxsize;

    std::vector<std::pair<size_t, BYTE*>> m_freeMemoryChunks;

    bool Alloc(bool fStatic, ISubPic** ppSubPic);

public:
    CMemSubPicAllocator(int type, SIZE maxsize);
    virtual ~CMemSubPicAllocator();

    bool AllocSpdBits(SubPicDesc& spd);
    void FreeSpdBits(SubPicDesc& spd);

    STDMETHODIMP SetMaxTextureSize(SIZE maxTextureSize) override;
};
