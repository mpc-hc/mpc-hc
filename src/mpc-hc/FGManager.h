/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2017 see Authors.txt
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

#include "FGFilter.h"
#include "FGFilterLAV.h"
#include "IGraphBuilder2.h"

class CFGManager
    : public CUnknown
    , public IGraphBuilder2
    , public IGraphBuilderDeadEnd
    , public CCritSec
{
public:
    struct path_t {
        CLSID clsid;
        CString filter, pin;
    };

    class CStreamPath : public CAtlList<path_t>
    {
    public:
        void Append(IBaseFilter* pBF, IPin* pPin);
        bool Compare(const CStreamPath& path);
    };

    class CStreamDeadEnd : public CStreamPath
    {
    public:
        CAtlList<CMediaType> mts;
    };

private:
    CComPtr<IUnknown> m_pUnkInner;
    DWORD m_dwRegister;

    CStreamPath m_streampath;
    CAutoPtrArray<CStreamDeadEnd> m_deadends;

protected:
    CComPtr<IFilterMapper2> m_pFM;
    CInterfaceList<IUnknown, &IID_IUnknown> m_pUnks;
    CAtlList<CFGFilter*> m_source, m_transform, m_override;

    static bool CheckBytes(HANDLE hFile, CString chkbytes);

    HRESULT EnumSourceFilters(LPCWSTR lpcwstrFileName, CFGFilterList& fl);
    HRESULT AddSourceFilter(CFGFilter* pFGF, LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppBF);
    HRESULT Connect(IPin* pPinOut, IPin* pPinIn, bool bContinueRender);

    // IFilterGraph

    STDMETHODIMP AddFilter(IBaseFilter* pFilter, LPCWSTR pName);
    STDMETHODIMP RemoveFilter(IBaseFilter* pFilter);
    STDMETHODIMP EnumFilters(IEnumFilters** ppEnum);
    STDMETHODIMP FindFilterByName(LPCWSTR pName, IBaseFilter** ppFilter);
    STDMETHODIMP ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt);
    STDMETHODIMP Reconnect(IPin* ppin);
    STDMETHODIMP Disconnect(IPin* ppin);
    STDMETHODIMP SetDefaultSyncSource();

    // IGraphBuilder

    STDMETHODIMP Connect(IPin* pPinOut, IPin* pPinIn);
    STDMETHODIMP Render(IPin* pPinOut);
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

    // IGraphBuilderDeadEnd

    STDMETHODIMP_(size_t) GetCount();
    STDMETHODIMP GetDeadEnd(int iIndex, CAtlList<CStringW>& path, CAtlList<CMediaType>& mts);

public:
    CFGManager(LPCTSTR pName, LPUNKNOWN pUnk);
    virtual ~CFGManager();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
};

class CFGManagerCustom : public CFGManager
{
public:
    // IFilterGraph

    STDMETHODIMP AddFilter(IBaseFilter* pFilter, LPCWSTR pName);

public:
    CFGManagerCustom(LPCTSTR pName, LPUNKNOWN pUnk);
};

class CFGManagerPlayer : public CFGManagerCustom
{
protected:
    HWND m_hWnd;

    // IFilterGraph

    STDMETHODIMP ConnectDirect(IPin* pPinOut, IPin* pPinIn, const AM_MEDIA_TYPE* pmt);

public:
    CFGManagerPlayer(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd);
};

class CFGManagerDVD : public CFGManagerPlayer
{
protected:
    // IGraphBuilder

    STDMETHODIMP RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList);
    STDMETHODIMP AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter** ppFilter);

public:
    CFGManagerDVD(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd);
};

class CFGManagerCapture : public CFGManagerPlayer
{
public:
    CFGManagerCapture(LPCTSTR pName, LPUNKNOWN pUnk, HWND hWnd);
};

class CFGManagerMuxer : public CFGManagerCustom
{
public:
    CFGManagerMuxer(LPCTSTR pName, LPUNKNOWN pUnk);
};

//

class CFGAggregator : public CUnknown
{
protected:
    CComPtr<IUnknown> m_pUnkInner;

public:
    CFGAggregator(const CLSID& clsid, LPCTSTR pName, LPUNKNOWN pUnk, HRESULT& hr);
    virtual ~CFGAggregator();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
};
