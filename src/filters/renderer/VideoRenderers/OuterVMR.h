/*
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

#pragma once

namespace DSObjects
{
    class COuterVMR9
        : public CUnknown
        , public IVideoWindow
        , public IBasicVideo2
        , public IVMRWindowlessControl
        , public IVMRffdshow9
        , public IVMRMixerBitmap9
    {
        CComPtr<IUnknown> m_pVMR;
        VMR9AlphaBitmap* m_pVMR9AlphaBitmap;
        CVMR9AllocatorPresenter* m_pAllocatorPresenter;

    public:

        COuterVMR9(const TCHAR* pName, LPUNKNOWN pUnk, VMR9AlphaBitmap* pVMR9AlphaBitmap, CVMR9AllocatorPresenter* pAllocatorPresenter) : CUnknown(pName, pUnk) {
            m_pVMR.CoCreateInstance(CLSID_VideoMixingRenderer9, GetOwner());
            m_pVMR9AlphaBitmap = pVMR9AlphaBitmap;
            m_pAllocatorPresenter = pAllocatorPresenter;
        }

        ~COuterVMR9() {
            m_pVMR = nullptr;
        }

        DECLARE_IUNKNOWN;
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv) {
            HRESULT hr;

            // Casimir666 : in renderless mode, do the inlaying in place of VMR
            if (riid == __uuidof(IVMRMixerBitmap9)) {
                return GetInterface((IVMRMixerBitmap9*)this, ppv);
            }

            hr = m_pVMR ? m_pVMR->QueryInterface(riid, ppv) : E_NOINTERFACE;
            if (m_pVMR && FAILED(hr)) {
                hr = m_pAllocatorPresenter ? m_pAllocatorPresenter->QueryInterface(riid, ppv) : E_NOINTERFACE;
                if (FAILED(hr)) {
                    if (riid == __uuidof(IVideoWindow)) {
                        return GetInterface((IVideoWindow*)this, ppv);
                    }
                    if (riid == __uuidof(IBasicVideo)) {
                        return GetInterface((IBasicVideo*)this, ppv);
                    }
                    if (riid == __uuidof(IBasicVideo2)) {
                        return GetInterface((IBasicVideo2*)this, ppv);
                    }
                    if (riid == __uuidof(IVMRffdshow9)) { // Support ffdshow queueing. We show ffdshow that this is patched MPC-HC.
                        return GetInterface((IVMRffdshow9*)this, ppv);
                    }
                    /*if (riid == __uuidof(IVMRWindowlessControl))
                    return GetInterface((IVMRWindowlessControl*)this, ppv);
                    */
                }
            }

            return SUCCEEDED(hr) ? hr : __super::NonDelegatingQueryInterface(riid, ppv);
        }

        // IVMRWindowlessControl

        STDMETHODIMP GetNativeVideoSize(LONG* lpWidth, LONG* lpHeight, LONG* lpARWidth, LONG* lpARHeight);

        STDMETHODIMP GetMinIdealVideoSize(LONG* lpWidth, LONG* lpHeight) { return E_NOTIMPL; }
        STDMETHODIMP GetMaxIdealVideoSize(LONG* lpWidth, LONG* lpHeight) { return E_NOTIMPL; }
        STDMETHODIMP SetVideoPosition(const LPRECT lpSRCRect, const LPRECT lpDSTRect) { return E_NOTIMPL; }
        STDMETHODIMP GetVideoPosition(LPRECT lpSRCRect, LPRECT lpDSTRect);

        STDMETHODIMP GetAspectRatioMode(DWORD* lpAspectRatioMode);
        STDMETHODIMP SetAspectRatioMode(DWORD AspectRatioMode) { return E_NOTIMPL; }
        STDMETHODIMP SetVideoClippingWindow(HWND hwnd) { return E_NOTIMPL; }
        STDMETHODIMP RepaintVideo(HWND hwnd, HDC hdc) { return E_NOTIMPL; }
        STDMETHODIMP DisplayModeChanged() { return E_NOTIMPL; }
        STDMETHODIMP GetCurrentImage(BYTE** lpDib) { return E_NOTIMPL; }
        STDMETHODIMP SetBorderColor(COLORREF Clr) { return E_NOTIMPL; }
        STDMETHODIMP GetBorderColor(COLORREF* lpClr) { return E_NOTIMPL; }
        STDMETHODIMP SetColorKey(COLORREF Clr) { return E_NOTIMPL; }
        STDMETHODIMP GetColorKey(COLORREF* lpClr) { return E_NOTIMPL; }

        // IVideoWindow
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) { return E_NOTIMPL; }
        STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) { return E_NOTIMPL; }
        STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
            return E_NOTIMPL;
        }
        STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams,
                            VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
            return E_NOTIMPL;
        }
        STDMETHODIMP put_Caption(BSTR strCaption) { return E_NOTIMPL; }
        STDMETHODIMP get_Caption(BSTR* strCaption) { return E_NOTIMPL; }
        STDMETHODIMP put_WindowStyle(long WindowStyle) { return E_NOTIMPL; }
        STDMETHODIMP get_WindowStyle(long* WindowStyle) { return E_NOTIMPL; }
        STDMETHODIMP put_WindowStyleEx(long WindowStyleEx) { return E_NOTIMPL; }
        STDMETHODIMP get_WindowStyleEx(long* WindowStyleEx) { return E_NOTIMPL; }
        STDMETHODIMP put_AutoShow(long AutoShow) { return E_NOTIMPL; }
        STDMETHODIMP get_AutoShow(long* AutoShow) { return E_NOTIMPL; }
        STDMETHODIMP put_WindowState(long WindowState) { return E_NOTIMPL; }
        STDMETHODIMP get_WindowState(long* WindowState) { return E_NOTIMPL; }
        STDMETHODIMP put_BackgroundPalette(long BackgroundPalette) { return E_NOTIMPL; }
        STDMETHODIMP get_BackgroundPalette(long* pBackgroundPalette) { return E_NOTIMPL; }
        STDMETHODIMP put_Visible(long Visible) { return E_NOTIMPL; }
        STDMETHODIMP get_Visible(long* pVisible) { return E_NOTIMPL; }
        STDMETHODIMP put_Left(long Left) { return E_NOTIMPL; }
        STDMETHODIMP get_Left(long* pLeft) { return E_NOTIMPL; }
        STDMETHODIMP put_Width(long Width) { return E_NOTIMPL; }
        STDMETHODIMP get_Width(long* pWidth);

        STDMETHODIMP put_Top(long Top) { return E_NOTIMPL; }
        STDMETHODIMP get_Top(long* pTop) { return E_NOTIMPL; }
        STDMETHODIMP put_Height(long Height) { return E_NOTIMPL; }
        STDMETHODIMP get_Height(long* pHeight);

        STDMETHODIMP put_Owner(OAHWND Owner) { return E_NOTIMPL; }
        STDMETHODIMP get_Owner(OAHWND* Owner) { return E_NOTIMPL; }
        STDMETHODIMP put_MessageDrain(OAHWND Drain) { return E_NOTIMPL; }
        STDMETHODIMP get_MessageDrain(OAHWND* Drain) { return E_NOTIMPL; }
        STDMETHODIMP get_BorderColor(long* Color) { return E_NOTIMPL; }
        STDMETHODIMP put_BorderColor(long Color) { return E_NOTIMPL; }
        STDMETHODIMP get_FullScreenMode(long* FullScreenMode) { return E_NOTIMPL; }
        STDMETHODIMP put_FullScreenMode(long FullScreenMode) { return E_NOTIMPL; }
        STDMETHODIMP SetWindowForeground(long Focus) { return E_NOTIMPL; }
        STDMETHODIMP NotifyOwnerMessage(OAHWND hwnd, long uMsg, LONG_PTR wParam, LONG_PTR lParam) {
            return E_NOTIMPL;
        }
        STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height) { return E_NOTIMPL; }
        STDMETHODIMP GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) {
            return E_NOTIMPL;
        }
        STDMETHODIMP GetMinIdealImageSize(long* pWidth, long* pHeight) { return E_NOTIMPL; }
        STDMETHODIMP GetMaxIdealImageSize(long* pWidth, long* pHeight) { return E_NOTIMPL; }
        STDMETHODIMP GetRestorePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight) { return E_NOTIMPL; }
        STDMETHODIMP HideCursor(long HideCursor) { return E_NOTIMPL; }
        STDMETHODIMP IsCursorHidden(long* CursorHidden) { return E_NOTIMPL; }

        // IBasicVideo2
        STDMETHODIMP get_AvgTimePerFrame(REFTIME* pAvgTimePerFrame) { return E_NOTIMPL; }
        STDMETHODIMP get_BitRate(long* pBitRate) { return E_NOTIMPL; }
        STDMETHODIMP get_BitErrorRate(long* pBitErrorRate) { return E_NOTIMPL; }
        STDMETHODIMP get_VideoWidth(long* pVideoWidth) { return E_NOTIMPL; }
        STDMETHODIMP get_VideoHeight(long* pVideoHeight) { return E_NOTIMPL; }
        STDMETHODIMP put_SourceLeft(long SourceLeft) { return E_NOTIMPL; }
        STDMETHODIMP get_SourceLeft(long* pSourceLeft) { return E_NOTIMPL; }
        STDMETHODIMP put_SourceWidth(long SourceWidth) { return E_NOTIMPL; }
        STDMETHODIMP get_SourceWidth(long* pSourceWidth) { return E_NOTIMPL; }
        STDMETHODIMP put_SourceTop(long SourceTop) { return E_NOTIMPL; }
        STDMETHODIMP get_SourceTop(long* pSourceTop) { return E_NOTIMPL; }
        STDMETHODIMP put_SourceHeight(long SourceHeight) { return E_NOTIMPL; }
        STDMETHODIMP get_SourceHeight(long* pSourceHeight) { return E_NOTIMPL; }
        STDMETHODIMP put_DestinationLeft(long DestinationLeft) { return E_NOTIMPL; }
        STDMETHODIMP get_DestinationLeft(long* pDestinationLeft) { return E_NOTIMPL; }
        STDMETHODIMP put_DestinationWidth(long DestinationWidth) { return E_NOTIMPL; }
        STDMETHODIMP get_DestinationWidth(long* pDestinationWidth) { return E_NOTIMPL; }
        STDMETHODIMP put_DestinationTop(long DestinationTop) { return E_NOTIMPL; }
        STDMETHODIMP get_DestinationTop(long* pDestinationTop) { return E_NOTIMPL; }
        STDMETHODIMP put_DestinationHeight(long DestinationHeight) { return E_NOTIMPL; }
        STDMETHODIMP get_DestinationHeight(long* pDestinationHeight) { return E_NOTIMPL; }
        STDMETHODIMP SetSourcePosition(long Left, long Top, long Width, long Height) { return E_NOTIMPL; }
        STDMETHODIMP GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);

        STDMETHODIMP SetDefaultSourcePosition() { return E_NOTIMPL; }
        STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height) { return E_NOTIMPL; }
        STDMETHODIMP GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);

        STDMETHODIMP SetDefaultDestinationPosition() { return E_NOTIMPL; }
        STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight);

        // IVMRffdshow9
        STDMETHODIMP support_ffdshow() {
            queue_ffdshow_support = true;
            return S_OK;
        }

        STDMETHODIMP GetVideoPaletteEntries(long StartIndex, long Entries, long* pRetrieved, long* pPalette) {
            return E_NOTIMPL;
        }
        STDMETHODIMP GetCurrentImage(long* pBufferSize, long* pDIBImage) { return E_NOTIMPL; }
        STDMETHODIMP IsUsingDefaultSource() { return E_NOTIMPL; }
        STDMETHODIMP IsUsingDefaultDestination() { return E_NOTIMPL; }

        STDMETHODIMP GetPreferredAspectRatio(long* plAspectX, long* plAspectY);

        // IVMRMixerBitmap9
        STDMETHODIMP GetAlphaBitmapParameters(VMR9AlphaBitmap* pBmpParms);
        STDMETHODIMP SetAlphaBitmap(const VMR9AlphaBitmap*  pBmpParms);
        STDMETHODIMP UpdateAlphaBitmapParameters(const VMR9AlphaBitmap* pBmpParms);
    };
}
