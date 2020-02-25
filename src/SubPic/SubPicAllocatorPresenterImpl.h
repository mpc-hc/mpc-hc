/*
 * (C) 2003-2006 Gabest
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

#pragma once

#include <atlbase.h>
#include <atlcoll.h>
#include <condition_variable>
#include "ISubPic.h"
#include "CoordGeom.h"
#include "SubRenderIntf.h"

class CSubPicAllocatorPresenterImpl
    : public CUnknown
    , public CCritSec
    , public ISubPicAllocatorPresenter2
    , public ISubRenderConsumer2
{
private:
    CCritSec m_csSubPicProvider;

protected:
    HWND m_hWnd;
    REFERENCE_TIME m_rtSubtitleDelay;

    CSize m_maxSubtitleTextureSize;
    CSize m_curSubtitleTextureSize;
    CSize m_nativeVideoSize, m_aspectRatio;
    CRect m_videoRect, m_windowRect;

    REFERENCE_TIME m_rtNow;
    double m_fps;
    UINT m_refreshRate;

    CMediaType m_inputMediaType;

    CComPtr<ISubPicProvider> m_pSubPicProvider;
    CComPtr<ISubPicAllocator> m_pAllocator;
    CComPtr<ISubPicQueue> m_pSubPicQueue;

    std::condition_variable m_condAllocatorReady;

    bool m_bDeviceResetRequested;
    bool m_bPendingResetDevice;

    enum SubtitleTextureLimit {
        STATIC, VIDEO, DESKTOP
    };
    SubtitleTextureLimit m_SubtitleTextureLimit;
    void InitMaxSubtitleTextureSize(int maxSize, CSize desktopSize);

    HRESULT AlphaBltSubPic(const CRect& windowRect,
                           const CRect& videoRect,
                           SubPicDesc* pTarget = nullptr,
                           const double videoStretchFactor = 1.0,
                           int xOffsetInPixels = 0);

    void UpdateXForm();
    HRESULT CreateDIBFromSurfaceData(D3DSURFACE_DESC desc, D3DLOCKED_RECT r, BYTE* lpDib) const;

    Vector m_defaultVideoAngle, m_videoAngle;
    bool m_bDefaultVideoAngleSwitchAR;
    XForm m_xform;
    void Transform(CRect r, Vector v[4]);

public:
    CSubPicAllocatorPresenterImpl(HWND hWnd, HRESULT& hr, CString* _pError);
    virtual ~CSubPicAllocatorPresenterImpl();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    STDMETHODIMP CreateRenderer(IUnknown** ppRenderer) PURE;

    STDMETHODIMP_(void) SetVideoSize(CSize szVideo, CSize szAspectRatio = CSize(0, 0));
    STDMETHODIMP_(SIZE) GetVideoSize(bool bCorrectAR = true) const;
    STDMETHODIMP_(SIZE) GetVisibleVideoSize() const {
        return m_nativeVideoSize;
    };
    STDMETHODIMP_(void) SetPosition(RECT w, RECT v);
    STDMETHODIMP_(bool) Paint(bool bAll) PURE;

    STDMETHODIMP_(void) SetTime(REFERENCE_TIME rtNow);
    STDMETHODIMP_(void) SetSubtitleDelay(int delayMs);
    STDMETHODIMP_(int) GetSubtitleDelay() const;
    STDMETHODIMP_(double) GetFPS() const;

    STDMETHODIMP_(void) SetSubPicProvider(ISubPicProvider* pSubPicProvider);
    STDMETHODIMP_(void) Invalidate(REFERENCE_TIME rtInvalidate = -1);

    STDMETHODIMP GetDIB(BYTE* lpDib, DWORD* size) { return E_NOTIMPL; }

    STDMETHODIMP_(bool) ResetDevice() { return false; }

    STDMETHODIMP_(bool) DisplayChange() { return false; }

    STDMETHODIMP SetVideoAngle(Vector v);
    STDMETHODIMP SetPixelShader(LPCSTR pSrcData, LPCSTR pTarget) { return E_NOTIMPL; }

    // ISubPicAllocatorPresenter2

    STDMETHODIMP SetPixelShader2(LPCSTR pSrcData, LPCSTR pTarget, bool bScreenSpace) {
        if (!bScreenSpace) {
            return SetPixelShader(pSrcData, pTarget);
        }
        return E_NOTIMPL;
    }

    STDMETHODIMP SetIsRendering(bool bIsRendering) { return E_NOTIMPL; }

    STDMETHODIMP SetDefaultVideoAngle(Vector v);

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

    // ISubRenderConsumer2

    STDMETHODIMP Clear(REFERENCE_TIME clearNewerThan = 0);
};
