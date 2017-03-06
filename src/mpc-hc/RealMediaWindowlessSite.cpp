/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2016 see Authors.txt
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
#include "RealMediaWindowlessSite.h"
#include "DSUtil.h"

#include "realmedia/pncom.h"
#include "realmedia/pnresult.h"
#include "realmedia/pntypes.h"
#include "realmedia/pnwintyp.h"
#include "realmedia/rmacomm.h"
#include "realmedia/rmaevent.h"
#include "realmedia/rmapckts.h"
#include "realmedia/rmasite2.h"
#include "realmedia/rmavsurf.h"
#include "realmedia/rmawin.h"

using namespace DSObjects;


void DSObjects::ExtractRects(REGION* pRegion)
{
    LPRGNDATA lpRgnData;

    DWORD sizeNeeed = GetRegionData((HRGN)pRegion->pOSRegion, 0, nullptr);

    lpRgnData = (LPRGNDATA)DEBUG_NEW char[sizeNeeed];
    DWORD returnValue = GetRegionData((HRGN)pRegion->pOSRegion, sizeNeeed, lpRgnData);
    UNREFERENCED_PARAMETER(returnValue);

    PN_VECTOR_DELETE(pRegion->rects);

    pRegion->numRects       = lpRgnData->rdh.nCount;
    pRegion->extents.left   = lpRgnData->rdh.rcBound.left;
    pRegion->extents.top    = lpRgnData->rdh.rcBound.top;
    pRegion->extents.right  = lpRgnData->rdh.rcBound.right;
    pRegion->extents.bottom = lpRgnData->rdh.rcBound.bottom;

    if (lpRgnData->rdh.nCount) {
        pRegion->rects = DEBUG_NEW PNxRect[lpRgnData->rdh.nCount];

        // now extract the information.

        for (int j = 0; j < (int) lpRgnData->rdh.nCount; j++) {
            RECT* pRect = (RECT*)lpRgnData->Buffer;
            pRegion->rects[j].left = pRect[j].left;
            pRegion->rects[j].top = pRect[j].top;
            pRegion->rects[j].right = pRect[j].right;
            pRegion->rects[j].bottom = pRect[j].bottom;
        }
    }

    PN_VECTOR_DELETE(lpRgnData);
}

REGION* DSObjects::RMACreateRectRegion(int left, int top, int right, int bottom)
{
    REGION* retVal = DEBUG_NEW REGION;
    retVal->pOSRegion = (void*)CreateRectRgn(left, top, right, bottom);
    ExtractRects(retVal);
    return retVal;
}

void DSObjects::RMASubtractRegion(REGION* regM, REGION* regS, REGION* regD)
{
    CombineRgn((HRGN)regD->pOSRegion, (HRGN)regM->pOSRegion, (HRGN)regS->pOSRegion, RGN_DIFF);
    ExtractRects(regD);
}

void DSObjects::RMAUnionRegion(REGION* reg1, REGION* reg2, REGION* regD)
{
    CombineRgn((HRGN)regD->pOSRegion, (HRGN)reg1->pOSRegion, (HRGN)reg2->pOSRegion, RGN_OR);
    ExtractRects(regD);
}

void DSObjects::RMAIntersectRegion(REGION* reg1, REGION* reg2, REGION* regD)
{
    CombineRgn((HRGN)regD->pOSRegion, (HRGN)reg1->pOSRegion, (HRGN)reg2->pOSRegion, RGN_AND);
    ExtractRects(regD);
}

BOOL DSObjects::RMAEqualRegion(REGION* reg1, REGION* reg2)
{
    return EqualRgn((HRGN)reg1->pOSRegion, (HRGN)reg2->pOSRegion)
           && !memcmp(&reg1->extents, &reg2->extents, sizeof(PNxRect)) ? TRUE : FALSE;
}

void DSObjects::RMADestroyRegion(REGION* reg)
{
    if (reg) {
        DeleteObject((HRGN)reg->pOSRegion);
        PN_VECTOR_DELETE(reg->rects);
    }
    PN_DELETE(reg);
}

