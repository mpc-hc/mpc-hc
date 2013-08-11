/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "SubPicAllocatorPresenterImpl.h"
#include "../DSUtil/DSUtil.h"
#include "../filters/renderer/VideoRenderers/RenderersSettings.h"
#include <math.h>
#include "version.h"
#include "XySubPicQueueImpl.h"
#include "XySubPicProvider.h"
#include <d3d9.h>
#include <evr.h>
#include <dxva2api.h>

CSubPicAllocatorPresenterImpl::CSubPicAllocatorPresenterImpl(HWND hWnd, HRESULT& hr, CString* _pError)
    : CUnknown(NAME("CSubPicAllocatorPresenterImpl"), nullptr)
    , m_hWnd(hWnd)
    , m_NativeVideoSize(0, 0)
    , m_AspectRatio(0, 0)
    , m_VideoRect(0, 0, 0, 0)
    , m_WindowRect(0, 0, 0, 0)
    , m_fps(25.0)
    , m_rtSubtitleDelay(0)
    , m_bDeviceResetRequested(false)
    , m_bPendingResetDevice(false)
    , m_rtNow(0)
{
    if (!IsWindow(m_hWnd)) {
        hr = E_INVALIDARG;
        if (_pError) {
            *_pError += "Invalid window handle in ISubPicAllocatorPresenterImpl\n";
        }
        return;
    }
    GetWindowRect(m_hWnd, &m_WindowRect);
    SetVideoAngle(Vector(), false);
    hr = S_OK;
}

