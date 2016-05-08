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

#pragma once

#include "realmedia/pncom.h"
#include "realmedia/pntypes.h"
#include "realmedia/pnwintyp.h"
#include "realmedia/rmacomm.h"
#include "realmedia/rmasite2.h"
#include "realmedia/rmavsurf.h"
#include "realmedia/rmawin.h"


namespace DSObjects
{

    struct REGION {
        long size;
        long numRects;
        PNxRect* rects;
        PNxRect extents;
        void* pOSRegion;

        REGION()
            : size(0)
            , numRects(0)
            , rects(nullptr)
            , pOSRegion(nullptr) {
            ZeroMemory(&extents, sizeof(extents));
        }
    };

    void ExtractRects(REGION* pRegion);
    REGION* RMACreateRectRegion(int left, int top, int right, int bottom);
    void RMASubtractRegion(REGION* regM, REGION* regS, REGION* regD);
    void RMAUnionRegion(REGION* reg1, REGION* reg2, REGION* regD);
    void RMAIntersectRegion(REGION* reg1, REGION* reg2, REGION* regD);
    BOOL RMAEqualRegion(REGION* reg1, REGION* reg2);
    void RMADestroyRegion(REGION* reg);
    REGION* RMACreateRegion();

    //
    // CRealMediaVideoSurface
    //

    class CRealMediaWindowlessSite;

