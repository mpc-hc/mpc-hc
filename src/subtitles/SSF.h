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

#include "..\SubPic\ISubPic.h"
#include ".\libssf\SubtitleFile.h"
#include ".\libssf\Renderer.h"

#pragma once

namespace ssf
{
	[uuid("E0593632-0AB7-47CA-8BE1-E9D2A6A4825E")]
	class CRenderer : public ISubPicProviderImpl, public ISubStream
	{
		CString m_fn, m_name;
		CAutoPtr<SubtitleFile> m_file;
		CAutoPtr<Renderer> m_renderer;

	public:
		CRenderer(CCritSec* pLock);
		virtual ~CRenderer();

		bool Open(CString fn, CString name = _T(""));
		bool Open(InputStream& s, CString name);

		void Append(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, LPCWSTR str);

		DECLARE_IUNKNOWN
		STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

		// ISubPicProvider
		STDMETHODIMP_(POSITION) GetStartPosition(REFERENCE_TIME rt, double fps);
		STDMETHODIMP_(POSITION) GetNext(POSITION pos);
		STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps);
		STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps);
		STDMETHODIMP_(bool) IsAnimated(POSITION pos);
		STDMETHODIMP Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);

		// IPersist
		STDMETHODIMP GetClassID(CLSID* pClassID);

		// ISubStream
		STDMETHODIMP_(int) GetStreamCount();
		STDMETHODIMP GetStreamInfo(int i, WCHAR** ppName, LCID* pLCID);
		STDMETHODIMP_(int) GetStream();
		STDMETHODIMP SetStream(int iStream);
		STDMETHODIMP Reload();
	};

}