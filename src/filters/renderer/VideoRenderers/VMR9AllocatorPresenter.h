/*
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

#include "DX9AllocatorPresenter.h"

namespace DSObjects
{
    class CVMR9AllocatorPresenter
        : public CDX9AllocatorPresenter
        , public IVMRSurfaceAllocator9
        , public IVMRImagePresenter9
        , public IVMRWindowlessControl9
    {
    protected:
        CComPtr<IVMRSurfaceAllocatorNotify9> m_pIVMRSurfAllocNotify;
        CInterfaceArray<IDirect3DSurface9> m_pSurfaces;

        HRESULT CreateDevice(CString& _Error);
        void DeleteSurfaces();

        bool m_fUseInternalTimer;
        REFERENCE_TIME m_rtPrevStart;

    public:
        CVMR9AllocatorPresenter(HWND hWnd, bool bFullscreen, HRESULT& hr, CString& _Error);

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

        // ISubPicAllocatorPresenter
        STDMETHODIMP CreateRenderer(IUnknown** ppRenderer);
        STDMETHODIMP_(void) SetTime(REFERENCE_TIME rtNow);

        // IVMRSurfaceAllocator9
        STDMETHODIMP InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers);
        STDMETHODIMP TerminateDevice(DWORD_PTR dwID);
        STDMETHODIMP GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9** lplpSurface);
        STDMETHODIMP AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify);

        // IVMRImagePresenter9
        STDMETHODIMP StartPresenting(DWORD_PTR dwUserID);
        STDMETHODIMP StopPresenting(DWORD_PTR dwUserID);
        STDMETHODIMP PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo);

        // IVMRWindowlessControl9
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
    };
}