    class CRealMediaVideoSurface
        : public CUnknown
        , public IRMAVideoSurface
    {
        void IntersectRect(PNxRect* pRect, PNxRect* pBox, PNxRect* pRetVal);

    protected:
        CComPtr<IUnknown> m_pContext;
        CRealMediaWindowlessSite* m_pSiteWindowless;
        RMABitmapInfoHeader m_bitmapInfo;
        RMABitmapInfoHeader m_lastBitmapInfo;

    public:
        CRealMediaVideoSurface(IUnknown* pContext, CRealMediaWindowlessSite* pSiteWindowless);
        virtual ~CRealMediaVideoSurface();

        DECLARE_IUNKNOWN;
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

        // IRMAVideoSurface

        STDMETHODIMP Blt(UCHAR* /*IN*/ pImageData, RMABitmapInfoHeader* /*IN*/ pBitmapInfo,
                         REF(PNxRect) /*IN*/ inDestRect, REF(PNxRect) /*IN*/ inSrcRect);
        STDMETHODIMP BeginOptimizedBlt(RMABitmapInfoHeader* /*IN*/ pBitmapInfo);
        STDMETHODIMP OptimizedBlt(UCHAR* /*IN*/ pImageBits, REF(PNxRect) /*IN*/ rDestRect, REF(PNxRect) /*IN*/ rSrcRect);
        STDMETHODIMP EndOptimizedBlt();
        STDMETHODIMP GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType);
        STDMETHODIMP GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType);
    };

    //
    // CRealMediaWindowlessSite
    //

    class CRealMediaWindowlessSite
        : public CUnknown
        , public IRMASite
        , public IRMASite2
        , public IRMASiteWindowless
        , public IRMAVideoSurface
    {
        CComQIPtr<IRMACommonClassFactory, &IID_IRMACommonClassFactory> m_pCCF;
        CComPtr<IUnknown> m_pContext;
        CComPtr<IRMAValues> m_pValues;

        CComPtr<IRMASiteUser> m_pUser;

        CRealMediaWindowlessSite* m_pParentSite;
        CInterfaceList<IRMASite, &IID_IRMASite> m_pChildren;

        CComPtr<IRMASiteWatcher> m_pWatcher;
        CInterfaceList<IRMAPassiveSiteWatcher, &IID_IRMAPassiveSiteWatcher> m_pPassiveWatchers;

        PNxSize m_size;
        PNxPoint m_position;
        bool m_fDamaged, m_fInRedraw, m_fIsVisible;
        INT32 m_lZOrder;

        //

        REGION* m_pRegion;
        REGION* m_pRegionWithoutChildren;

        void RecomputeRegion();
        void InternalRecomputeRegion();
        void ComputeRegion();
        void SubtractSite(REGION* pRegion);

        void UpdateZOrder(const CRealMediaWindowlessSite* pUpdatedChildSite, INT32 lOldZOrder, INT32 lNewZOrder);
        void SetInternalZOrder(INT32 lZOrder);

    public:
        CRealMediaWindowlessSite(HRESULT& hr, IUnknown* pContext, CRealMediaWindowlessSite* pParentSite = nullptr, IUnknown* pUnkOuter = nullptr);
        virtual ~CRealMediaWindowlessSite();

        DECLARE_IUNKNOWN;
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

        void GetTopLeft(PNxPoint* pPoint);
        REGION* GetRegion();

        // IRMASiteWindowless

        STDMETHODIMP EventOccurred(PNxEvent* /*IN*/ pEvent);
        STDMETHODIMP_(PNxWindow*) GetParentWindow();

        // IRMASite

        STDMETHODIMP AttachUser(IRMASiteUser* /*IN*/ pUser);
        STDMETHODIMP DetachUser();
        STDMETHODIMP GetUser(REF(IRMASiteUser*) /*OUT*/ pUser);

        STDMETHODIMP CreateChild(REF(IRMASite*) /*OUT*/ pChildSite);
        STDMETHODIMP DestroyChild(IRMASite* /*IN*/ pChildSite);

        STDMETHODIMP AttachWatcher(IRMASiteWatcher* /*IN*/ pWatcher);
        STDMETHODIMP DetachWatcher();

        STDMETHODIMP SetPosition(PNxPoint position);
        STDMETHODIMP GetPosition(REF(PNxPoint) position);
        STDMETHODIMP SetSize(PNxSize size);
        STDMETHODIMP GetSize(REF(PNxSize) size);

        STDMETHODIMP DamageRect(PNxRect rect);
        STDMETHODIMP DamageRegion(PNxRegion region);
        STDMETHODIMP ForceRedraw();

        // IRMASite2

        STDMETHODIMP UpdateSiteWindow(PNxWindow* /*IN*/ pWindow);
        STDMETHODIMP ShowSite(BOOL bShow);
        STDMETHODIMP_(BOOL) IsSiteVisible();
        STDMETHODIMP SetZOrder(INT32 lZOrder);
        STDMETHODIMP GetZOrder(REF(INT32) lZOrder);
        STDMETHODIMP MoveSiteToTop();
        STDMETHODIMP GetVideoSurface(REF(IRMAVideoSurface*) pSurface);
        STDMETHODIMP_(UINT32) GetNumberOfChildSites();

        STDMETHODIMP AddPassiveSiteWatcher(IRMAPassiveSiteWatcher* pWatcher);
        STDMETHODIMP RemovePassiveSiteWatcher(IRMAPassiveSiteWatcher* pWatcher);

        STDMETHODIMP SetCursor(PNxCursor cursor, REF(PNxCursor) oldCursor);

    private:
        void IntersectRect(const PNxRect* pRect, const PNxRect* pBox, PNxRect* pRetVal);

    protected:
        RMABitmapInfoHeader m_bitmapInfo;
        RMABitmapInfoHeader m_lastBitmapInfo;

        CComPtr<IRMAVideoSurface> m_pBltService;

    public:
        bool GetBltService(IRMAVideoSurface** ppBltService);
        void SetBltService(IRMAVideoSurface* ppBltService);

        // IRMAVideoSurface

        STDMETHODIMP Blt(UCHAR* /*IN*/ pImageData, RMABitmapInfoHeader* /*IN*/ pBitmapInfo,
                         REF(PNxRect) /*IN*/ inDestRect, REF(PNxRect) /*IN*/ inSrcRect);
        STDMETHODIMP BeginOptimizedBlt(RMABitmapInfoHeader* /*IN*/ pBitmapInfo);
        STDMETHODIMP OptimizedBlt(UCHAR* /*IN*/ pImageBits, REF(PNxRect) /*IN*/ rDestRect, REF(PNxRect) /*IN*/ rSrcRect);
        STDMETHODIMP EndOptimizedBlt();
        STDMETHODIMP GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType);
        STDMETHODIMP GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType);
    };

}
