/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014, 2017 see Authors.txt
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
#include "ShockwaveGraph.h"
#include "resource.h"
#include "DSUtil.h"
#include <cmath>
#include <Audiopolicy.h>
#include <Mmdeviceapi.h>

using namespace DSObjects;

CShockwaveGraph::CShockwaveGraph(HWND hParent, HRESULT& hr)
    : m_fs(State_Stopped)
    , m_fInitialVolume(1)
{
    hr = S_OK;

    if (!m_wndWindowFrame.Create(nullptr, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                 CRect(0, 0, 0, 0), CWnd::FromHandle(hParent), 0, nullptr)) {
        hr = E_FAIL;
        return;
    }

    if (!m_wndDestFrame.Create(nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               CRect(0, 0, 0, 0), &m_wndWindowFrame, 0)) {
        hr = E_FAIL;
        return;
    }
    m_wndDestFrame.put_BackgroundColor(0);

    CComPtr<IMMDeviceEnumerator> pDeviceEnumerator;
    if (SUCCEEDED(pDeviceEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator)))) {
        CComPtr<IMMDevice> pDevice;
        if (SUCCEEDED(pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice))) {
            CComPtr<IAudioSessionManager> pSessionManager;
            if (SUCCEEDED(pDevice->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, nullptr,
                                            reinterpret_cast<void**>(&pSessionManager)))) {
                if (SUCCEEDED(pSessionManager->GetSimpleAudioVolume(nullptr, FALSE, &m_pSimpleVolume))) {
                    VERIFY(SUCCEEDED(m_pSimpleVolume->GetMasterVolume(&m_fInitialVolume)));
                }
            }
        }
    }
    ASSERT(m_pSimpleVolume);
}

CShockwaveGraph::~CShockwaveGraph()
{
    m_wndDestFrame.DestroyWindow();
    m_wndWindowFrame.DestroyWindow();
    if (m_pSimpleVolume) {
        VERIFY(SUCCEEDED(m_pSimpleVolume->SetMasterVolume(m_fInitialVolume, nullptr)));
    }
}

// IGraphBuilder
STDMETHODIMP CShockwaveGraph::RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList)
{
    try {
        m_wndDestFrame.LoadMovie(0, CString(lpcwstrFile));
        m_wndDestFrame.Stop();
    } catch (CException* e) {
        e->Delete();
        return E_FAIL;
    }
    return S_OK;
}

// IMediaControl
STDMETHODIMP CShockwaveGraph::Run()
{
    try {
        // XXX - Does the following line have some side effect
        // or is the variable unused?
        /*long scale_mode = */this->m_wndDestFrame.get_ScaleMode();

        if (m_fs != State_Running) {
            m_wndDestFrame.Play();
        }
    } catch (CException* e) {
        e->Delete();
        return E_FAIL;
    }
    m_fs = State_Running;
    m_wndWindowFrame.EnableWindow();
    //  m_wndDestFrame.EnableWindow();
    return S_OK;
}

STDMETHODIMP CShockwaveGraph::Pause()
{
    try {
        if (m_fs == State_Running) {
            m_wndDestFrame.Stop();
        }
    } catch (CException* e) {
        e->Delete();
        return E_FAIL;
    }
    m_fs = State_Paused;
    return S_OK;
}

STDMETHODIMP CShockwaveGraph::Stop()
{
    try {
        m_wndDestFrame.Stop();
    } catch (CException* e) {
        e->Delete();
        return E_FAIL;
    }
    m_fs = State_Stopped;
    return S_OK;
}

STDMETHODIMP CShockwaveGraph::GetState(LONG msTimeout, OAFilterState* pfs)
{
    OAFilterState fs = m_fs;

    try {
        if (m_wndDestFrame.IsPlaying() && m_fs == State_Stopped) {
            m_fs = State_Running;
        } else if (!m_wndDestFrame.IsPlaying() && m_fs == State_Running) {
            m_fs = State_Stopped;
        }
        fs = m_fs;
    } catch (CException* e) {
        e->Delete();
        return E_FAIL;
    }

    CheckPointer(pfs, E_POINTER);

    *pfs = fs;

    return S_OK;
}

// IMediaSeeking
STDMETHODIMP CShockwaveGraph::IsFormatSupported(const GUID* pFormat)
{
    return !pFormat ? E_POINTER : *pFormat == TIME_FORMAT_FRAME ? S_OK : S_FALSE;
}

STDMETHODIMP CShockwaveGraph::GetTimeFormat(GUID* pFormat)
{
    CheckPointer(pFormat, E_POINTER);

    *pFormat = TIME_FORMAT_FRAME;

    return S_OK;
}

