/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2015 see Authors.txt
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

#ifndef _WIN64

#include "BaseGraph.h"
#include "AllocatorCommon.h"

#include "qt/qt.h"

namespace DSObjects
{

    class CQuicktimeGraph;

    class CQuicktimeWindow : public CPlayerWindow
    {
        CDC m_dc;
        CBitmap m_bm;
        QT::GWorldPtr m_offscreenGWorld;

        CQuicktimeGraph* m_pGraph;
        FILTER_STATE m_fs;
        UINT m_idEndPoller;

        static QT::OSErr MyMovieDrawingCompleteProc(QT::Movie theMovie, long refCon);

        void ProcessMovieEvent(unsigned int message, unsigned int wParam, long lParam);

    protected:
        virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    public:
        CQuicktimeWindow(CQuicktimeGraph* pGraph);

        bool OpenMovie(CString fn);
        void CloseMovie();

        void Run(), Pause(), Stop();
        FILTER_STATE GetState();

        QT::Movie           theMovie;
        QT::MovieController theMC;
        CSize               m_size;

    public:
        DECLARE_MESSAGE_MAP()
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg void OnDestroy();
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnTimer(UINT_PTR nIDEvent);
    };

    class CQuicktimeGraph : public CBaseGraph, public IVideoFrameStep
    {
    protected:
        bool m_fQtInitialized;

        CPlayerWindow m_wndWindowFrame;
        CQuicktimeWindow m_wndDestFrame;

        CComPtr<ISubPicAllocatorPresenter2> m_pQTAP;

    public:
        CQuicktimeGraph(HWND hParent, HRESULT& hr);
        virtual ~CQuicktimeGraph();

        DECLARE_IUNKNOWN;
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    protected:
        // IGraphBuilder
        STDMETHODIMP RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList);

        // IMediaControl
        STDMETHODIMP Run();
        STDMETHODIMP Pause();
        STDMETHODIMP Stop();
        STDMETHODIMP GetState(LONG msTimeout, OAFilterState* pfs);

        // IMediaSeeking
        STDMETHODIMP GetDuration(LONGLONG* pDuration);
        STDMETHODIMP GetCurrentPosition(LONGLONG* pCurrent);
        STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags,
                                  LONGLONG* pStop, DWORD dwStopFlags);
        STDMETHODIMP SetRate(double dRate);
        STDMETHODIMP GetRate(double* pdRate);

        // IVideoWindow
        STDMETHODIMP SetWindowPosition(long Left, long Top, long Width, long Height);

        // IBasicVideo
        STDMETHODIMP SetDestinationPosition(long Left, long Top, long Width, long Height);
        STDMETHODIMP GetVideoSize(long* pWidth, long* pHeight);

        // IBasicAudio
        STDMETHODIMP put_Volume(long lVolume);
        STDMETHODIMP get_Volume(long* plVolume);

        // IVideoFrameStep
        STDMETHODIMP Step(DWORD dwFrames, IUnknown* pStepObject);
        STDMETHODIMP CanStep(long bMultiple, IUnknown* pStepObject);
        STDMETHODIMP CancelStep();

        // IGraphEngine
        STDMETHODIMP_(engine_t) GetEngine();
    };

}
#endif
