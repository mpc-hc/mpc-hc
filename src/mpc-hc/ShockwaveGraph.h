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

#pragma once

#include "BaseGraph.h"
#include "CShockwaveFlash.h"
#include <Audioclient.h>

namespace DSObjects
{

    class CShockwaveGraph : public CBaseGraph
    {
        CPlayerWindow m_wndWindowFrame;
        CShockwaveFlash m_wndDestFrame;

        FILTER_STATE m_fs;

        CComPtr<ISimpleAudioVolume> m_pSimpleVolume;
        float m_fInitialVolume;

    public:
        CShockwaveGraph(HWND hParent, HRESULT& hr);
        virtual ~CShockwaveGraph();

    protected:
        // IGraphBuilder
        STDMETHODIMP RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList);

        // IMediaControl
        STDMETHODIMP Run();
        STDMETHODIMP Pause();
        STDMETHODIMP Stop();
        STDMETHODIMP GetState(LONG msTimeout, OAFilterState* pfs);

        // IMediaSeeking
        STDMETHODIMP IsFormatSupported(const GUID* pFormat);
        STDMETHODIMP GetTimeFormat(GUID* pFormat);
        STDMETHODIMP GetDuration(LONGLONG* pDuration);
        STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
        STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags);

        // IVideoWindow
        STDMETHODIMP put_Visible(long Visible);
        STDMETHODIMP get_Visible(long* pVisible);
        STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height);

        // IBasicVideo
        STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height);
        STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight);

        // IBasicAudio
        STDMETHODIMP put_Volume(long lVolume);
        STDMETHODIMP get_Volume(long* plVolume);

        // IAMOpenProgress
        STDMETHODIMP QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent);

        // IGraphEngine
        STDMETHODIMP_(engine_t) GetEngine();
    };

}
