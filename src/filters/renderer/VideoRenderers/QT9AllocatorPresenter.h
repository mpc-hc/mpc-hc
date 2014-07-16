/*
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

#pragma once

#include "DX9AllocatorPresenter.h"
#include "IQTVideoSurface.h"

namespace DSObjects
{
    class CQT9AllocatorPresenter
        : public CDX9AllocatorPresenter
        , public IQTVideoSurface
    {
        CComPtr<IDirect3DSurface9> m_pVideoSurfaceOff;

    protected:
        HRESULT AllocSurfaces();
        void DeleteSurfaces();

    public:
        CQT9AllocatorPresenter(HWND hWnd, bool bFullscreen, HRESULT& hr, CString& _Error);

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

        // ISubPicAllocatorPresenter2
        STDMETHODIMP SetIsRendering(bool bIsRendering) {
            m_bIsRendering = bIsRendering;
            return S_OK;
        }

        // IQTVideoSurface
        STDMETHODIMP BeginBlt(const BITMAP& bm);
        STDMETHODIMP DoBlt(const BITMAP& bm);
    };
}
