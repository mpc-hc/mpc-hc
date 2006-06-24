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
#include "CShockwaveFlash.h"

namespace DSObjects
{

class CShockwaveGraph : public CBaseGraph
{
	CPlayerWindow m_wndWindowFrame;
	CShockwaveFlash m_wndDestFrame;

	FILTER_STATE m_fs;

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
using namespace DSObjects;