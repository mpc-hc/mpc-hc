/*
 * $Id$
 *
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

#include "stdafx.h"
#include "madVRAllocatorPresenter.h"
#include "../../../SubPic/DX9SubPic.h"
#include "../../../SubPic/SubPicQueueImpl.h"
#include "RenderersSettings.h"
#include <moreuuids.h>

using namespace DSObjects;


interface __declspec(uuid("D6EE8031-214E-4E9E-A3A7-458925F933AB"))
IMadVRExclusiveModeInfo :
public IUnknown {
    STDMETHOD_(BOOL, IsExclusiveModeActive)(void) = 0;
    STDMETHOD_(BOOL, IsMadVRSeekbarEnabled)(void) = 0;
};


//
// CmadVRAllocatorPresenter
//

CmadVRAllocatorPresenter::CmadVRAllocatorPresenter(HWND hWnd, HRESULT& hr, CString& _Error)
    : CSubPicAllocatorPresenterImpl(hWnd, hr, &_Error)
    , m_ScreenSize(0, 0)
{
    if (FAILED(hr)) {
        _Error += L"ISubPicAllocatorPresenterImpl failed\n";
        return;
    }

    hr = S_OK;
}

CmadVRAllocatorPresenter::~CmadVRAllocatorPresenter()
{
    if (m_pSRCB) {
        // nasty, but we have to let it know about our death somehow
        ((CSubRenderCallback*)(ISubRenderCallback2*)m_pSRCB)->SetDXRAP(NULL);
    }

    // the order is important here
    m_pSubPicQueue = NULL;
    m_pAllocator = NULL;
    m_pDXR = NULL;
}

STDMETHODIMP CmadVRAllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    /*
        if (riid == __uuidof(IVideoWindow))
            return GetInterface((IVideoWindow*)this, ppv);
        if (riid == __uuidof(IBasicVideo))
            return GetInterface((IBasicVideo*)this, ppv);
        if (riid == __uuidof(IBasicVideo2))
            return GetInterface((IBasicVideo2*)this, ppv);
    */
    /*
        if (riid == __uuidof(IVMRWindowlessControl))
            return GetInterface((IVMRWindowlessControl*)this, ppv);
    */

    if (riid != IID_IUnknown && m_pDXR) {
        if (SUCCEEDED(m_pDXR->QueryInterface(riid, ppv))) {
            return S_OK;
        }
    }

    return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CmadVRAllocatorPresenter::SetDevice(IDirect3DDevice9* pD3DDev)
{
    if (!pD3DDev) {
        // release all resources
        m_pSubPicQueue = NULL;
        m_pAllocator = NULL;
        return S_OK;
    }

    CSize size;
    switch (GetRenderersSettings().nSPCMaxRes) {
        case 0:
        default:
            size = m_ScreenSize;
            break;
        case 1:
            size.SetSize(1024, 768);
            break;
        case 2:
            size.SetSize(800, 600);
            break;
        case 3:
            size.SetSize(640, 480);
            break;
        case 4:
            size.SetSize(512, 384);
            break;
        case 5:
            size.SetSize(384, 288);
            break;
        case 6:
            size.SetSize(2560, 1600);
            break;
        case 7:
            size.SetSize(1920, 1080);
            break;
        case 8:
            size.SetSize(1320, 900);
            break;
        case 9:
            size.SetSize(1280, 720);
            break;
    }

    if (m_pAllocator) {
        m_pAllocator->ChangeDevice(pD3DDev);
    } else {
        m_pAllocator = DNew CDX9SubPicAllocator(pD3DDev, size, GetRenderersSettings().fSPCPow2Tex, true);
        if (!m_pAllocator) {
            return E_FAIL;
        }
    }

    HRESULT hr = S_OK;

    m_pSubPicQueue = GetRenderersSettings().nSPCSize > 0
                     ? (ISubPicQueue*)DNew CSubPicQueue(GetRenderersSettings().nSPCSize, !GetRenderersSettings().fSPCAllowAnimationWhenBuffering, m_pAllocator, &hr)
                     : (ISubPicQueue*)DNew CSubPicQueueNoThread(m_pAllocator, &hr);
    if (!m_pSubPicQueue || FAILED(hr)) {
        return E_FAIL;
    }

    if (m_SubPicProvider) {
        m_pSubPicQueue->SetSubPicProvider(m_SubPicProvider);
    }

    return S_OK;
}

HRESULT CmadVRAllocatorPresenter::Render(
    REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME atpf,
    int left, int top, int right, int bottom, int width, int height)
{
    __super::SetPosition(CRect(0, 0, width, height), CRect(left, top, right, bottom)); // needed? should be already set by the player
    SetTime(rtStart);
    if (atpf > 0 && m_pSubPicQueue) {
        m_fps = (double)(10000000.0 / atpf);
        m_pSubPicQueue->SetFPS(m_fps);
    }
    AlphaBltSubPic(CSize(width, height));
    return S_OK;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CmadVRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);

    if (m_pDXR) {
        return E_UNEXPECTED;
    }
    m_pDXR.CoCreateInstance(CLSID_madVR, GetOwner());
    if (!m_pDXR) {
        return E_FAIL;
    }

    CComQIPtr<ISubRender> pSR = m_pDXR;
    if (!pSR) {
        m_pDXR = NULL;
        return E_FAIL;
    }

    m_pSRCB = DNew CSubRenderCallback(this);
    if (FAILED(pSR->SetCallback(m_pSRCB))) {
        m_pDXR = NULL;
        return E_FAIL;
    }

    (*ppRenderer = this)->AddRef();

    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    if (GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi)) {
        m_ScreenSize.SetSize(mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top);
    }

    return S_OK;
}

STDMETHODIMP_(void) CmadVRAllocatorPresenter::SetPosition(RECT w, RECT v)
{
    if (CComQIPtr<IBasicVideo> pBV = m_pDXR) {
        pBV->SetDefaultSourcePosition();
        pBV->SetDestinationPosition(v.left, v.top, v.right - v.left, v.bottom - v.top);
    }

    if (CComQIPtr<IVideoWindow> pVW = m_pDXR) {
        pVW->SetWindowPosition(w.left, w.top, w.right - w.left, w.bottom - w.top);
    }
}

STDMETHODIMP_(SIZE) CmadVRAllocatorPresenter::GetVideoSize(bool fCorrectAR)
{
    SIZE size = {0, 0};

    if (!fCorrectAR) {
        if (CComQIPtr<IBasicVideo> pBV = m_pDXR) {
            pBV->GetVideoSize(&size.cx, &size.cy);
        }
    } else {
        if (CComQIPtr<IBasicVideo2> pBV2 = m_pDXR) {
            pBV2->GetPreferredAspectRatio(&size.cx, &size.cy);
        }
    }

    return size;
}

STDMETHODIMP_(bool) CmadVRAllocatorPresenter::Paint(bool fAll)
{
    return false; // TODO
}

STDMETHODIMP CmadVRAllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
    HRESULT hr = E_NOTIMPL;
    if (CComQIPtr<IBasicVideo> pBV = m_pDXR) {
        hr = pBV->GetCurrentImage((long*)size, (long*)lpDib);
    }
    return hr;
}

STDMETHODIMP CmadVRAllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
    return E_NOTIMPL; // TODO
}