STDMETHODIMP CShockwaveGraph::GetDuration(LONGLONG* pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    *pDuration = 0;
    try {
        if (m_wndDestFrame.get_ReadyState() >= READYSTATE_COMPLETE) {
            *pDuration = m_wndDestFrame.get_TotalFrames();
        }
    } catch (CException* e) {
        e->Delete();
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP CShockwaveGraph::GetCurrentPosition(LONGLONG* pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);
    *pCurrent = 0;
    try {
        if (m_wndDestFrame.get_ReadyState() >= READYSTATE_COMPLETE) {
            *pCurrent = m_wndDestFrame.get_FrameNum();
        }
    } catch (CException* e) {
        e->Delete();
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP CShockwaveGraph::SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags)
{
    if (dwCurrentFlags & AM_SEEKING_AbsolutePositioning) {
        m_wndDestFrame.put_FrameNum((long)*pCurrent);

        if (m_fs == State_Running && !m_wndDestFrame.IsPlaying()) {
            m_wndDestFrame.Play();
        } else if ((m_fs == State_Paused || m_fs == State_Stopped) && m_wndDestFrame.IsPlaying()) {
            m_wndDestFrame.Stop();
        }

        m_wndDestFrame.put_Quality(1); // 0=Low, 1=High, 2=AutoLow, 3=AutoHigh

        return S_OK;
    }

    return E_INVALIDARG;
}

// IVideoWindow
STDMETHODIMP CShockwaveGraph::put_Visible(long Visible)
{
    if (IsWindow(m_wndDestFrame.m_hWnd)) {
        m_wndDestFrame.ShowWindow(Visible == OATRUE ? SW_SHOWNORMAL : SW_HIDE);
    }
    return S_OK;
}

STDMETHODIMP CShockwaveGraph::get_Visible(long* pVisible)
{
    CheckPointer(pVisible, E_POINTER);

    *pVisible = m_wndDestFrame.IsWindowVisible() ? OATRUE : OAFALSE;

    return S_OK;
}

STDMETHODIMP CShockwaveGraph::SetWindowPosition(long Left, long Top, long Width, long Height)
{
    if (IsWindow(m_wndWindowFrame.m_hWnd)) {
        m_wndWindowFrame.MoveWindow(Left, Top, Width, Height);
    }

    return S_OK;
}

// IBasicVideo
STDMETHODIMP CShockwaveGraph::SetDestinationPosition(long Left, long Top, long Width, long Height)// {return E_NOTIMPL;}
{
    if (IsWindow(m_wndDestFrame.m_hWnd)) {
        m_wndDestFrame.MoveWindow(Left, Top, Width, Height);
    }

    return S_OK;
}

STDMETHODIMP CShockwaveGraph::GetVideoSize(long* pWidth, long* pHeight)
{
    if (!pWidth || !pHeight) {
        return E_POINTER;
    }

    // no call exists to determine these...
    *pWidth = 480;
    *pHeight = 360;

    return S_OK;
}

// IBasicAudio
STDMETHODIMP CShockwaveGraph::put_Volume(long lVolume)
{
    HRESULT hr = S_OK;

    if (m_pSimpleVolume) {
        float fVolume = (lVolume <= -10000) ? 0 : (lVolume >= 0) ? 1 : pow(10.f, lVolume / 4000.f);
        hr = m_pSimpleVolume->SetMasterVolume(fVolume, nullptr);
    } else {
        lVolume = (lVolume <= -10000) ? 0 : (long)(pow(10.0, lVolume / 4000.0) * 100);
        lVolume = lVolume * 0x10000 / 100;
        lVolume = std::max(std::min(lVolume, LONG_MAX), 0l);
        waveOutSetVolume(0, (lVolume << 16) | lVolume);
    }

    return hr;
}

STDMETHODIMP CShockwaveGraph::get_Volume(long* plVolume)
{
    CheckPointer(plVolume, E_POINTER);
    HRESULT hr = S_OK;

    if (m_pSimpleVolume) {
        float fVolume;
        hr = m_pSimpleVolume->GetMasterVolume(&fVolume);
        if (SUCCEEDED(hr)) {
            *plVolume = (fVolume == 0) ? -10000 : long(4000 * log10(fVolume));
        }
    } else {
        waveOutGetVolume(0, (DWORD*)plVolume);
        *plVolume = (*plVolume & 0xffff + ((*plVolume >> 16) & 0xffff)) / 2 * 100 / 0x10000;
        if (*plVolume > 0) {
            *plVolume = std::min((long)(4000 * log10(*plVolume / 100.0f)), 0l);
        } else {
            *plVolume = -10000;
        }
    }

    return hr;
}

// IAMOpenProgress
STDMETHODIMP CShockwaveGraph::QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent)
{
    *pllTotal = 100;
    *pllCurrent = m_wndDestFrame.PercentLoaded();
    return S_OK;
}

// IGraphEngine
STDMETHODIMP_(engine_t) CShockwaveGraph::GetEngine()
{
    return ShockWave;
}
