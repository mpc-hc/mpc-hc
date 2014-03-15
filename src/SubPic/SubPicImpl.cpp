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
#include "SubPicImpl.h"
#include "../DSUtil/DSUtil.h"

//
// CSubPicImpl
//

CSubPicImpl::CSubPicImpl()
    : CUnknown(NAME("CSubPicImpl"), nullptr)
    , m_rtStart(0)
    , m_rtStop(0)
    , m_rtSegmentStart(0)
    , m_rtSegmentStop(0)
    , m_rcDirty(0, 0, 0, 0)
    , m_maxsize(0, 0)
    , m_size(0, 0)
    , m_vidrect(0, 0, 0, 0)
    , m_VirtualTextureSize(0, 0)
    , m_VirtualTextureTopLeft(0, 0)
    , m_invAlpha(false)
    , m_relativeTo(WINDOW)
{
}

STDMETHODIMP CSubPicImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(ISubPic)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPic

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetStart()
{
    return m_rtStart;
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetStop()
{
    return m_rtStop;
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetSegmentStart()
{
    return m_rtSegmentStart ? m_rtSegmentStart : m_rtStart;
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetSegmentStop()
{
    return m_rtSegmentStop ? m_rtSegmentStop : m_rtStop;
}

STDMETHODIMP_(void) CSubPicImpl::SetSegmentStart(REFERENCE_TIME rtStart)
{
    m_rtSegmentStart = rtStart;
}

STDMETHODIMP_(void) CSubPicImpl::SetSegmentStop(REFERENCE_TIME rtStop)
{
    m_rtSegmentStop = rtStop;
}

STDMETHODIMP_(void) CSubPicImpl::SetStart(REFERENCE_TIME rtStart)
{
    m_rtStart = rtStart;
}

STDMETHODIMP_(void) CSubPicImpl::SetStop(REFERENCE_TIME rtStop)
{
    m_rtStop = rtStop;
}

STDMETHODIMP CSubPicImpl::CopyTo(ISubPic* pSubPic)
{
    if (!pSubPic) {
        return E_POINTER;
    }

    pSubPic->SetStart(m_rtStart);
    pSubPic->SetStop(m_rtStop);
    pSubPic->SetSegmentStart(m_rtSegmentStart);
    pSubPic->SetSegmentStop(m_rtSegmentStop);
    pSubPic->SetDirtyRect(m_rcDirty);
    pSubPic->SetSize(m_size, m_vidrect);
    pSubPic->SetVirtualTextureSize(m_VirtualTextureSize, m_VirtualTextureTopLeft);
    pSubPic->SetInverseAlpha(m_invAlpha);

    return S_OK;
}

STDMETHODIMP CSubPicImpl::GetDirtyRect(RECT* pDirtyRect)
{
    return pDirtyRect ? *pDirtyRect = m_rcDirty, S_OK : E_POINTER;
}

STDMETHODIMP CSubPicImpl::GetSourceAndDest(RECT rcWindow, RECT rcVideo, RECT* pRcSource, RECT* pRcDest)
{
    CheckPointer(pRcSource, E_POINTER);
    CheckPointer(pRcDest,   E_POINTER);

    if (m_size.cx > 0 && m_size.cy > 0) {
        CPoint offset(0, 0);
        double scaleX = 1.0, scaleY = 1.0;

        // Enable best fit only for HD contents since SD contents
        // are often anamorphic and thus break the auto-fit logic
        if (m_relativeTo == BEST_FIT && m_VirtualTextureSize.cx > 720) {
            double scaleFactor = 1.0;
            CSize szVideo = CRect(rcVideo).Size();

            double subtitleAR = double(m_VirtualTextureSize.cx) / m_VirtualTextureSize.cy;
            double videoAR = double(szVideo.cx) / szVideo.cy;

            double dCRVideoWidth = szVideo.cy * subtitleAR;
            double dCRVideoHeight = szVideo.cx / subtitleAR;

            if ((dCRVideoHeight > dCRVideoWidth) != (videoAR > subtitleAR)) {
                scaleFactor = dCRVideoHeight / m_VirtualTextureSize.cy;
                offset.y = lround((szVideo.cy - dCRVideoHeight) / 2.0);
            } else {
                scaleFactor = dCRVideoWidth / m_VirtualTextureSize.cx;
                offset.x = lround((szVideo.cx - dCRVideoWidth) / 2.0);
            }

            scaleX = scaleY = scaleFactor;
            offset += CRect(rcVideo).TopLeft();
        } else {
            CRect rcTarget = (m_relativeTo == WINDOW) ? rcWindow : rcVideo;
            CSize szTarget = rcTarget.Size();
            scaleX = double(szTarget.cx) / m_VirtualTextureSize.cx;
            scaleY = double(szTarget.cy) / m_VirtualTextureSize.cy;
            offset += rcTarget.TopLeft();
        }

        CRect rcTemp = m_rcDirty;
        *pRcSource = rcTemp;

        rcTemp.OffsetRect(m_VirtualTextureTopLeft);
        rcTemp = CRect(lround(rcTemp.left   * scaleX),
                       lround(rcTemp.top    * scaleY),
                       lround(rcTemp.right  * scaleX),
                       lround(rcTemp.bottom * scaleY));
        rcTemp.OffsetRect(offset);
        *pRcDest = rcTemp;

        return S_OK;
    } else {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CSubPicImpl::SetDirtyRect(RECT* pDirtyRect)
{
    return pDirtyRect ? m_rcDirty = *pDirtyRect, S_OK : E_POINTER;
}

STDMETHODIMP CSubPicImpl::GetMaxSize(SIZE* pMaxSize)
{
    return pMaxSize ? *pMaxSize = m_maxsize, S_OK : E_POINTER;
}

STDMETHODIMP CSubPicImpl::SetSize(SIZE size, RECT vidrect)
{
    m_size = size;
    m_vidrect = vidrect;

    if (m_size.cx > m_maxsize.cx) {
        m_size.cy = MulDiv(m_size.cy, m_maxsize.cx, m_size.cx);
        m_size.cx = m_maxsize.cx;
    }

    if (m_size.cy > m_maxsize.cy) {
        m_size.cx = MulDiv(m_size.cx, m_maxsize.cy, m_size.cy);
        m_size.cy = m_maxsize.cy;
    }

    if (m_size.cx != size.cx || m_size.cy != size.cy) {
        m_vidrect.top    = MulDiv(m_vidrect.top,    m_size.cx, size.cx);
        m_vidrect.bottom = MulDiv(m_vidrect.bottom, m_size.cx, size.cx);
        m_vidrect.left   = MulDiv(m_vidrect.left,   m_size.cy, size.cy);
        m_vidrect.right  = MulDiv(m_vidrect.right,  m_size.cy, size.cy);
    }
    m_VirtualTextureSize = m_size;

    return S_OK;
}

STDMETHODIMP CSubPicImpl::SetVirtualTextureSize(const SIZE pSize, const POINT pTopLeft)
{
    m_VirtualTextureSize.SetSize(pSize.cx, pSize.cy);
    m_VirtualTextureTopLeft.SetPoint(pTopLeft.x, pTopLeft.y);

    return S_OK;
}

STDMETHODIMP_(bool) CSubPicImpl::GetInverseAlpha()
{
    return m_invAlpha;
}

STDMETHODIMP_(void) CSubPicImpl::SetInverseAlpha(bool bInverted)
{
    m_invAlpha = bInverted;
}

STDMETHODIMP CSubPicImpl::GetRelativeTo(RelativeTo* pRelativeTo)
{
    return pRelativeTo ? *pRelativeTo = m_relativeTo, S_OK : E_POINTER;
}

STDMETHODIMP CSubPicImpl::SetRelativeTo(RelativeTo relativeTo)
{
    m_relativeTo = relativeTo;

    return S_OK;
}

//
// ISubPicAllocatorImpl
//

CSubPicAllocatorImpl::CSubPicAllocatorImpl(SIZE cursize, bool fDynamicWriteOnly, bool fPow2Textures)
    : CUnknown(NAME("ISubPicAllocatorImpl"), nullptr)
    , m_cursize(cursize)
    , m_fDynamicWriteOnly(fDynamicWriteOnly)
    , m_fPow2Textures(fPow2Textures)
{
    m_curvidrect = CRect(CPoint(0, 0), m_cursize);
}

STDMETHODIMP CSubPicAllocatorImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI(ISubPicAllocator)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPicAllocator

STDMETHODIMP CSubPicAllocatorImpl::SetCurSize(SIZE cursize)
{
    m_cursize = cursize;
    return S_OK;
}

STDMETHODIMP CSubPicAllocatorImpl::SetCurVidRect(RECT curvidrect)
{
    m_curvidrect = curvidrect;
    return S_OK;
}

STDMETHODIMP CSubPicAllocatorImpl::GetStatic(ISubPic** ppSubPic)
{
    if (!ppSubPic) {
        return E_POINTER;
    }

    SIZE maxSize;
    if (m_pStatic && (FAILED(m_pStatic->GetMaxSize(&maxSize)) || maxSize.cx < m_cursize.cx || maxSize.cy < m_cursize.cy)) {
        m_pStatic.Release();
        m_pStatic = nullptr;
    }

    if (!m_pStatic) {
        if (!Alloc(true, &m_pStatic) || !m_pStatic) {
            return E_OUTOFMEMORY;
        }
    }

    m_pStatic->SetSize(m_cursize, m_curvidrect);

    (*ppSubPic = m_pStatic)->AddRef();

    return S_OK;
}

STDMETHODIMP CSubPicAllocatorImpl::AllocDynamic(ISubPic** ppSubPic)
{
    if (!ppSubPic) {
        return E_POINTER;
    }

    if (!Alloc(false, ppSubPic) || !*ppSubPic) {
        return E_OUTOFMEMORY;
    }

    (*ppSubPic)->SetSize(m_cursize, m_curvidrect);

    return S_OK;
}

STDMETHODIMP_(bool) CSubPicAllocatorImpl::IsDynamicWriteOnly()
{
    return m_fDynamicWriteOnly;
}

STDMETHODIMP CSubPicAllocatorImpl::ChangeDevice(IUnknown* pDev)
{
    m_pStatic = nullptr;
    return S_OK;
}