REGION* DSObjects::RMACreateRegion()
{
    return RMACreateRectRegion(0, 0, 0, 0);
}

//
// CRealMediaWindowlessSite
//

CRealMediaWindowlessSite::CRealMediaWindowlessSite(HRESULT& hr, IUnknown* pContext, CRealMediaWindowlessSite* pParentSite, IUnknown* pUnkOuter)
    : CUnknown(NAME("CRealMediaWindowlessSite"), pUnkOuter, &hr)
    , m_pContext(pContext)
    , m_pParentSite(pParentSite)
    , m_pCCF(pContext)
    , m_fDamaged(false)
    , m_fInRedraw(false)
    , m_fIsVisible(true)
    , m_lZOrder(0)
    , m_pRegion(nullptr)
    , m_pRegionWithoutChildren(nullptr)
{
    ZeroMemory(&m_size, sizeof(m_size));
    ZeroMemory(&m_position, sizeof(m_position));
    ZeroMemory(&m_bitmapInfo, sizeof(m_bitmapInfo));
    ZeroMemory(&m_lastBitmapInfo, sizeof(m_lastBitmapInfo));

    hr = S_OK;

    if (!m_pContext || !m_pCCF) {
        hr = E_POINTER;
        return;
    }

    m_pCCF->CreateInstance(CLSID_IRMAValues, (void**)&m_pValues);
}

CRealMediaWindowlessSite::~CRealMediaWindowlessSite()
{
    POSITION pos = m_pChildren.GetHeadPosition();
    while (pos) {
        DestroyChild(m_pChildren.GetNext(pos));
    }

    RMADestroyRegion(m_pRegion);
    RMADestroyRegion(m_pRegionWithoutChildren);
}

STDMETHODIMP CRealMediaWindowlessSite::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    return
        QI2(IRMASite)
        QI2(IRMASite2)
        QI2(IRMASiteWindowless)
        QI2(IRMAVideoSurface)
        (m_pValues && m_pValues->QueryInterface(riid, ppv) == PNR_OK) ? PNR_OK :
        CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

// public

void CRealMediaWindowlessSite::GetTopLeft(PNxPoint* pPoint)
{
    pPoint->x += m_position.x;
    pPoint->y += m_position.y;

    if (m_pParentSite) {
        m_pParentSite->GetTopLeft(pPoint);
    }
}

REGION* CRealMediaWindowlessSite::GetRegion()
{
    return m_pRegion;
}

// private

void CRealMediaWindowlessSite::RecomputeRegion()
{
    if (m_pParentSite) {
        m_pParentSite->RecomputeRegion();
    } else {
        InternalRecomputeRegion();
    }
}

void CRealMediaWindowlessSite::InternalRecomputeRegion()
{
    ComputeRegion();

    POSITION pos = m_pChildren.GetHeadPosition();
    while (pos) {
        CRealMediaWindowlessSite* pSite = (CRealMediaWindowlessSite*)(IRMASite*)m_pChildren.GetNext(pos);
        if (pSite) {
            pSite->InternalRecomputeRegion();
        }
    }
}

