/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once
#include "BaseGraph.h"
#include "DX7AllocatorPresenter.h"
#include "DX9AllocatorPresenter.h"

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

	QT::Movie			theMovie;
    QT::MovieController theMC;
	CSize				m_size;

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT nIDEvent);
};

class CQuicktimeGraph : public CBaseGraph, public IVideoFrameStep
{
protected:
	bool m_fQtInitialized;

	CPlayerWindow m_wndWindowFrame;
	CQuicktimeWindow m_wndDestFrame;

	CComPtr<ISubPicAllocatorPresenter> m_pQTAP;

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
	STDMETHODIMP SetPositions(LONGLONG* pCurrent, DWORD dwCurrentFlags, LONGLONG* pStop, DWORD dwStopFlags);
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
using namespace DSObjects;