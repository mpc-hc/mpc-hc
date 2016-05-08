/*
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

#include "DX9AllocatorPresenter.h"
#include "realmedia/pncom.h"
#include "realmedia/pnwintyp.h"
#include "realmedia/rmavsurf.h"

namespace DSObjects
{
    class CRM9AllocatorPresenter
        : public CDX9AllocatorPresenter
        , public IRMAVideoSurface
    {
        CComPtr<IDirect3DSurface9> m_pVideoSurfaceOff;
        CComPtr<IDirect3DSurface9> m_pVideoSurfaceYUY2;

        RMABitmapInfoHeader m_bitmapInfo;
        RMABitmapInfoHeader m_lastBitmapInfo;

    protected:
        HRESULT AllocSurfaces();
        void DeleteSurfaces();

    public:
        CRM9AllocatorPresenter(HWND hWnd, bool bFullscreen, HRESULT& hr, CString& _Error);

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

        // ISubPicAllocatorPresenter2
        STDMETHODIMP SetIsRendering(bool bIsRendering) {
            m_bIsRendering = bIsRendering;
            return S_OK;
        }

        // IRMAVideoSurface
        STDMETHODIMP Blt(UCHAR* pImageData, RMABitmapInfoHeader* pBitmapInfo, REF(PNxRect) inDestRect, REF(PNxRect) inSrcRect);
        STDMETHODIMP BeginOptimizedBlt(RMABitmapInfoHeader* pBitmapInfo);
        STDMETHODIMP OptimizedBlt(UCHAR* pImageBits, REF(PNxRect) rDestRect, REF(PNxRect) rSrcRect);
        STDMETHODIMP EndOptimizedBlt();
        STDMETHODIMP GetOptimizedFormat(REF(RMA_COMPRESSION_TYPE) ulType);
        STDMETHODIMP GetPreferredFormat(REF(RMA_COMPRESSION_TYPE) ulType);
    };
}