void CRealMediaWindowlessSite::ComputeRegion()
{
    REGION* pTempRegion = nullptr;

    if (m_pRegion) {
        pTempRegion = RMACreateRegion();
        RMAUnionRegion(pTempRegion, m_pRegion, pTempRegion);
        RMADestroyRegion(m_pRegion);
    }

    if (m_pRegionWithoutChildren) {
        RMADestroyRegion(m_pRegionWithoutChildren);
    }

    PNxPoint topleft = {0, 0};
    GetTopLeft(&topleft);

    if (IsSiteVisible()) {
        m_pRegionWithoutChildren = RMACreateRectRegion(topleft.x, topleft.y, topleft.x + m_size.cx, topleft.y + m_size.cy);

        if (m_pParentSite) {
            RMAIntersectRegion(m_pRegionWithoutChildren, m_pParentSite->m_pRegionWithoutChildren, m_pRegionWithoutChildren);

            POSITION pos = m_pParentSite->m_pChildren.GetHeadPosition();
            while (pos) {
                CRealMediaWindowlessSite* pSiblingSite = (CRealMediaWindowlessSite*)(IRMASite*)m_pParentSite->m_pChildren.GetNext(pos);
                if (pSiblingSite != this) {
                    INT32 zOrder;
                    pSiblingSite->GetZOrder(zOrder);

                    if (zOrder > m_lZOrder && pSiblingSite->IsSiteVisible()) {
                        pSiblingSite->SubtractSite(m_pRegionWithoutChildren);
                    }
                }
            }
        }

        m_pRegion = RMACreateRegion();
        RMAUnionRegion(m_pRegion, m_pRegionWithoutChildren, m_pRegion);

        POSITION pos = m_pChildren.GetHeadPosition();
        while (pos) {
            CRealMediaWindowlessSite* pChildSite = (CRealMediaWindowlessSite*)(IRMASite*)m_pChildren.GetNext(pos);
            if (pChildSite->IsSiteVisible()) {
                pChildSite->SubtractSite(m_pRegion);
            }
        }
    } else {
        m_pRegionWithoutChildren = RMACreateRectRegion(0, 0, 0, 0);
        m_pRegion = RMACreateRectRegion(0, 0, 0, 0);
    }

    if (pTempRegion && !RMAEqualRegion(m_pRegion, pTempRegion)) {
        ForceRedraw();
    }

    RMADestroyRegion(pTempRegion);
}

void CRealMediaWindowlessSite::SubtractSite(REGION* pRegion)
{
    PNxPoint topLeft;
    ZeroMemory(&topLeft, sizeof(PNxPoint));
    GetTopLeft(&topLeft);

    REGION* pTempRegion = RMACreateRectRegion(topLeft.x, topLeft.y, topLeft.x + m_size.cx, topLeft.y + m_size.cy);

    RMASubtractRegion(pRegion, pTempRegion, pRegion);
    RMADestroyRegion(pTempRegion);
}

void CRealMediaWindowlessSite::UpdateZOrder(const CRealMediaWindowlessSite* pUpdatedChildSite, INT32 lOldZOrder, INT32 lNewZOrder)
{
    POSITION pos = m_pChildren.GetHeadPosition();
    while (pos) {
        CRealMediaWindowlessSite* pSite = (CRealMediaWindowlessSite*)(IRMASite*)m_pChildren.GetNext(pos);

        INT32 lItsOldZOrder;
        pSite->GetZOrder(lItsOldZOrder);

        if (pSite != pUpdatedChildSite) {
            if (lOldZOrder < lNewZOrder) {
                if (lItsOldZOrder >= lOldZOrder && lItsOldZOrder < lNewZOrder) {
                    pSite->SetInternalZOrder(lItsOldZOrder - 1);
                }
            } else {
                if (lItsOldZOrder >= lNewZOrder && lItsOldZOrder < lOldZOrder) {
                    pSite->SetInternalZOrder(lItsOldZOrder + 1);
                }
            }
        } else {
            pSite->SetInternalZOrder(lNewZOrder);
        }
    }
}

void CRealMediaWindowlessSite::SetInternalZOrder(INT32 lZOrder)
{
    m_lZOrder = lZOrder;
}

// IRMASiteWindowless

STDMETHODIMP CRealMediaWindowlessSite::EventOccurred(PNxEvent* /*IN*/ pEvent)
{
    return PNR_NOTIMPL;  /* not necessary within our implementation */
}

STDMETHODIMP_(PNxWindow*) CRealMediaWindowlessSite::GetParentWindow()
{
    return nullptr;
}

// IRMASite

STDMETHODIMP CRealMediaWindowlessSite::AttachUser(IRMASiteUser* /*IN*/ pUser)
{
    HRESULT hr = PNR_FAIL;

    if (m_pUser) {
        return PNR_UNEXPECTED;
    }

    if (CComQIPtr<IRMASite, &IID_IRMASite> pOuterSite = GetOwner()) {
        hr = pUser->AttachSite(pOuterSite);
    }

    if (PNR_OK == hr) {
        m_pUser = pUser;
    }

    return hr;
}