CSubPicAllocatorPresenterImpl::~CSubPicAllocatorPresenterImpl()
{
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{

    return
        QI(ISubPicAllocatorPresenter)
        QI(ISubPicAllocatorPresenter2)
        QI(ISubRenderOptions)
        QI(ISubRenderConsumer)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

void CSubPicAllocatorPresenterImpl::AlphaBltSubPic(const CRect& windowRect, const CRect& videoRect, SubPicDesc* pTarget)
{
    CComPtr<ISubPic> pSubPic;
    if (m_pSubPicQueue->LookupSubPic(m_rtNow, pSubPic)) {
        CRect rcSource, rcDest;
        if (SUCCEEDED(pSubPic->GetSourceAndDest(windowRect, videoRect, rcSource, rcDest))) {
            pSubPic->AlphaBlt(rcSource, rcDest, pTarget);
        }
    }
}

// ISubPicAllocatorPresenter

STDMETHODIMP_(SIZE) CSubPicAllocatorPresenterImpl::GetVideoSize(bool fCorrectAR)
{
    CSize VideoSize(GetVisibleVideoSize());

    if (fCorrectAR && m_AspectRatio.cx > 0 && m_AspectRatio.cy > 0) {
        VideoSize.cx = (LONGLONG(VideoSize.cy) * LONGLONG(m_AspectRatio.cx)) / LONGLONG(m_AspectRatio.cy);
    }

    return VideoSize;
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetPosition(RECT w, RECT v)
{
    bool fWindowPosChanged = !!(m_WindowRect != w);
    bool fWindowSizeChanged = !!(m_WindowRect.Size() != CRect(w).Size());

    m_WindowRect = w;

    bool fVideoRectChanged = !!(m_VideoRect != v);

    m_VideoRect = v;

    if (fWindowSizeChanged || fVideoRectChanged) {
        if (m_pAllocator) {
            m_pAllocator->SetCurSize(m_WindowRect.Size());
            m_pAllocator->SetCurVidRect(m_VideoRect);
        }

        if (m_pSubPicQueue) {
            m_pSubPicQueue->Invalidate();
        }
    }

    if (fWindowPosChanged || fVideoRectChanged) {
        Paint(fWindowSizeChanged || fVideoRectChanged);
    }
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetTime(REFERENCE_TIME rtNow)
{
    /*
    if (m_rtNow <= rtNow && rtNow <= m_rtNow + 1000000)
        return;
    */
    m_rtNow = rtNow - m_rtSubtitleDelay;

    if (m_pSubPicQueue) {
        m_pSubPicQueue->SetTime(m_rtNow);
    }
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetSubtitleDelay(int delay_ms)
{
    m_rtSubtitleDelay = delay_ms * 10000i64;
}

STDMETHODIMP_(int) CSubPicAllocatorPresenterImpl::GetSubtitleDelay()
{
    return (int)(m_rtSubtitleDelay / 10000);
}

STDMETHODIMP_(double) CSubPicAllocatorPresenterImpl::GetFPS()
{
    return m_fps;
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::SetSubPicProvider(ISubPicProvider* pSubPicProvider)
{
    m_SubPicProvider = pSubPicProvider;

    if (m_pSubPicQueue) {
        m_pSubPicQueue->SetSubPicProvider(pSubPicProvider);
    }
}

STDMETHODIMP_(void) CSubPicAllocatorPresenterImpl::Invalidate(REFERENCE_TIME rtInvalidate)
{
    if (m_pSubPicQueue) {
        m_pSubPicQueue->Invalidate(rtInvalidate);
    }
}

void CSubPicAllocatorPresenterImpl::Transform(CRect r, Vector v[4])
{
    v[0] = Vector((float)r.left, (float)r.top, 0);
    v[1] = Vector((float)r.right, (float)r.top, 0);
    v[2] = Vector((float)r.left, (float)r.bottom, 0);
    v[3] = Vector((float)r.right, (float)r.bottom, 0);

    Vector center((float)r.CenterPoint().x, (float)r.CenterPoint().y, 0);
    int l = (int)(Vector((float)r.Size().cx, (float)r.Size().cy, 0).Length() * 1.5f) + 1;

    for (size_t i = 0; i < 4; i++) {
        v[i] = m_xform << (v[i] - center);
        v[i].z = v[i].z / l + 0.5f;
        v[i].x /= v[i].z * 2;
        v[i].y /= v[i].z * 2;
        v[i] += center;
    }
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetVideoAngle(Vector v, bool fRepaint)
{
    m_xform = XForm(Ray(Vector(0, 0, 0), v), Vector(1, 1, 1), false);
    if (fRepaint) {
        Paint(true);
    }
    return S_OK;
}

// ISubRenderOptions

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetBool(LPCSTR field, bool* value)
{
    CheckPointer(value, E_POINTER);
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetInt(LPCSTR field, int* value)
{
    CheckPointer(value, E_POINTER);
    if (!strcmp(field, "supportedLevels")) {
        if (CComQIPtr<IMFVideoPresenter> pEVR = (ISubPicAllocatorPresenter*)this) {
            const CRenderersSettings& r = GetRenderersSettings();
            if (r.m_AdvRendSets.iEVROutputRange == 1) {
                *value = 3; // TV preferred
            } else {
                *value = 2; // PC preferred
            }
        } else {
            *value = 0; // PC only
        }
        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetSize(LPCSTR field, SIZE* value)
{
    CheckPointer(value, E_POINTER);
    if (!strcmp(field, "originalVideoSize")) {
        *value = GetVideoSize(false);
        return S_OK;
    } else if (!strcmp(field, "arAdjustedVideoSize")) {
        *value = GetVideoSize(true);
        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetRect(LPCSTR field, RECT* value)
{
    CheckPointer(value, E_POINTER);
    if (!strcmp(field, "videoOutputRect") || !strcmp(field, "subtitleTargetRect")) {
        value->left = 0;
        value->top = 0;
        value->right = m_VideoRect.Width();
        value->bottom = m_VideoRect.Height();
        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetUlonglong(LPCSTR field, ULONGLONG* value)
{
    CheckPointer(value, E_POINTER);
    if (!strcmp(field, "frameRate")) {
        *value = (REFERENCE_TIME)(10000000.0 / m_fps);
        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetDouble(LPCSTR field, double* value)
{
    CheckPointer(value, E_POINTER);
    if (!strcmp(field, "refreshRate")) {
        *value = 0;
        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetString(LPCSTR field, LPWSTR* value, int* chars)
{
    CheckPointer(value, E_POINTER);
    CheckPointer(chars, E_POINTER);
    CStringW ret = nullptr;

    if (!strcmp(field, "name")) {
        ret = L"MPC-HC";
    } else if (!strcmp(field, "version")) {
        ret = MPC_VERSION_STR;
    } else if (!strcmp(field, "yuvMatrix")) {
        ret = L"None";

        if (m_InputMediaType.IsValid() && m_InputMediaType.formattype == FORMAT_VideoInfo2) {
            VIDEOINFOHEADER2* pVIH2 = (VIDEOINFOHEADER2*)m_InputMediaType.pbFormat;

            if (pVIH2->dwControlFlags & AMCONTROL_COLORINFO_PRESENT) {
                DXVA2_ExtendedFormat& flags = (DXVA2_ExtendedFormat&)pVIH2->dwControlFlags;

                ret = (flags.NominalRange == DXVA2_NominalRange_Normal) ? L"PC." : L"TV.";

                switch (flags.VideoTransferMatrix) {
                    case DXVA2_VideoTransferMatrix_BT601:
                        ret.Append(L"601");
                        break;
                    case DXVA2_VideoTransferMatrix_BT709:
                        ret.Append(L"709");
                        break;
                    case DXVA2_VideoTransferMatrix_SMPTE240M:
                        ret.Append(L"240M");
                        break;
                    default:
                        ret = L"None";
                        break;
                }
            }
        }
    }

    if (!ret.IsEmpty()) {
        size_t len = ret.GetLength();
        size_t sz = (len + 1) * sizeof(WCHAR);
        LPWSTR buf = (LPWSTR)LocalAlloc(LPTR, sz);

        if (!buf) {
            return E_OUTOFMEMORY;
        }

        wcscpy_s(buf, len + 1, ret);
        *chars = len;
        *value = buf;

        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::GetBin(LPCSTR field, LPVOID* value, int* size)
{
    CheckPointer(value, E_POINTER);
    CheckPointer(size, E_POINTER);
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetBool(LPCSTR field, bool value)
{
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetInt(LPCSTR field, int value)
{
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetSize(LPCSTR field, SIZE value)
{
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetRect(LPCSTR field, RECT value)
{
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetUlonglong(LPCSTR field, ULONGLONG value)
{
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetDouble(LPCSTR field, double value)
{
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetString(LPCSTR field, LPWSTR value, int chars)
{
    return E_INVALIDARG;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::SetBin(LPCSTR field, LPVOID value, int size)
{
    return E_INVALIDARG;
}

// ISubRenderConsumer

STDMETHODIMP CSubPicAllocatorPresenterImpl::Connect(ISubRenderProvider* subtitleRenderer)
{
    HRESULT hr = E_FAIL;

    hr = subtitleRenderer->SetBool("combineBitmaps", true);
    if (FAILED(hr)) {
        return hr;
    }

    if (CComQIPtr<ISubRenderConsumer> pSubConsumer = m_pSubPicQueue) {
        hr = pSubConsumer->Connect(subtitleRenderer);
    } else {
        ISubPicProvider* pSubPicProvider = (ISubPicProvider*)DEBUG_NEW CXySubPicProvider(subtitleRenderer);
        ISubPicQueue* pSubPicQueue = GetRenderersSettings().nSPCSize > 0
                                     ? (ISubPicQueue*)DEBUG_NEW CXySubPicQueue(GetRenderersSettings().nSPCSize, m_pAllocator, &hr)
                                     : (ISubPicQueue*)DEBUG_NEW CXySubPicQueueNoThread(m_pAllocator, &hr);
        if (pSubPicQueue && SUCCEEDED(hr)) {
            pSubPicQueue->SetSubPicProvider(pSubPicProvider);
            m_SubPicProvider = pSubPicProvider;
            m_pSubPicQueue = pSubPicQueue;
        }
    }

    return hr;
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::Disconnect()
{
    return m_pSubPicQueue->SetSubPicProvider(nullptr);
}

STDMETHODIMP CSubPicAllocatorPresenterImpl::DeliverFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame)
{
    HRESULT hr = E_FAIL;

    if (CComQIPtr<IXyCompatProvider> pXyProvider = m_SubPicProvider) {
        hr = pXyProvider->DeliverFrame(start, stop, context, subtitleFrame);
    }

    return hr;
}
