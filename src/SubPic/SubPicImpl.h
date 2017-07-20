/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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

class CSubPicImpl : public CUnknown, public ISubPic
{
protected:
    REFERENCE_TIME m_rtStart, m_rtStop;
    REFERENCE_TIME m_rtSegmentStart, m_rtSegmentStop;
    CRect  m_rcDirty;
    CSize  m_maxsize;
    CSize  m_size;
    CRect  m_vidrect;
    CSize  m_virtualTextureSize;
    CPoint m_virtualTextureTopLeft;
    bool   m_bInvAlpha;
    RelativeTo m_relativeTo;

    /*

                             Texture
           +-------+---------------------------------+
           |       .                                 |   .
           |       .             m_maxsize           |       .
    TextureTopLeft .<=============================== |======>    .              Video
           | . . . +-------------------------------- | -----+       +-----------------------------------+
           |       |                         .       |      |       |  m_vidrect                        |
           |       |                         .       |      |       |                                   |
           |       |                         .       |      |       |                                   |
           |       |        +-----------+    .       |      |       |                                   |
           |       |        | m_rcDirty |    .       |      |       |                                   |
           |       |        |           |    .       |      |       |                                   |
           |       |        +-----------+    .       |      |       |                                   |
           |       +-------------------------------- | -----+       |                                   |
           |                    m_size               |              |                                   |
           |       <=========================>       |              |                                   |
           |                                         |              |                                   |
           |                                         |              +-----------------------------------+
           |                                         |          .
           |                                         |      .
           |                                         |   .
           +-----------------------------------------+
                      m_VirtualTextureSize
           <=========================================>

    */


public:
    CSubPicImpl();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPic

    STDMETHODIMP_(REFERENCE_TIME) GetStart() const;
    STDMETHODIMP_(REFERENCE_TIME) GetStop() const;
    STDMETHODIMP_(void) SetStart(REFERENCE_TIME rtStart);
    STDMETHODIMP_(void) SetStop(REFERENCE_TIME rtStop);

    STDMETHODIMP GetDesc(SubPicDesc& spd) PURE;
    STDMETHODIMP CopyTo(ISubPic* pSubPic);

    STDMETHODIMP ClearDirtyRect(DWORD color) PURE;
    STDMETHODIMP GetDirtyRect(RECT* pDirtyRect) const;
    STDMETHODIMP SetDirtyRect(const RECT* pDirtyRect);

    STDMETHODIMP GetMaxSize(SIZE* pMaxSize) const;
    STDMETHODIMP SetSize(SIZE size, RECT vidrect);

    STDMETHODIMP Lock(SubPicDesc& spd) PURE;
    STDMETHODIMP Unlock(RECT* pDirtyRect) PURE;

    STDMETHODIMP AlphaBlt(RECT* pSrc, RECT* pDst, SubPicDesc* pTarget) PURE;

    STDMETHODIMP SetVirtualTextureSize(const SIZE pSize, const POINT pTopLeft);
    STDMETHODIMP GetSourceAndDest(RECT rcWindow, RECT rcVideo, RECT* pRcSource,
                                  RECT* pRcDest, const double videoStretchFactor = 1.0,
                                  int xOffsetInPixels = 1) const;
    STDMETHODIMP GetRelativeTo(RelativeTo* pRelativeTo) const;
    STDMETHODIMP SetRelativeTo(RelativeTo relativeTo);

    STDMETHODIMP_(REFERENCE_TIME) GetSegmentStart() const;
    STDMETHODIMP_(REFERENCE_TIME) GetSegmentStop() const;
    STDMETHODIMP_(void) SetSegmentStart(REFERENCE_TIME rtStart);
    STDMETHODIMP_(void) SetSegmentStop(REFERENCE_TIME rtStop);
    STDMETHODIMP_(bool) GetInverseAlpha() const;
    STDMETHODIMP_(void) SetInverseAlpha(bool bInverted);
};


class CSubPicAllocatorImpl : public CUnknown, public ISubPicAllocator
{
protected:
    CCritSec m_staticLock;
    CComPtr<ISubPic> m_pStatic;

    CSize m_cursize;
    CRect m_curvidrect;
    bool m_fDynamicWriteOnly;

    virtual bool Alloc(bool fStatic, ISubPic** ppSubPic) PURE;

public:
    CSubPicAllocatorImpl(SIZE cursize, bool fDynamicWriteOnly);

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicAllocator

    STDMETHODIMP SetCurSize(SIZE cursize);
    STDMETHODIMP SetCurVidRect(RECT curvidrect);
    STDMETHODIMP GetStatic(ISubPic** ppSubPic);
    STDMETHODIMP AllocDynamic(ISubPic** ppSubPic);
    STDMETHODIMP_(bool) IsDynamicWriteOnly() const;
    STDMETHODIMP ChangeDevice(IUnknown* pDev);
    STDMETHODIMP SetMaxTextureSize(SIZE maxTextureSize) PURE;
    STDMETHODIMP FreeStatic();
};
