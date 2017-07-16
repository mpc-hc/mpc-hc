/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2016 see Authors.txt
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
    , m_virtualTextureSize(0, 0)
    , m_virtualTextureTopLeft(0, 0)
    , m_bInvAlpha(false)
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

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetStart() const
{
    return m_rtStart;
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetStop() const
{
    return m_rtStop;
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetSegmentStart() const
{
    return m_rtSegmentStart >= 0 ? m_rtSegmentStart : m_rtStart;
}

STDMETHODIMP_(REFERENCE_TIME) CSubPicImpl::GetSegmentStop() const
{
    return m_rtSegmentStop >= 0 ? m_rtSegmentStop : m_rtStop;
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
    CheckPointer(pSubPic, E_POINTER);

    pSubPic->SetStart(m_rtStart);
    pSubPic->SetStop(m_rtStop);
    pSubPic->SetSegmentStart(m_rtSegmentStart);
    pSubPic->SetSegmentStop(m_rtSegmentStop);
    pSubPic->SetDirtyRect(m_rcDirty);
    pSubPic->SetSize(m_size, m_vidrect);
    pSubPic->SetVirtualTextureSize(m_virtualTextureSize, m_virtualTextureTopLeft);
    pSubPic->SetInverseAlpha(m_bInvAlpha);

    return S_OK;
}

STDMETHODIMP CSubPicImpl::GetDirtyRect(RECT* pDirtyRect) const
{
    CheckPointer(pDirtyRect, E_POINTER);

    *pDirtyRect = m_rcDirty;

    return S_OK;
}

STDMETHODIMP CSubPicImpl::GetSourceAndDest(RECT rcWindow, RECT rcVideo,
                                           RECT* pRcSource, RECT* pRcDest,
                                           const double videoStretchFactor /*= 1.0*/,
                                           int xOffsetInPixels /*= 0*/) const
{
    CheckPointer(pRcSource, E_POINTER);
    CheckPointer(pRcDest,   E_POINTER);

    if (m_size.cx > 0 && m_size.cy > 0) {
        CPoint offset(0, 0);
        double scaleX, scaleY;

        // Enable best fit only for HD contents since SD contents
        // are often anamorphic and thus break the auto-fit logic
        if (m_relativeTo == BEST_FIT && m_virtualTextureSize.cx > 720) {
            double scaleFactor;
            CRect videoRect(rcVideo);
            LONG stretch = lround(videoRect.Width() * (1.0 - videoStretchFactor) / 2.0);
            videoRect.left += stretch;
            videoRect.right -= stretch;
            CSize szVideo = videoRect.Size();

            double subtitleAR = double(m_virtualTextureSize.cx) / m_virtualTextureSize.cy;
            double videoAR = double(szVideo.cx) / szVideo.cy;

            double dCRVideoWidth = szVideo.cy * subtitleAR;
            double dCRVideoHeight = szVideo.cx / subtitleAR;

            if ((dCRVideoHeight > dCRVideoWidth) != (videoAR > subtitleAR)) {
                scaleFactor = dCRVideoHeight / m_virtualTextureSize.cy;
                offset.y = lround((szVideo.cy - dCRVideoHeight) / 2.0);
            } else {
                scaleFactor = dCRVideoWidth / m_virtualTextureSize.cx;
                offset.x = lround((szVideo.cx - dCRVideoWidth) / 2.0);
            }

            scaleX = scaleY = scaleFactor;
            offset += videoRect.TopLeft();
        } else {
            CRect rcTarget = (m_relativeTo == WINDOW) ? rcWindow : rcVideo;
            CSize szTarget = rcTarget.Size();
            scaleX = double(szTarget.cx) / m_virtualTextureSize.cx;
            scaleY = double(szTarget.cy) / m_virtualTextureSize.cy;
            offset += rcTarget.TopLeft();
        }

        CRect rcTemp = m_rcDirty;
        *pRcSource = rcTemp;

        rcTemp.OffsetRect(m_virtualTextureTopLeft + CPoint(xOffsetInPixels, 0));

        rcTemp = CRect(lround(rcTemp.left   * scaleX),
                       lround(rcTemp.top    * scaleY),
                       lround(rcTemp.right  * scaleX),
                       lround(rcTemp.bottom * scaleY));
        rcTemp.OffsetRect(offset);

        LONG stretch = lround(rcTemp.Width() * (1.0 - 1.0 / videoStretchFactor) / 2.0);
        rcTemp.left += stretch;
        rcTemp.right -= stretch;

        *pRcDest = rcTemp;

        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CSubPicImpl::SetDirtyRect(const RECT* pDirtyRect)
{
    CheckPointer(pDirtyRect, E_POINTER);

    m_rcDirty = *pDirtyRect;

    return S_OK;
}

STDMETHODIMP CSubPicImpl::GetMaxSize(SIZE* pMaxSize) const
{
    CheckPointer(pMaxSize, E_POINTER);

    *pMaxSize = m_maxsize;

    return S_OK;
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
    m_virtualTextureSize = m_size;

    return S_OK;
}

STDMETHODIMP CSubPicImpl::SetVirtualTextureSize(const SIZE pSize, const POINT pTopLeft)
{
    m_virtualTextureSize.SetSize(pSize.cx, pSize.cy);
    m_virtualTextureTopLeft.SetPoint(pTopLeft.x, pTopLeft.y);

    return S_OK;
}

STDMETHODIMP_(bool) CSubPicImpl::GetInverseAlpha() const
{
    return m_bInvAlpha;
}

STDMETHODIMP_(void) CSubPicImpl::SetInverseAlpha(bool bInverted)
{
    m_bInvAlpha = bInverted;
}

STDMETHODIMP CSubPicImpl::GetRelativeTo(RelativeTo* pRelativeTo) const
{
    CheckPointer(pRelativeTo, E_POINTER);

    *pRelativeTo = m_relativeTo;

    return S_OK;
}

STDMETHODIMP CSubPicImpl::SetRelativeTo(RelativeTo relativeTo)
{
    m_relativeTo = relativeTo;

    return S_OK;
}

//
// ISubPicAllocatorImpl
//

CSubPicAllocatorImpl::CSubPicAllocatorImpl(SIZE cursize, bool fDynamicWriteOnly)
    : CUnknown(NAME("ISubPicAllocatorImpl"), nullptr)
    , m_cursize(cursize)
    , m_fDynamicWriteOnly(fDynamicWriteOnly)
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
    CheckPointer(ppSubPic, E_POINTER);

    {
        CAutoLock cAutoLock(&m_staticLock);

        SIZE maxSize;
        if (m_pStatic && (FAILED(m_pStatic->GetMaxSize(&maxSize)) || maxSize.cx < m_cursize.cx || maxSize.cy < m_cursize.cy)) {
            m_pStatic.Release();
        }

        if (!m_pStatic) {
            if (!Alloc(true, &m_pStatic) || !m_pStatic) {
                return E_OUTOFMEMORY;
            }
        }

        *ppSubPic = m_pStatic;
    }

    (*ppSubPic)->AddRef();
    (*ppSubPic)->SetSize(m_cursize, m_curvidrect);

    return S_OK;
}

STDMETHODIMP CSubPicAllocatorImpl::AllocDynamic(ISubPic** ppSubPic)
{
    CheckPointer(ppSubPic, E_POINTER);

    if (!Alloc(false, ppSubPic) || !*ppSubPic) {
        return E_OUTOFMEMORY;
    }

    (*ppSubPic)->SetSize(m_cursize, m_curvidrect);

    return S_OK;
}

STDMETHODIMP_(bool) CSubPicAllocatorImpl::IsDynamicWriteOnly() const
{
    return m_fDynamicWriteOnly;
}

STDMETHODIMP CSubPicAllocatorImpl::ChangeDevice(IUnknown* pDev)
{
    return FreeStatic();
}

STDMETHODIMP CSubPicAllocatorImpl::FreeStatic()
{
    CAutoLock cAutoLock(&m_staticLock);
    m_pStatic.Release();
    return S_OK;
}
