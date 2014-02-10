/*
 * (C) 2003-2006 Gabest
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

#include <atlbase.h>
#include <atlcoll.h>
#include "ISubPic.h"
#include "CoordGeom.h"
#include "SubRenderIntf.h"

class CSubPicAllocatorPresenterImpl
    : public CUnknown
    , public CCritSec
    , public ISubPicAllocatorPresenter2
    , public ISubRenderConsumer
{
protected:
    HWND m_hWnd;
    REFERENCE_TIME m_rtSubtitleDelay;

    CSize m_NativeVideoSize, m_AspectRatio;
    CRect m_VideoRect, m_WindowRect;

    REFERENCE_TIME m_rtNow;
    double m_fps;
    UINT m_RefreshRate;

    CMediaType m_InputMediaType;

    CComPtr<ISubPicProvider> m_SubPicProvider;
    CComPtr<ISubPicAllocator> m_pAllocator;
    CComPtr<ISubPicQueue> m_pSubPicQueue;

    bool m_bDeviceResetRequested;
    bool m_bPendingResetDevice;

    void AlphaBltSubPic(const CRect& windowRect, const CRect& videoRect, SubPicDesc* pTarget = nullptr);

    XForm m_xform;
    void Transform(CRect r, Vector v[4]);

public:
    CSubPicAllocatorPresenterImpl(HWND hWnd, HRESULT& hr, CString* _pError);
    virtual ~CSubPicAllocatorPresenterImpl();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // ISubPicAllocatorPresenter

    STDMETHODIMP CreateRenderer(IUnknown** ppRenderer) PURE;

    STDMETHODIMP_(SIZE) GetVideoSize(bool fCorrectAR = true);
    STDMETHODIMP_(SIZE) GetVisibleVideoSize() {
        return m_NativeVideoSize;
    };
    STDMETHODIMP_(void) SetPosition(RECT w, RECT v);
    STDMETHODIMP_(bool) Paint(bool fAll) PURE;

    STDMETHODIMP_(void) SetTime(REFERENCE_TIME rtNow);
    STDMETHODIMP_(void) SetSubtitleDelay(int delay_ms);
    STDMETHODIMP_(int) GetSubtitleDelay();
    STDMETHODIMP_(double) GetFPS();

    STDMETHODIMP_(void) SetSubPicProvider(ISubPicProvider* pSubPicProvider);
    STDMETHODIMP_(void) Invalidate(REFERENCE_TIME rtInvalidate = -1);

    STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size) { return E_NOTIMPL; }

    STDMETHODIMP_(bool) ResetDevice() { return false; }

    STDMETHODIMP_(bool) DisplayChange() { return false; }

    STDMETHODIMP SetVideoAngle(Vector v);
    STDMETHODIMP SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget) { return E_NOTIMPL; }
    STDMETHODIMP SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace) {
        if (!bScreenSpace) {
            return SetPixelShader(pSrcData, pTarget);
        }
        return E_NOTIMPL;
    }

    // ISubRenderOptions

    STDMETHODIMP GetBool(LPCSTR field, bool* value);
    STDMETHODIMP GetInt(LPCSTR field, int* value);
    STDMETHODIMP GetSize(LPCSTR field, SIZE* value);
    STDMETHODIMP GetRect(LPCSTR field, RECT* value);
    STDMETHODIMP GetUlonglong(LPCSTR field, ULONGLONG* value);
    STDMETHODIMP GetDouble(LPCSTR field, double* value);
    STDMETHODIMP GetString(LPCSTR field, LPWSTR* value, int* chars);
    STDMETHODIMP GetBin(LPCSTR field, LPVOID* value, int* size);
    STDMETHODIMP SetBool(LPCSTR field, bool value);
    STDMETHODIMP SetInt(LPCSTR field, int value);
    STDMETHODIMP SetSize(LPCSTR field, SIZE value);
    STDMETHODIMP SetRect(LPCSTR field, RECT value);
    STDMETHODIMP SetUlonglong(LPCSTR field, ULONGLONG value);
    STDMETHODIMP SetDouble(LPCSTR field, double value);
    STDMETHODIMP SetString(LPCSTR field, LPWSTR value, int chars);
    STDMETHODIMP SetBin(LPCSTR field, LPVOID value, int size);

    // ISubRenderConsumer

    STDMETHODIMP GetMerit(ULONG* plMerit) {
        CheckPointer(plMerit, E_POINTER);
        *plMerit = 4 << 16;
        return S_OK;
    }
    STDMETHODIMP Connect(ISubRenderProvider* subtitleRenderer);
    STDMETHODIMP Disconnect();
    STDMETHODIMP DeliverFrame(REFERENCE_TIME start, REFERENCE_TIME stop, LPVOID context, ISubRenderFrame* subtitleFrame);
};
