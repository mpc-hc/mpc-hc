/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

#include "DX7AllocatorPresenter.h"

namespace DSObjects
{

    class CVMR7AllocatorPresenter
        : public CDX7AllocatorPresenter
        , public IVMRSurfaceAllocator
        , public IVMRImagePresenter
        , public IVMRWindowlessControl
    {
        CComPtr<IVMRSurfaceAllocatorNotify> m_pIVMRSurfAllocNotify;
        CComPtr<IVMRSurfaceAllocator> m_pSA;

        HRESULT CreateDevice();
        void DeleteSurfaces();

        bool m_fUseInternalTimer;

    public:
        CVMR7AllocatorPresenter(HWND hWnd, HRESULT& hr);

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

        // ISubPicAllocatorPresenter
        STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
        STDMETHODIMP_(void) SetTime(REFERENCE_TIME rtNow);

        // IVMRSurfaceAllocator
        STDMETHODIMP AllocateSurface(DWORD_PTR dwUserID, VMRALLOCATIONINFO* lpAllocInfo, DWORD* lpdwBuffer, LPDIRECTDRAWSURFACE7* lplpSurface);
        STDMETHODIMP FreeSurface(DWORD_PTR dwUserID);
        STDMETHODIMP PrepareSurface(DWORD_PTR dwUserID, IDirectDrawSurface7* lpSurface, DWORD dwSurfaceFlags);
        STDMETHODIMP AdviseNotify(IVMRSurfaceAllocatorNotify* lpIVMRSurfAllocNotify);

        // IVMRImagePresenter
        STDMETHODIMP StartPresenting(DWORD_PTR dwUserID);
        STDMETHODIMP StopPresenting(DWORD_PTR dwUserID);
        STDMETHODIMP PresentImage(DWORD_PTR dwUserID, VMRPRESENTATIONINFO* lpPresInfo);

        // IVMRWindowlessControl
        STDMETHODIMP GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight);
        STDMETHODIMP GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight);
        STDMETHODIMP GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight);
        STDMETHODIMP SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect);
        STDMETHODIMP GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect);
        STDMETHODIMP GetAspectRatioMode(DWORD* lpAspectRatioMode);
        STDMETHODIMP SetAspectRatioMode(DWORD AspectRatioMode);
        STDMETHODIMP SetVideoClippingWindow(HWND hwnd);
        STDMETHODIMP RepaintVideo(HWND hwnd, HDC hdc);
        STDMETHODIMP DisplayModeChanged();
        STDMETHODIMP GetCurrentImage(BYTE** lpDib);
        STDMETHODIMP SetBorderColor(COLORREF Clr);
        STDMETHODIMP GetBorderColor(COLORREF* lpClr);
        STDMETHODIMP SetColorKey(COLORREF Clr);
        STDMETHODIMP GetColorKey(COLORREF* lpClr);
    };

}
