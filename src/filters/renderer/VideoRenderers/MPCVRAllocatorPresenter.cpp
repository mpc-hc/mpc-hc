/*
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
#include "RenderersSettings.h"
#include "MPCVRAllocatorPresenter.h"
#include "../../../SubPic/DX9SubPic.h"
#include "../../../SubPic/SubPicQueueImpl.h"
#include "moreuuids.h"
#include "FilterInterfaces.h"

using namespace DSObjects;

//
// CMPCVRAllocatorPresenter
//

CMPCVRAllocatorPresenter::CMPCVRAllocatorPresenter(HWND hWnd, HRESULT& hr, CString& _Error)
    : CSubPicAllocatorPresenterImpl(hWnd, hr, &_Error)
    , m_ScreenSize(0, 0)
{
    if (FAILED(hr)) {
        _Error += L"ISubPicAllocatorPresenterImpl failed\n";
        return;
    }

    hr = S_OK;
}

CMPCVRAllocatorPresenter::~CMPCVRAllocatorPresenter()
{
    if (m_pSRCB) {
        // nasty, but we have to let it know about our death somehow
        ((CSubRenderCallback*)(ISubRenderCallback*)m_pSRCB)->SetMPCVRAP(nullptr);
    }

    // the order is important here
    m_pSubPicQueue = nullptr;
    m_pAllocator = nullptr;
    m_pMPCVR = nullptr;
}

STDMETHODIMP CMPCVRAllocatorPresenter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid != IID_IUnknown && m_pMPCVR) {
        if (SUCCEEDED(m_pMPCVR->QueryInterface(riid, ppv))) {
            return S_OK;
        }
    }

    return __super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CMPCVRAllocatorPresenter::SetDevice(IDirect3DDevice9* pD3DDev)
{
    if (!pD3DDev) {
        // release all resources
        m_pSubPicQueue = nullptr;
        m_pAllocator = nullptr;
        __super::SetPosition(CRect(), CRect());
        return S_OK;
    }

    const CRenderersSettings& r = GetRenderersSettings();

    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    if (GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &mi)) {
        m_ScreenSize.SetSize(mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top);
    }
    InitMaxSubtitleTextureSize(r.subPicQueueSettings.nMaxRes, m_ScreenSize);

    if (m_pAllocator) {
        m_pAllocator->ChangeDevice(pD3DDev);
    } else {
        m_pAllocator = DEBUG_NEW CDX9SubPicAllocator(pD3DDev, m_maxSubtitleTextureSize, true);
        m_condAllocatorReady.notify_one();
    }

    HRESULT hr = S_OK;
    {
        // Lock before check because m_pSubPicQueue might be initialized in CSubPicAllocatorPresenterImpl::Connect
        CAutoLock cAutoLock(this);
        if (!m_pSubPicQueue) {
            m_pSubPicQueue = r.subPicQueueSettings.nSize > 0
                             ? (ISubPicQueue*)DEBUG_NEW CSubPicQueue(r.subPicQueueSettings, m_pAllocator, &hr)
                             : (ISubPicQueue*)DEBUG_NEW CSubPicQueueNoThread(r.subPicQueueSettings, m_pAllocator, &hr);
        } else {
            this->Unlock();
            m_pSubPicQueue->Invalidate();
        }
    }

    if (SUCCEEDED(hr) && m_pSubPicQueue && m_pSubPicProvider) {
        m_pSubPicQueue->SetSubPicProvider(m_pSubPicProvider);
    }

    return hr;
}

HRESULT CMPCVRAllocatorPresenter::Render(
    REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME atpf,
    int left, int top, int right, int bottom, int width, int height)
{
    CRect wndRect(0, 0, width, height);
    CRect videoRect(left, top, right, bottom);
    __super::SetPosition(wndRect, videoRect); // needed? should be already set by the player
    SetTime(rtStart);
    if (atpf > 0 && m_pSubPicQueue) {
        m_pSubPicQueue->SetFPS(10000000.0 / atpf);
    }
    AlphaBltSubPic(wndRect, videoRect);
    return S_OK;
}

// ISubPicAllocatorPresenter

STDMETHODIMP CMPCVRAllocatorPresenter::CreateRenderer(IUnknown** ppRenderer)
{
    CheckPointer(ppRenderer, E_POINTER);

    if (m_pMPCVR) {
        return E_UNEXPECTED;
    }
    m_pMPCVR.CoCreateInstance(CLSID_MPCVR, GetOwner());
    if (!m_pMPCVR) {
        return E_FAIL;
    }

    CComQIPtr<ISubRender> pSR = m_pMPCVR;
    if (!pSR) {
        m_pMPCVR = nullptr;
        return E_FAIL;
    }

    m_pSRCB = DEBUG_NEW CSubRenderCallback(this);
    if (FAILED(pSR->SetCallback(m_pSRCB))) {
        m_pMPCVR = nullptr;
        return E_FAIL;
    }

    (*ppRenderer = (IUnknown*)(INonDelegatingUnknown*)(this))->AddRef();

    return S_OK;
}

STDMETHODIMP_(void) CMPCVRAllocatorPresenter::SetPosition(RECT w, RECT v)
{
    if (CComQIPtr<IBasicVideo> pBV = m_pMPCVR) {
        pBV->SetDefaultSourcePosition();
        pBV->SetDestinationPosition(v.left, v.top, v.right - v.left, v.bottom - v.top);
    }

    if (CComQIPtr<IVideoWindow> pVW = m_pMPCVR) {
        pVW->SetWindowPosition(w.left, w.top, w.right - w.left, w.bottom - w.top);
    }

    SetVideoSize(GetVideoSize(), GetVideoSize(true));
}

STDMETHODIMP CMPCVRAllocatorPresenter::SetRotation(int rotation)
{
    HRESULT hr = E_NOTIMPL;
    if (CComQIPtr<IExFilterConfig> pIExFilterConfig = m_pMPCVR) {
        hr = pIExFilterConfig->SetInt("rotate", rotation);
    }
    return hr;
}

STDMETHODIMP_(int) CMPCVRAllocatorPresenter::GetRotation()
{
    if (CComQIPtr<IExFilterConfig> pIExFilterConfig = m_pMPCVR) {
        int rotation = 0;
        if (SUCCEEDED(pIExFilterConfig->GetInt("rotation", &rotation))) {
            return rotation;
        }
    }
    return 0;
}


STDMETHODIMP_(SIZE) CMPCVRAllocatorPresenter::GetVideoSize(bool bCorrectAR) const
{
    SIZE size = {0, 0};

    if (!bCorrectAR) {
        if (CComQIPtr<IBasicVideo> pBV = m_pMPCVR) {
            pBV->GetVideoSize(&size.cx, &size.cy);
        }
    } else {
        if (CComQIPtr<IBasicVideo2> pBV2 = m_pMPCVR) {
            pBV2->GetPreferredAspectRatio(&size.cx, &size.cy);
        }
    }

    return size;
}

STDMETHODIMP_(bool) CMPCVRAllocatorPresenter::Paint(bool bAll)
{
    if (CComQIPtr<IExFilterConfig> pIExFilterConfig = m_pMPCVR) {
        return SUCCEEDED(pIExFilterConfig->SetBool("cmd_redraw", true));
    }
    return false;
}

STDMETHODIMP CMPCVRAllocatorPresenter::GetDIB(BYTE* lpDib, DWORD* size)
{
    HRESULT hr = E_NOTIMPL;
    if (CComQIPtr<IBasicVideo> pBV = m_pMPCVR) {
        hr = pBV->GetCurrentImage((long*)size, (long*)lpDib);
    }
    return hr;
}

STDMETHODIMP_(bool) CMPCVRAllocatorPresenter::IsRendering()
{
    if (CComQIPtr<IExFilterConfig> pIExFilterConfig = m_pMPCVR) {
        int playbackState;
        if (SUCCEEDED(pIExFilterConfig->GetInt("playbackState", &playbackState))) {
            return playbackState == State_Running;
        }
    }
    return false;
}

STDMETHODIMP CMPCVRAllocatorPresenter::SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget)
{
    return E_NOTIMPL; // TODO
}