STDMETHODIMP CRealMediaWindowlessSite::DetachUser()
{
    HRESULT hr = PNR_OK;

    if (!m_pUser) {
        return PNR_UNEXPECTED;
    }

    hr = m_pUser->DetachSite();

    if (PNR_OK == hr) {
        m_pUser = nullptr;
    }

    return hr;
}

STDMETHODIMP CRealMediaWindowlessSite::GetUser(REF(IRMASiteUser*) /*OUT*/ pUser)
{
    HRESULT hr = PNR_OK;

    if (!m_pUser) {
        return PNR_UNEXPECTED;
    }

    (pUser = m_pUser)->AddRef();

    return hr;
}

STDMETHODIMP CRealMediaWindowlessSite::CreateChild(REF(IRMASite*) /*OUT*/ pChildSite)
{
    HRESULT hr = PNR_OK;

    CComPtr<IRMASite> pSite =
        (IRMASite*)DEBUG_NEW CRealMediaWindowlessSite(hr, m_pContext, this);

    if (FAILED(hr) || !pSite) {
        return E_FAIL;
    }

    pChildSite = pSite.Detach();

    m_pChildren.AddTail(pChildSite);

    return hr;
}

STDMETHODIMP CRealMediaWindowlessSite::DestroyChild(IRMASite* /*IN*/ pChildSite)
{
    if (POSITION pos = m_pChildren.Find(pChildSite)) {
        m_pChildren.RemoveAt(pos);
        return PNR_OK;
    }

    return PNR_UNEXPECTED;
}

