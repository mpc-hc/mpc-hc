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

#include "stdafx.h"
#include "BaseGraph.h"
#include "DSUtil.h"


//
// CPlayerWindow
//

BOOL CPlayerWindow::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs)) {
        return FALSE;
    }

    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       ::LoadCursor(nullptr, IDC_HAND), nullptr, nullptr);

    return TRUE;
}

BEGIN_MESSAGE_MAP(CPlayerWindow, CWnd)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CPlayerWindow::OnEraseBkgnd(CDC* pDC)
{
    for (CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow()) {
        if (!pChild->IsWindowVisible()) {
            continue;
        }

        CRect r;
        pChild->GetClientRect(&r);
        pChild->MapWindowPoints(this, &r);
        pDC->ExcludeClipRect(&r);
    }

    CRect r;
    GetClientRect(&r);
    pDC->FillSolidRect(&r, 0);

    return TRUE;
}

//
// CBaseGraph
//

CBaseGraph::CBaseGraph()
    : CUnknown(NAME("CBaseGraph"), nullptr)
    , m_hNotifyWnd(0)
    , m_lNotifyMsg(0)
    , m_lNotifyInstData(0)
{
}

CBaseGraph::~CBaseGraph()
{
}

STDMETHODIMP CBaseGraph::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(IFilterGraph)
        QI(IGraphBuilder)
        QI(IFilterGraph2)
        QI(IGraphBuilder2)
        QI(IMediaControl)
        QI(IMediaSeeking)
        QI(IMediaEventEx)
        QI(IVideoWindow)
        QI(IBasicVideo)
        QI(IBasicAudio)
        QI(IAMOpenProgress)
        QI(IGraphEngine)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

void CBaseGraph::ClearMessageQueue()
{
    while (!m_msgqueue.IsEmpty()) {
        GMSG msg = m_msgqueue.RemoveHead();
        FreeEventParams(msg.m_lEventCode, msg.m_lParam1, msg.m_lParam2);
    }
}

void CBaseGraph::NotifyEvent(long lEventCode, LONG_PTR lParam1, LONG_PTR lParam2)
{
    if (!m_hNotifyWnd) {
        return;
    }

    GMSG msg;
    msg.m_lEventCode = lEventCode;
    msg.m_lParam1 = lParam1;
    msg.m_lParam2 = lParam2;
    m_msgqueue.AddTail(msg);

    PostMessage((HWND)m_hNotifyWnd, m_lNotifyMsg, 0, m_lNotifyInstData);
}

// IDispatch
STDMETHODIMP CBaseGraph::GetTypeInfoCount(UINT* pctinfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    return E_NOTIMPL;
}

