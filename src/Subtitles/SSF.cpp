/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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

/*
 *  TODO:
 *  - fill effect
 *  - outline bkg still very slow
 *
 */

#include "stdafx.h"
#include <xmmintrin.h>
#include <emmintrin.h>
#include "SSF.h"
#include "../SubPic/MemSubPic.h"

namespace ssf
{
	CRenderer::CRenderer(CCritSec* pLock)
		: CSubPicProviderImpl(pLock)
	{
	}

	CRenderer::~CRenderer()
	{
	}

	bool CRenderer::Open(CString fn, CString name)
	{
		m_fn.Empty();
		m_name.Empty();
		m_file.Free();
		m_renderer.Free();

		if (name.IsEmpty()) {
			CString str = fn;
			str.Replace('\\', '/');
			name = str.Left(str.ReverseFind('.'));
			name = name.Mid(name.ReverseFind('/')+1);
			name = name.Mid(name.ReverseFind('.')+1);
		}

		try {
			if (Open(FileInputStream(fn), name)) {
				m_fn = fn;
				return true;
			}
		} catch (Exception& e) {
			UNREFERENCED_PARAMETER(e);
			TRACE(_T("%s\n"), e.ToString());
		}

		return false;
	}

	bool CRenderer::Open(InputStream& s, CString name)
	{
		m_fn.Empty();
		m_name.Empty();
		m_file.Free();
		m_renderer.Free();

		try {
			m_file.Attach(DNew SubtitleFile());
			m_file->Parse(s);
			m_renderer.Attach(DNew Renderer());
			m_name = name;
			return true;
		} catch (Exception& e) {
			UNREFERENCED_PARAMETER(e);
			TRACE(_T("%s\n"), e.ToString());
		}

		return false;
	}

	void CRenderer::Append(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, LPCWSTR str)
	{
		if (!m_file) {
			return;
		}

		try {
			m_file->Append(ssf::WCharInputStream(str), (float)rtStart / 10000000, (float)rtStop / 10000000);
		} catch (Exception& e) {
			UNREFERENCED_PARAMETER(e);
			TRACE(_T("%s\n"), e.ToString());
		}
	}

	STDMETHODIMP CRenderer::NonDelegatingQueryInterface(REFIID riid, void** ppv)
	{
		CheckPointer(ppv, E_POINTER);
		*ppv = NULL;

		return
			QI(IPersist)
			QI(ISubStream)
			QI(ISubPicProvider)
			__super::NonDelegatingQueryInterface(riid, ppv);
	}

	// ISubPicProvider

	STDMETHODIMP_(POSITION) CRenderer::GetStartPosition(REFERENCE_TIME rt, double fps)
	{
		size_t k = 0;
		return m_file && m_file->m_segments.Lookup((float)rt/10000000, k) ? (POSITION)(++k) : NULL;
	}

	STDMETHODIMP_(POSITION) CRenderer::GetNext(POSITION pos)
	{
		size_t k = (size_t)pos;
		return m_file && m_file->m_segments.GetSegment(k) ? (POSITION)(++k) : NULL;
	}

	STDMETHODIMP_(REFERENCE_TIME) CRenderer::GetStart(POSITION pos, double fps)
	{
		size_t k = (size_t)pos-1;
		const SubtitleFile::Segment* s = m_file ? m_file->m_segments.GetSegment(k) : NULL;
		return s ? (REFERENCE_TIME)(s->m_start*10000000) : 0;
	}

	STDMETHODIMP_(REFERENCE_TIME) CRenderer::GetStop(POSITION pos, double fps)
	{
		CheckPointer(m_file, 0);

		size_t k = (size_t)pos-1;
		const SubtitleFile::Segment* s = m_file ? m_file->m_segments.GetSegment(k) : NULL;
		return s ? (REFERENCE_TIME)(s->m_stop*10000000) : 0;
	}

	STDMETHODIMP_(bool) CRenderer::IsAnimated(POSITION pos)
	{
		return true;
	}

	STDMETHODIMP CRenderer::Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox)
	{
		CheckPointer(m_file, E_UNEXPECTED);
		CheckPointer(m_renderer, E_UNEXPECTED);

		if (spd.type != MSP_RGB32) {
			return E_INVALIDARG;
		}

		CAutoLock csAutoLock(m_pLock);

		CRect bbox2;
		bbox2.SetRectEmpty();

		CAutoPtrList<Subtitle> subs;
		m_file->Lookup((float)rt/10000000, subs);

		m_renderer->NextSegment(subs);

		POSITION pos = subs.GetHeadPosition();
		while (pos) {
			const Subtitle* s = subs.GetNext(pos);
			const RenderedSubtitle* rs = m_renderer->Lookup(s, CSize(spd.w, spd.h), spd.vidrect);
			if (rs) {
				bbox2 |= rs->Draw(spd);
			}
		}

		bbox = bbox2 & CRect(0, 0, spd.w, spd.h);

		return S_OK;
	}

	// IPersist

	STDMETHODIMP CRenderer::GetClassID(CLSID* pClassID)
	{
		return pClassID ? *pClassID = __uuidof(this), S_OK : E_POINTER;
	}

	// ISubStream

	STDMETHODIMP_(int) CRenderer::GetStreamCount()
	{
		return 1;
	}

	STDMETHODIMP CRenderer::GetStreamInfo(int iStream, WCHAR** ppName, LCID* pLCID)
	{
		if (iStream != 0) {
			return E_INVALIDARG;
		}

		if (ppName) {
			*ppName = (WCHAR*)CoTaskMemAlloc((m_name.GetLength()+1)*sizeof(WCHAR));
			if (!(*ppName)) {
				return E_OUTOFMEMORY;
			}

			wcscpy(*ppName, CStringW(m_name));
		}

		if (pLCID) {
			*pLCID = 0; // TODO
		}

		return S_OK;
	}

	STDMETHODIMP_(int) CRenderer::GetStream()
	{
		return 0;
	}

	STDMETHODIMP CRenderer::SetStream(int iStream)
	{
		return iStream == 0 ? S_OK : E_FAIL;
	}

	STDMETHODIMP CRenderer::Reload()
	{
		CAutoLock csAutoLock(m_pLock);

		return !m_fn.IsEmpty() && Open(m_fn, m_name) ? S_OK : E_FAIL;
	}
}