STDMETHODIMP CRealMediaWindowlessSite::AttachWatcher(IRMASiteWatcher* /*IN*/ pWatcher)
{
    if (m_pWatcher) {
        return PNR_UNEXPECTED;
    }

    if (m_pWatcher = pWatcher) {
        m_pWatcher->AttachSite((IRMASite*)this);
    }

    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::DetachWatcher()
{
    if (!m_pWatcher) {
        return PNR_UNEXPECTED;
    }

    m_pWatcher->DetachSite();
    m_pWatcher = nullptr;

    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::SetPosition(PNxPoint position)
{
    HRESULT hr = PNR_OK;

    if (m_pWatcher) {
        hr = m_pWatcher->ChangingPosition(m_position, position);
    }

    if (PNR_OK == hr) {
        m_position = position;

        POSITION pos = m_pPassiveWatchers.GetHeadPosition();
        while (pos) {
            m_pPassiveWatchers.GetNext(pos)->PositionChanged(&position);
        }

        RecomputeRegion();
    }

    return hr;
}

STDMETHODIMP CRealMediaWindowlessSite::GetPosition(REF(PNxPoint) position)
{
    position = m_position;
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::SetSize(PNxSize size)
{
    HRESULT hr = PNR_OK;

    if (m_pWatcher) {
        hr = m_pWatcher->ChangingSize(m_size, size);
    }

    if (PNR_OK == hr && size.cx != 0 && size.cy != 0) {
        m_size = size;

        POSITION pos = m_pPassiveWatchers.GetHeadPosition();
        while (pos) {
            m_pPassiveWatchers.GetNext(pos)->SizeChanged(&size);
        }

        RecomputeRegion();
    }

    return hr;
}

STDMETHODIMP CRealMediaWindowlessSite::GetSize(REF(PNxSize) size)
{
    size = m_size;
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::DamageRect(PNxRect rect)
{
    m_fDamaged = TRUE;
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::DamageRegion(PNxRegion region)
{
    m_fDamaged = TRUE;
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::ForceRedraw()
{
    // make sure we have a visible window and are not re-entering and we have damage
    if (!m_fInRedraw && m_fDamaged && m_fIsVisible) {
        m_fInRedraw = TRUE;

        PNxEvent event = {RMA_SURFACE_UPDATE, nullptr, (IRMAVideoSurface*)this, nullptr, 0, 0};
        m_pUser->HandleEvent(&event);

        m_fInRedraw = FALSE;
        m_fDamaged = FALSE;
    }

    return PNR_OK;
}

// IRMASite2

STDMETHODIMP CRealMediaWindowlessSite::UpdateSiteWindow(PNxWindow* /*IN*/ pWindow)
{
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::ShowSite(BOOL bShow)
{
    m_fIsVisible = !!bShow;
    RecomputeRegion();
    return PNR_OK;
}

STDMETHODIMP_(BOOL) CRealMediaWindowlessSite::IsSiteVisible()
{
    BOOL fIsVisible = m_fIsVisible;
    if (m_pParentSite) {
        fIsVisible = fIsVisible && m_pParentSite->IsSiteVisible();
    }
    return fIsVisible;
}

STDMETHODIMP CRealMediaWindowlessSite::SetZOrder(INT32 lZOrder)
{
    if (!m_pParentSite) {
        return PNR_UNEXPECTED;
    }

    if (lZOrder == -1 || lZOrder >= (INT32)m_pParentSite->GetNumberOfChildSites()) {
        lZOrder = m_pParentSite->GetNumberOfChildSites() - 1;
    }

    if (m_lZOrder != lZOrder) {
        m_pParentSite->UpdateZOrder(this, m_lZOrder, lZOrder);
    }

    m_lZOrder = lZOrder;

    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::GetZOrder(REF(INT32) lZOrder)
{
    if (!m_pParentSite) {
        return PNR_UNEXPECTED;
    }
    lZOrder = m_lZOrder;
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::MoveSiteToTop()
{
    if (!m_pParentSite) {
        return PNR_UNEXPECTED;
    }
    return PNR_NOTIMPL;
}

STDMETHODIMP CRealMediaWindowlessSite::GetVideoSurface(REF(IRMAVideoSurface*) pSurface)
{
    (pSurface = (IRMAVideoSurface*)this)->AddRef();
    return PNR_OK;
}

STDMETHODIMP_(UINT32) CRealMediaWindowlessSite::GetNumberOfChildSites()
{
    return (UINT32)m_pChildren.GetCount();
}

STDMETHODIMP CRealMediaWindowlessSite::AddPassiveSiteWatcher(IRMAPassiveSiteWatcher* pWatcher)
{
    m_pPassiveWatchers.AddTail(pWatcher);
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::RemovePassiveSiteWatcher(IRMAPassiveSiteWatcher* pWatcher)
{
    if (POSITION pos = m_pPassiveWatchers.Find(pWatcher)) {
        m_pPassiveWatchers.RemoveAt(pos);
        return PNR_OK;
    }

    return PNR_UNEXPECTED;
}

STDMETHODIMP CRealMediaWindowlessSite::SetCursor(PNxCursor cursor, REF(PNxCursor) oldCursor)
{
    return PNR_NOTIMPL;
}

// private

void CRealMediaWindowlessSite::IntersectRect(const PNxRect* pRect, const PNxRect* pBox, PNxRect* pRetVal)
{
    pRetVal->left   = std::max(pRect->left, pBox->left);
    pRetVal->top    = std::max(pRect->top, pBox->top);
    pRetVal->right  = std::min(pRect->right, pBox->right);
    pRetVal->bottom = std::min(pRect->bottom, pBox->bottom);
}

// protected

bool CRealMediaWindowlessSite::GetBltService(IRMAVideoSurface** ppBltService)
{
    bool fRet = false;

    if (ppBltService) {
        if (m_pParentSite) {
            fRet = m_pParentSite->GetBltService(ppBltService);
        } else if (m_pBltService) {
            (*ppBltService = m_pBltService)->AddRef();
            fRet = true;
        }
    }

    return fRet;
}

void CRealMediaWindowlessSite::SetBltService(IRMAVideoSurface* pBltService)
{
    m_pBltService = pBltService;
}

// IRMAVideoSurface

STDMETHODIMP CRealMediaWindowlessSite::Blt(UCHAR*   /*IN*/ pImageData, RMABitmapInfoHeader* /*IN*/ pBitmapInfo, REF(PNxRect) /*IN*/ inDestRect, REF(PNxRect) /*IN*/ inSrcRect)
{
    BeginOptimizedBlt(pBitmapInfo);
    return OptimizedBlt(pImageData, inDestRect, inSrcRect);
}

STDMETHODIMP CRealMediaWindowlessSite::BeginOptimizedBlt(RMABitmapInfoHeader* /*IN*/ pBitmapInfo)
{
    if (memcmp(&m_bitmapInfo, pBitmapInfo, sizeof(RMABitmapInfoHeader))) {
        memcpy(&m_bitmapInfo, pBitmapInfo, sizeof(RMABitmapInfoHeader));

        // format of image has changed somehow.
        // do something here if this affects you.
    }

    /*
        CComPtr<IRMAVideoSurface> pBltService;
        GetBltService(&pBltService);
        if (!pBltService)
            return PNR_UNEXPECTED;

        RMA_COMPRESSION_TYPE ulType = (RMA_COMPRESSION_TYPE)-1;
        pBltService->GetPreferredFormat(ulType);

        if (pBitmapInfo->biCompression != ulType)
            return PNR_UNEXPECTED;
    */
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::OptimizedBlt(UCHAR* /*IN*/ pImageBits, REF(PNxRect) /*IN*/ rDestRect, REF(PNxRect) /*IN*/ rSrcRect)
{
    CComPtr<IRMAVideoSurface> pBltService;
    GetBltService(&pBltService);

    REGION* pRegion = GetRegion();

    if (!pBltService || !pRegion) {
        return PNR_UNEXPECTED;
    }

    PNxPoint origin;
    ZeroMemory(&origin, sizeof(PNxPoint));
    GetTopLeft(&origin);
    PNxRect adjustedDestRect;
    adjustedDestRect.left   = rDestRect.left + origin.x;
    adjustedDestRect.top    = rDestRect.top + origin.y;
    adjustedDestRect.right  = rDestRect.right + origin.x;
    adjustedDestRect.bottom = rDestRect.bottom + origin.y;

    for (int i = 0; i < pRegion->numRects; i++) {
        PNxRect* pRect = pRegion->rects + i;

        // intersect the dest rect with the rect from the
        // region to get the final dest rect.
        PNxRect finalDestRect;
        IntersectRect(&adjustedDestRect, pRect, &finalDestRect);

        // now compute the src rect for this blt.
        double xStretch = (double)(rDestRect.right - rDestRect.left) / (double)(rSrcRect.right - rSrcRect.left);
        double yStretch = (double)(rDestRect.bottom - rDestRect.top) / (double)(rSrcRect.bottom - rSrcRect.top);

        PNxRect finalSrcRect;
        finalSrcRect.left   = (INT32)((double)(finalDestRect.left - origin.x) / xStretch + 0.5);
        finalSrcRect.top    = (INT32)((double)(finalDestRect.top - origin.y)  / yStretch + 0.5);
        finalSrcRect.right  = (INT32)((double)(finalDestRect.right - origin.x) / xStretch + 0.5);
        finalSrcRect.bottom = (INT32)((double)(finalDestRect.bottom - origin.y) / yStretch + 0.5);

        pBltService->Blt(pImageBits, &m_bitmapInfo, finalDestRect, finalSrcRect);
    }

    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::EndOptimizedBlt()
{
    ZeroMemory(&m_bitmapInfo, sizeof(m_bitmapInfo));
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType)
{
    ulType = m_bitmapInfo.biCompression;
    return PNR_OK;
}

STDMETHODIMP CRealMediaWindowlessSite::GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType)
{
    CComPtr<IRMAVideoSurface> pBltService;
    GetBltService(&pBltService);
    if (!pBltService) {
        return PNR_UNEXPECTED;
    }

    return pBltService->GetPreferredFormat(ulType);
}
