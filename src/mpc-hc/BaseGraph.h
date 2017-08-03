/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2017 see Authors.txt
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

#include "IGraphBuilder2.h"


class CPlayerWindow : public CWnd
{
public:
    CPlayerWindow() {}

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()
};

enum engine_t {
    DirectShow = 0,
    RealMedia,
    QuickTime,
    ShockWave
};

interface __declspec(uuid("B110CDE5-6331-4118-8AAF-A870D6F7E2E4"))
    IGraphEngine :
    public IUnknown
{
    STDMETHOD_(engine_t, GetEngine)() PURE;
};

enum {
    EC_BG_AUDIO_CHANGED = EC_USER + 1,
    EC_BG_ERROR
};

class CBaseGraph
    : public CUnknown
    , public IGraphBuilder2
    , public IMediaControl
    , public IMediaEventEx
    , public IMediaSeeking
    , public IVideoWindow
    , public IBasicVideo
    , public IBasicAudio
    , public IAMOpenProgress
    , public IGraphEngine
{
    OAHWND m_hNotifyWnd;
    long m_lNotifyMsg;
    LONG_PTR m_lNotifyInstData;

    struct GMSG {
        long m_lEventCode;
        LONG_PTR m_lParam1, m_lParam2;
    };
    CList<GMSG> m_msgqueue;

protected:
    void ClearMessageQueue();

public:
    CBaseGraph();
    virtual ~CBaseGraph();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    void NotifyEvent(long lEventCode, LONG_PTR lParam1 = 0, LONG_PTR lParam2 = 0);

protected:
    // IDispatch
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId);
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

    // IFilterGraph
    STDMETHODIMP AddFilter(IBaseFilter* pFilter, LPCWSTR pName);
    STDMETHODIMP RemoveFilter(IBaseFilter* pFilter);
    STDMETHODIMP EnumFilters(IEnumFilters** ppEnum);
    STDMETHODIMP FindFilterByName(LPCWSTR pName, IBaseFilter** ppFilter);
    STDMETHODIMP ConnectDirect(IPin* ppinOut, IPin* ppinIn, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP Reconnect(IPin* ppin);
    STDMETHODIMP Disconnect(IPin* ppin);
    STDMETHODIMP SetDefaultSyncSource();

    // IGraphBuilder
    STDMETHODIMP Connect(IPin* ppinOut, IPin* ppinIn);
    STDMETHODIMP Render(IPin* ppinOut);
    STDMETHODIMP RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList);
    STDMETHODIMP AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter);
    STDMETHODIMP SetLogFile(DWORD_PTR hFile);
    STDMETHODIMP Abort();
    STDMETHODIMP ShouldOperationContinue();

    // IFilterGraph2
    STDMETHODIMP AddSourceFilterForMoniker(IMoniker* pMoniker, IBindCtx* pCtx, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter);
    STDMETHODIMP ReconnectEx(IPin* ppin, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP RenderEx(IPin* pPinOut, DWORD dwFlags, DWORD* pvContext);

    // IGraphBuilder2
    STDMETHODIMP IsPinDirection(IPin* pPin, PIN_DIRECTION dir);
    STDMETHODIMP IsPinConnected(IPin* pPin);
    STDMETHODIMP ConnectFilter(IBaseFilter* pBF, IPin* pPinIn);
    STDMETHODIMP ConnectFilter(IPin* pPinOut, IBaseFilter* pBF);
    STDMETHODIMP ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP NukeDownstream(IUnknown* pUnk);
    STDMETHODIMP FindInterface(REFIID iid, void** ppv, BOOL bRemove);
    STDMETHODIMP AddToROT();
    STDMETHODIMP RemoveFromROT();

    // IMediaControl
    STDMETHODIMP Run();
    STDMETHODIMP Pause();
    STDMETHODIMP Stop();
    STDMETHODIMP GetState(LONG msTimeout, OAFilterState* pfs);
    STDMETHODIMP RenderFile(BSTR strFilename);
    STDMETHODIMP AddSourceFilter(BSTR strFilename, IDispatch** ppUnk);
    STDMETHODIMP get_FilterCollection(IDispatch** ppUnk);
    STDMETHODIMP get_RegFilterCollection(IDispatch** ppUnk);
    STDMETHODIMP StopWhenReady();

    // IMediaEvent
    STDMETHODIMP GetEventHandle(OAEVENT* hEvent);
    STDMETHODIMP GetEvent(long* lEventCode, LONG_PTR* lParam1, LONG_PTR* lParam2, long msTimeout);
    STDMETHODIMP WaitForCompletion(long msTimeout, long* pEvCode);
    STDMETHODIMP CancelDefaultHandling(long lEvCode);
    STDMETHODIMP RestoreDefaultHandling(long lEvCode);
    STDMETHODIMP FreeEventParams(long lEvCode, LONG_PTR lParam1, LONG_PTR lParam2);

    // IMediaEventEx
    STDMETHODIMP SetNotifyWindow(OAHWND hwnd, long lMsg, LONG_PTR lInstanceData);
    STDMETHODIMP SetNotifyFlags(long lNoNotifyFlags);
    STDMETHODIMP GetNotifyFlags(long* lplNoNotifyFlags);

    // IMediaSeeking
    STDMETHODIMP GetCapabilities(DWORD* pCapabilities);
    STDMETHODIMP CheckCapabilities(DWORD* pCapabilities);
    STDMETHODIMP IsFormatSupported(const GUID* pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID* pFormat);
    STDMETHODIMP GetTimeFormat(GUID* pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID* pFormat);
    STDMETHODIMP SetTimeFormat(const GUID* pFormat);
    STDMETHODIMP GetDuration(LONGLONG* pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG* pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
    STDMETHODIMP ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat);
    STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags);
    STDMETHODIMP GetPositions(LONGLONG* pCurrent, LONGLONG* pStop);
    STDMETHODIMP GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest);
    STDMETHODIMP SetRate(double dRate);
    STDMETHODIMP GetRate(double* pdRate);
    STDMETHODIMP GetPreroll(LONGLONG* pllPreroll);

    // IVideoWindow
    STDMETHODIMP put_Caption(BSTR strCaption);
    STDMETHODIMP get_Caption(BSTR* strCaption);
    STDMETHODIMP put_WindowStyle(long WindowStyle);
    STDMETHODIMP get_WindowStyle(long* WindowStyle);
    STDMETHODIMP put_WindowStyleEx(long WindowStyleEx);
    STDMETHODIMP get_WindowStyleEx(long* WindowStyleEx);
    STDMETHODIMP put_AutoShow(long AutoShow);
    STDMETHODIMP get_AutoShow(long* AutoShow);
    STDMETHODIMP put_WindowState(long WindowState);
    STDMETHODIMP get_WindowState(long* WindowState);
    STDMETHODIMP put_BackgroundPalette(long BackgroundPalette);
    STDMETHODIMP get_BackgroundPalette(long* pBackgroundPalette);
    STDMETHODIMP put_Visible(long Visible);
    STDMETHODIMP get_Visible(long* pVisible);
    STDMETHODIMP put_Left(long Left);
    STDMETHODIMP get_Left(long* pLeft);
    STDMETHODIMP put_Width(long Width);
    STDMETHODIMP get_Width(long* pWidth);
    STDMETHODIMP put_Top(long Top);
    STDMETHODIMP get_Top(long* pTop);
    STDMETHODIMP put_Height(long Height);
    STDMETHODIMP get_Height(long* pHeight);
    STDMETHODIMP put_Owner(OAHWND Owner);
    STDMETHODIMP get_Owner(OAHWND* Owner);
    STDMETHODIMP put_MessageDrain(OAHWND Drain);
    STDMETHODIMP get_MessageDrain(OAHWND* Drain);
    STDMETHODIMP get_BorderColor(long* Color);
    STDMETHODIMP put_BorderColor(long Color);
    STDMETHODIMP get_FullScreenMode(long* FullScreenMode);
    STDMETHODIMP put_FullScreenMode(long FullScreenMode);
    STDMETHODIMP SetWindowForeground(long Focus);
    STDMETHODIMP NotifyOwnerMessage(OAHWND hwnd, long uMsg, LONG_PTR wParam, LONG_PTR lParam);
    STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height);
    STDMETHODIMP GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);
    STDMETHODIMP GetMinIdealImageSize(long* pWidth, long* pHeight);
    STDMETHODIMP GetMaxIdealImageSize(long* pWidth, long* pHeight);
    STDMETHODIMP GetRestorePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);
    STDMETHODIMP HideCursor(long HideCursor);
    STDMETHODIMP IsCursorHidden(long* CursorHidden);

    // IBasicVideo
    STDMETHODIMP get_AvgTimePerFrame(REFTIME* pAvgTimePerFrame);
    STDMETHODIMP get_BitRate(long* pBitRate);
    STDMETHODIMP get_BitErrorRate(long* pBitErrorRate);
    STDMETHODIMP get_VideoWidth(long* pVideoWidth);
    STDMETHODIMP get_VideoHeight(long* pVideoHeight);
    STDMETHODIMP put_SourceLeft(long SourceLeft);
    STDMETHODIMP get_SourceLeft(long* pSourceLeft);
    STDMETHODIMP put_SourceWidth(long SourceWidth);
    STDMETHODIMP get_SourceWidth(long* pSourceWidth);
    STDMETHODIMP put_SourceTop(long SourceTop);
    STDMETHODIMP get_SourceTop(long* pSourceTop);
    STDMETHODIMP put_SourceHeight(long SourceHeight);
    STDMETHODIMP get_SourceHeight(long* pSourceHeight);
    STDMETHODIMP put_DestinationLeft(long DestinationLeft);
    STDMETHODIMP get_DestinationLeft(long* pDestinationLeft);
    STDMETHODIMP put_DestinationWidth(long DestinationWidth);
    STDMETHODIMP get_DestinationWidth(long* pDestinationWidth);
    STDMETHODIMP put_DestinationTop(long DestinationTop);
    STDMETHODIMP get_DestinationTop(long* pDestinationTop);
    STDMETHODIMP put_DestinationHeight(long DestinationHeight);
    STDMETHODIMP get_DestinationHeight(long* pDestinationHeight);
    STDMETHODIMP SetSourcePosition(long Left, long Top, long Width, long Height);
    STDMETHODIMP GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);
    STDMETHODIMP SetDefaultSourcePosition();
    STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height);
    STDMETHODIMP GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight);
    STDMETHODIMP SetDefaultDestinationPosition();
    STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight);
    STDMETHODIMP GetVideoPaletteEntries(long StartIndex, long Entries, long* pRetrieved, long* pPalette);
    STDMETHODIMP GetCurrentImage(long* pBufferSize, long* pDIBImage);
    STDMETHODIMP IsUsingDefaultSource();
    STDMETHODIMP IsUsingDefaultDestination();

    // IBasicAudio
    STDMETHODIMP put_Volume(long lVolume);
    STDMETHODIMP get_Volume(long* plVolume);
    STDMETHODIMP put_Balance(long lBalance);
    STDMETHODIMP get_Balance(long* plBalance);

    // IAMOpenProgress
    STDMETHODIMP QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent);
    STDMETHODIMP AbortOperation();

    // IGraphEngine
    STDMETHODIMP_(engine_t) GetEngine();
};