// IFilterGraph
STDMETHODIMP CBaseGraph::AddFilter(IBaseFilter* pFilter, LPCWSTR pName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::RemoveFilter(IBaseFilter* pFilter)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::EnumFilters(IEnumFilters** ppEnum)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::FindFilterByName(LPCWSTR pName, IBaseFilter** ppFilter)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::ConnectDirect(IPin* ppinOut, IPin* ppinIn, const AM_MEDIA_TYPE* pmt)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::Reconnect(IPin* ppin)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::Disconnect(IPin* ppin)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetDefaultSyncSource()
{
    return E_NOTIMPL;
}

// IGraphBuilder
STDMETHODIMP CBaseGraph::Connect(IPin* ppinOut, IPin* ppinIn)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::Render(IPin* ppinOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
    *ppFilter = nullptr;
    return RenderFile(lpcwstrFileName, nullptr);
}//E_NOTIMPL;}

STDMETHODIMP CBaseGraph::SetLogFile(DWORD_PTR hFile)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::Abort()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::ShouldOperationContinue()
{
    return E_NOTIMPL;
}

// IFilterGraph2
STDMETHODIMP CBaseGraph::AddSourceFilterForMoniker(IMoniker* pMoniker, IBindCtx* pCtx, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::ReconnectEx(IPin* ppin, const AM_MEDIA_TYPE* pmt)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::RenderEx(IPin* pPinOut, DWORD dwFlags, DWORD* pvContext)
{
    return E_NOTIMPL;
}

// IGraphBuilder2
STDMETHODIMP CBaseGraph::IsPinDirection(IPin* pPin, PIN_DIRECTION dir)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::IsPinConnected(IPin* pPin)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::ConnectFilter(IBaseFilter* pBF, IPin* pPinIn)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::ConnectFilter(IPin* pPinOut, IBaseFilter* pBF)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::ConnectFilterDirect(IPin* pPinOut, IBaseFilter* pBF, const AM_MEDIA_TYPE* pmt)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::NukeDownstream(IUnknown* pUnk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::FindInterface(REFIID iid, void** ppv, BOOL bRemove)
{
    return QueryInterface(iid, ppv);
}

STDMETHODIMP CBaseGraph::AddToROT()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::RemoveFromROT()
{
    return E_NOTIMPL;
}

// IMediaControl
STDMETHODIMP CBaseGraph::Run()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::Pause()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::Stop()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetState(LONG msTimeout, OAFilterState* pfs)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::RenderFile(BSTR strFilename)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::AddSourceFilter(BSTR strFilename, IDispatch** ppUnk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_FilterCollection(IDispatch** ppUnk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_RegFilterCollection(IDispatch** ppUnk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::StopWhenReady()
{
    return Stop();
}

// IMediaEvent
STDMETHODIMP CBaseGraph::GetEventHandle(OAEVENT* hEvent)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetEvent(long* lEventCode, LONG_PTR* lParam1, LONG_PTR* lParam2, long msTimeout)
{
    if (m_msgqueue.IsEmpty()) {
        return E_FAIL;
    }

    GMSG msg = m_msgqueue.RemoveHead();
    if (lEventCode) {
        *lEventCode = msg.m_lEventCode;
    }
    if (lParam1) {
        *lParam1 = msg.m_lParam1;
    }
    if (lParam2) {
        *lParam2 = msg.m_lParam2;
    }

    return S_OK;
}

STDMETHODIMP CBaseGraph::WaitForCompletion(long msTimeout, long* pEvCode)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::CancelDefaultHandling(long lEvCode)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::RestoreDefaultHandling(long lEvCode)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::FreeEventParams(long lEvCode, LONG_PTR lParam1, LONG_PTR lParam2)
{
    if (EC_BG_ERROR == lEvCode) {
        if (lParam1) {
            CoTaskMemFree((void*)lParam1);
        }
    }

    return S_OK;
}

// IMediaEventEx
STDMETHODIMP CBaseGraph::SetNotifyWindow(OAHWND hwnd, long lMsg, LONG_PTR lInstanceData)
{
    m_hNotifyWnd = hwnd;
    m_lNotifyMsg = lMsg;
    m_lNotifyInstData = lInstanceData;

    if (!IsWindow((HWND)m_hNotifyWnd)) {
        m_hNotifyWnd = 0;
        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CBaseGraph::SetNotifyFlags(long lNoNotifyFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetNotifyFlags(long* lplNoNotifyFlags)
{
    return E_NOTIMPL;
}

// IMediaSeeking
STDMETHODIMP CBaseGraph::GetCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);

    *pCapabilities = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanGetCurrentPos | AM_SEEKING_CanGetDuration;

    return S_OK;
}

STDMETHODIMP CBaseGraph::CheckCapabilities(DWORD* pCapabilities)
{
    CheckPointer(pCapabilities, E_POINTER);

    if (*pCapabilities == 0) {
        return S_OK;
    }

    DWORD caps;
    GetCapabilities(&caps);

    DWORD caps2 = caps & *pCapabilities;

    return caps2 == 0 ? E_FAIL : caps2 == *pCapabilities ? S_OK : S_FALSE;
}

STDMETHODIMP CBaseGraph::IsFormatSupported(const GUID* pFormat)
{
    return !pFormat ? E_POINTER : *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

STDMETHODIMP CBaseGraph::QueryPreferredFormat(GUID* pFormat)
{
    return GetTimeFormat(pFormat);
}

STDMETHODIMP CBaseGraph::GetTimeFormat(GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);

    *pFormat = TIME_FORMAT_MEDIA_TIME;

    return S_OK;
}

STDMETHODIMP CBaseGraph::IsUsingTimeFormat(const GUID* pFormat)
{
    return IsFormatSupported(pFormat);
}

STDMETHODIMP CBaseGraph::SetTimeFormat(const GUID* pFormat)
{
    return S_OK == IsFormatSupported(pFormat) ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CBaseGraph::GetDuration(LONGLONG* pDuration)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetStopPosition(LONGLONG* pStop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetCurrentPosition(LONGLONG* pCurrent)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::ConvertTimeFormat(LONGLONG* pTarget, const GUID* pTargetFormat, LONGLONG Source, const GUID* pSourceFormat)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetPositions(LONGLONG* pCurrent, LONGLONG* pStop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetAvailable(LONGLONG* pEarliest, LONGLONG* pLatest)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetRate(double dRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetRate(double* pdRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetPreroll(LONGLONG* pllPreroll)
{
    return E_NOTIMPL;
}

// IVideoWindow
STDMETHODIMP CBaseGraph::put_Caption(BSTR strCaption)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Caption(BSTR* strCaption)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_WindowStyle(long WindowStyle)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_WindowStyle(long* WindowStyle)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_WindowStyleEx(long WindowStyleEx)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_WindowStyleEx(long* WindowStyleEx)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_AutoShow(long AutoShow)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_AutoShow(long* AutoShow)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_WindowState(long WindowState)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_WindowState(long* WindowState)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_BackgroundPalette(long BackgroundPalette)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_BackgroundPalette(long* pBackgroundPalette)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_Visible(long Visible)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Visible(long* pVisible)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_Left(long Left)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Left(long* pLeft)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_Width(long Width)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Width(long* pWidth)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_Top(long Top)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Top(long* pTop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_Height(long Height)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Height(long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_Owner(OAHWND Owner)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Owner(OAHWND* Owner)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_MessageDrain(OAHWND Drain)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_MessageDrain(OAHWND* Drain)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_BorderColor(long* Color)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_BorderColor(long Color)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_FullScreenMode(long* FullScreenMode)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_FullScreenMode(long FullScreenMode)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetWindowForeground(long Focus)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::NotifyOwnerMessage(OAHWND hwnd, long uMsg, LONG_PTR wParam, LONG_PTR lParam)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetWindowPosition(long Left, long Top, long Width, long Height)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetWindowPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetMinIdealImageSize(long* pWidth, long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetMaxIdealImageSize(long* pWidth, long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetRestorePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::HideCursor(long HideCursor)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::IsCursorHidden(long* CursorHidden)
{
    return E_NOTIMPL;
}

// IBasicVideo
STDMETHODIMP CBaseGraph::get_AvgTimePerFrame(REFTIME* pAvgTimePerFrame)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_BitRate(long* pBitRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_BitErrorRate(long* pBitErrorRate)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_VideoWidth(long* pVideoWidth)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_VideoHeight(long* pVideoHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_SourceLeft(long SourceLeft)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_SourceLeft(long* pSourceLeft)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_SourceWidth(long SourceWidth)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_SourceWidth(long* pSourceWidth)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_SourceTop(long SourceTop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_SourceTop(long* pSourceTop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_SourceHeight(long SourceHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_SourceHeight(long* pSourceHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_DestinationLeft(long DestinationLeft)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_DestinationLeft(long* pDestinationLeft)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_DestinationWidth(long DestinationWidth)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_DestinationWidth(long* pDestinationWidth)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_DestinationTop(long DestinationTop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_DestinationTop(long* pDestinationTop)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_DestinationHeight(long DestinationHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_DestinationHeight(long* pDestinationHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetSourcePosition(long Left, long Top, long Width, long Height)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetSourcePosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetDefaultSourcePosition()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetDestinationPosition(long Left, long Top, long Width, long Height)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetDestinationPosition(long* pLeft, long* pTop, long* pWidth, long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::SetDefaultDestinationPosition()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetVideoSize(long* pWidth, long* pHeight)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetVideoPaletteEntries(long StartIndex, long Entries, long* pRetrieved, long* pPalette)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::GetCurrentImage(long* pBufferSize, long* pDIBImage)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::IsUsingDefaultSource()
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::IsUsingDefaultDestination()
{
    return E_NOTIMPL;
}

// IBasicAudio
STDMETHODIMP CBaseGraph::put_Volume(long lVolume)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Volume(long* plVolume)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::put_Balance(long lBalance)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::get_Balance(long* plBalance)
{
    return E_NOTIMPL;
}

// IAMOpenProgress
STDMETHODIMP CBaseGraph::QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBaseGraph::AbortOperation()
{
    return E_NOTIMPL;
}

// IGraphEngine
STDMETHODIMP_(engine_t) CBaseGraph::GetEngine()
{
    return DirectShow;
}
